#include "qhekate.h"
#include "ui_qhekate.h"

qHekate::qHekate(TegraRcmGUI *parent) : QWidget(parent), ui(new Ui::qHekate), parent(parent)
{
    ui->setupUi(this);
    m_kourou = parent->m_kourou;
    m_device = &parent->m_device;
    connect(this, SIGNAL(error(int)), parent,  SLOT(error(int)));

    // Binaries check
    QString hekate_payload;
    QStringList filter, files;
    QVector<HekatePayload> payloads;

    // Look for hekate's payloads
    filter << "hekate*.bin";
    QDirIterator it("hekate", filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        HekatePayload pl;
        pl.file_path = it.next();
        payloads.push_back(pl);
    }
    // Binary check & get version
    for (HekatePayload pl : payloads)
    {
        QFile file(pl.file_path);
        file.open(QIODevice::ReadOnly);
        file.seek(0x119);
        QString magic(file.read(3));
        if (magic == "CTC")
        {
            HekatePayload vpl;
            vpl.file_path = pl.file_path;
            vpl.version.major = QString(file.read(1)).toInt();
            vpl.version.minor = QString(file.read(1)).toInt();
            vpl.version.micro = QString(file.read(1)).toInt();
            m_payloads.push_back(vpl);
        }
        file.close();
    }
    std::sort(m_payloads.begin(), m_payloads.end()); // Desc sort

    // Check nyx bin & get version
    QFile nyx_bin("hekate/bootloader/sys/nyx.bin");
    nyx_bin.open(QIODevice::ReadOnly);
    if (nyx_bin.isOpen())
    {
        nyx_bin.seek(0x99);
        QString magic(nyx_bin.read(3));
        if (magic == "CTC")
        {
            m_nyx_version.major = QString(nyx_bin.read(1)).toInt();
            m_nyx_version.minor = QString(nyx_bin.read(1)).toInt();
            m_nyx_version.micro = QString(nyx_bin.read(1)).toInt();
        }
    }

    int i = 0;
}

qHekate::~qHekate()
{
    delete ui;
}

void qHekate::on_pushButton_clicked()
{
    m_progressWidget = new qProgressWidget(parent);    
    QtConcurrent::run(m_kourou, &QKourou::installSDFiles, QString("hekate"), m_progressWidget, false);
    m_progressWidget->exec();
}

void qHekate::on_pushButton_2_clicked()
{
    QtConcurrent::run(m_kourou, &QKourou::getKeys);
}
