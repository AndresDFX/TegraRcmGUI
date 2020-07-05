#ifndef QHEKATE_H
#define QHEKATE_H

#include <QWidget>
#include "tegrarcmgui.h"
#include "qutils.h"
#include "qprogress_widget.h"

class TegraRcmGUI;
class Kourou;
class QKourou;
class qProgressWidget;

QT_BEGIN_NAMESPACE
namespace Ui { class qHekate; }
QT_END_NAMESPACE

class qHekate : public QWidget
{
    Q_OBJECT

public:
    explicit qHekate(TegraRcmGUI *parent = nullptr);
    ~qHekate();

private:
    Ui::qHekate *ui;
    TegraRcmGUI *parent;
    QKourou *m_kourou;
    Kourou *m_device;
    qProgressWidget *m_progressWidget;

signals:
    void error(int);
private slots:
    void on_pushButton_clicked();
};

#endif // QHEKATE_H
