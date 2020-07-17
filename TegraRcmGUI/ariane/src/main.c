/*
 * Copyright (c) 2018 naehrwert
 * Copyright (c) 2018-2020 CTCaer
 * Copyright (c) 2020 eliboa
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "libs/hekate_config.h"
hekate_config h_cfg;
emummc_cfg_t emu_cfg;

#include <string.h>
#include <stdlib.h>
#include <memory_map.h>
#include <gfx/di.h>
#include <gfx_utils.h>
#include "gfx/gfx.h"
#include <ianos/ianos.h>

#include <mem/heap.h>
#include <mem/minerva.h>
#include <mem/sdram.h>
#include <soc/bpmp.h>
#include <soc/hw_init.h>
#include <soc/i2c.h>
#include <soc/t210.h>
#include <sec/tsec.h>
#include <storage/nx_sd.h>
#include <storage/sdmmc.h>
#include <utils/btn.h>
#include <utils/util.h>
#include <soc/clock.h>
#include <usb/usbd.h>

extern int _usbd_initialize_ep0();


#include "ariane.h"

// Hekate & boot config
boot_cfg_t __attribute__((section ("._boot_cfg"))) b_cfg;
const volatile ipl_ver_meta_t __attribute__((section ("._ipl_version"))) ipl_ver = {
    .magic = BL_MAGIC,
    .version = (BL_VER_MJ + '0') | ((BL_VER_MN + '0') << 8) | ((BL_VER_HF + '0') << 16),
    .rsvd0 = 0,
    .rsvd1 = 0
};

volatile nyx_storage_t *nyx_str = (nyx_storage_t *)NYX_STORAGE_ADDR;
extern void pivot_stack(u32 stack_top);

static void *coreboot_addr;

void reloc_patcher(u32 payload_dst, u32 payload_src, u32 payload_size)
{
    memcpy((u8 *)payload_src, (u8 *)IPL_LOAD_ADDR, PATCHED_RELOC_SZ);

    volatile reloc_meta_t *relocator = (reloc_meta_t *)(payload_src + RELOC_META_OFF);

    relocator->start = payload_dst - ALIGN(PATCHED_RELOC_SZ, 0x10);
    relocator->stack = PATCHED_RELOC_STACK;
    relocator->end   = payload_dst + payload_size;
    relocator->ep    = payload_dst;

    if (payload_size == 0x7000)
    {
        memcpy((u8 *)(payload_src + ALIGN(PATCHED_RELOC_SZ, 0x10)), coreboot_addr, 0x7000); //Bootblock
        *(vu32 *)CBFS_DRAM_EN_ADDR = CBFS_DRAM_MAGIC;
    }
}

bool is_ipl_updated(void *buf, char *path, bool force)
{
    ipl_ver_meta_t *update_ft = (ipl_ver_meta_t *)(buf + PATCHED_RELOC_SZ + sizeof(boot_cfg_t));

    bool magic_valid = update_ft->magic == ipl_ver.magic;
    bool force_update = force && !magic_valid;
    bool is_valid_old = magic_valid && (byte_swap_32(update_ft->version) < byte_swap_32(ipl_ver.version));

    if (magic_valid)
        if (byte_swap_32(update_ft->version) > byte_swap_32(ipl_ver.version))
            return false;
            
	// Update if old or broken.
	if (force_update || is_valid_old)
	{
		FIL fp;
		volatile reloc_meta_t *reloc = (reloc_meta_t *)(IPL_LOAD_ADDR + RELOC_META_OFF);
		boot_cfg_t *tmp_cfg = malloc(sizeof(boot_cfg_t));
		memset(tmp_cfg, 0, sizeof(boot_cfg_t));

		f_open(&fp, path, FA_WRITE | FA_CREATE_ALWAYS);
		f_write(&fp, (u8 *)reloc->start, reloc->end - reloc->start, NULL);

		// Write needed tag in case injected ipl uses old versioning.
        f_write(&fp, "ARIA01", 6, NULL);

		// Reset boot storage configuration.
		f_lseek(&fp, PATCHED_RELOC_SZ);
		f_write(&fp, tmp_cfg, sizeof(boot_cfg_t), NULL);

		f_close(&fp);
		free(tmp_cfg);
	}
    return true;
}

void ipl_main()
{
    u32 start_tmr = get_tmr_ms();
    config_hw(); // Do initial HW configuration. This is compatible with consecutive reruns without a reset.
    bpmp_mmu_disable(); // Disable MMU to get RCM USB access. We don't neeed mmu for Ariane.
    bpmp_clk_rate_set(BPMP_CLK_DEFAULT_BOOST);

    // Pivot the stack so we have enough space.
    pivot_stack(IPL_STACK_TOP);

    // Tegra/Horizon configuration goes to 0x80000000+, package2 goes to 0xA9800000, we place our heap in between.
    heap_init(IPL_HEAP_START);

    // Hekate default conf
    set_default_configuration();

    // Train DRAM and switch to max frequency.
    minerva_init();
    minerva_change_freq(FREQ_1600);

    if(!sd_mount())
        gfx_printf("FAILED TO MOUNT SD !!!\n");

    display_init();
    u32 *fb = display_init_framebuffer_pitch();
    gfx_init_ctxt(fb, 720, 1280, 720);
    gfx_con_init();
    gfx_clear_grey(0x1B);
    gfx_con_setpos(0, 0);
    display_backlight_pwm_init();
    display_backlight_brightness(100, 1000);

    gfx_printf("ARIANE by eliboa\n");
    gfx_printf("credits to CTCaer & naehrwert\n");

    /*
    if(usb_device_init())
    {
        usbd_end(false, true);
        gfx_printf("ERROR USB not initialized\n");
        goto end;
    }
    //usb_device_ep0_initialize(USB_GADGET_UMS);
    _usbd_initialize_ep0();
    */
    if(b_cfg.extra_cfg & EXTRA_CFG_NYX_BIS)
    {
        PMC(0x50) = (1 << 1);
        gfx_printf("EXTRA_CFG_NYX_BIS\n");
        gfx_printf("h_cfg.sept_run = %s\n", !h_cfg.sept_run ? "false" : "true");
        hos_set_keys(FORBID_SEPT_REBOOT);
        //bpmp_halt();
        //goto end;
    }        

    if (!rcm_usb_device_ready())
    {
        gfx_printf("ERROR RCM USB not initialized\n");
        goto end;
    }

    gfx_printf("TEST\n");
    set_deviceinfo();

    gfx_printf("%kARIANE loaded in %d ms%k\n\n", 0xFFFFFF00, get_tmr_ms() - start_tmr, 0xFFFFFFFF);

    while(1)
    {
        if (btn_read() == BTN_POWER)
            goto end;

        // Handle USB commands
        UC_CommandType command = usb_command_read();
        if (command != NONE)
            command_dispatcher(command);
    }

end:
    bpmp_halt();
    return;
}
