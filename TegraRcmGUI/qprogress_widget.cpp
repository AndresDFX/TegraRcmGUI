#include "qprogress_widget.h"
#include "ui_qprogress_widget.h"
#include "qutils.h"
#include <QDateTime>

qProgressWidget::qProgressWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qProgressWidget)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setStyleSheet(GetStyleSheetFromResFile(":/res/ProgressWidget.qss"));

    QMovie *movie = new QMovie(":/res/loader_bg646464.gif");
    ui->loadingLbl->setMovie(movie);
    ui->loadingLbl->show();
    movie->start();

    m_latest_activity = QDateTime::currentSecsSinceEpoch();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(check_activity()));
    timer->start(1000); // Every second
}

qProgressWidget::~qProgressWidget()
{
    delete ui;
}

void qProgressWidget::check_activity()
{
    qint64 now = QDateTime::currentSecsSinceEpoch();
    if (now > m_latest_activity + 5)
        close();
}
