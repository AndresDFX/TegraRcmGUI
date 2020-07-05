#include "qhekate.h"
#include "ui_qhekate.h"

qHekate::qHekate(TegraRcmGUI *parent) : QWidget(parent), ui(new Ui::qHekate), parent(parent)
{
    ui->setupUi(this);
    m_kourou = parent->m_kourou;
    m_device = &parent->m_device;
    connect(this, SIGNAL(error(int)), parent,  SLOT(error(int)));
}

qHekate::~qHekate()
{
    delete ui;
}

void qHekate::on_pushButton_clicked()
{
    m_progressWidget = new qProgressWidget(parent);
    m_progressWidget->exec();
}
