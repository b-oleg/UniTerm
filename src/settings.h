#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSerialPort>

QT_BEGIN_NAMESPACE

namespace Ui {
class DialogSettings;
}

class QIntValidator;

QT_END_NAMESPACE

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    typedef enum {
        Serial = 0,
        Tcp = 1,
        UdpUnicast = 2,
        UdpBroadcast = 3
    } ConnectionType;

    typedef struct {
        ConnectionType type;
        int timeoutWrite;

        // comport
        QString name;
        qint32 baudRate;
        QSerialPort::DataBits dataBits;
        QSerialPort::Parity parity;
        QSerialPort::StopBits stopBits;
        QSerialPort::FlowControl flowControl;
        bool dtr;
        bool rts;

        // tcp
        QString host;
        quint16 port;

        // terminal
        bool localEcho;
        bool timeStamp;
        bool hexLog;
        bool hexAll;
        bool linefeed;
        char linefeedChar;

    } Settings;

    explicit DialogSettings(Settings &settings, QWidget *parent = nullptr);
    ~DialogSettings();

    Settings settings() const;

private slots:
    void apply();
    void showPortInfo(int idx);
    void checkCustomDevicePathPolicy(int idx);
    void checkCustomBaudRatePolicy(int idx);
    void setDefault();
    void setType(int idx);

private:
    void fillPortsInfo();
    void fillParameters();
    void setSettings(Settings value);
    void updateSettings();

private:
    Ui::DialogSettings *m_ui = nullptr;
    Settings m_currentSettings;
    QIntValidator *m_intValidator = nullptr;
};

#endif // SETTINGS_H
