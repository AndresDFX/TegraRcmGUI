#ifndef QHEKATE_H
#define QHEKATE_H

#include <QWidget>
#include "tegrarcmgui.h"
#include "qutils.h"
#include "qprogress_widget.h"

struct AppVersion {
    int major = 0;
    int minor = 0;
    int micro = 0;

    bool operator==(const AppVersion &other) const
    {
        return other.major == major && other.minor == minor && other.micro == micro;
    }

    bool operator<(const AppVersion &other) const
    {
        if (other.major != major)
            return other.major > major;
        else if (other.minor != minor)
            return other.minor > minor;
        else
            return other.micro > micro;
    }
};

struct HekatePayload
{
    QString file_path;
    AppVersion version;

    bool operator<(const HekatePayload &other) const
    {
        // Default is desc sort
        if (!(other.version == version))
            return other.version < version;
        else
            return other.file_path < file_path;
    }
};

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
    QVector<HekatePayload> m_payloads;
    AppVersion m_nyx_version;

signals:
    void error(int);

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
};

#endif // QHEKATE_H
