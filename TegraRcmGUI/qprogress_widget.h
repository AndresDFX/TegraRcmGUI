#ifndef QPROGRESS_WIDGET_H
#define QPROGRESS_WIDGET_H

#include <QDialog>

namespace Ui {
class qProgressWidget;
}

class qProgressWidget : public QDialog
{
    Q_OBJECT

public:
    explicit qProgressWidget(QWidget *parent = nullptr);
    ~qProgressWidget();

private:
    Ui::qProgressWidget *ui;
    qint64 m_latest_activity;

private slots:
    void check_activity();
};

#endif // QPROGRESS_WIDGET_H
