#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTcpSocket>
#include <QSettings>
#include "settings.h"
#include "console.h"
#include "find.h"
#include "actionbutton.h"

QT_BEGIN_NAMESPACE

class QLabel;
class QLineEdit;
class QToolButton;
class QAction;
class QSpinBox;
class QTimer;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool isOpen() const;

private slots:
    void openFile();
    void saveFileAs();
    void showSettings();
    void selectFont();
    void about();

    void writeData(const QByteArray &data);
    void consoleContextMenu(const QPoint &pos);

    void open();
    void openError(QString message);
    void close();

    void connected();
    void disconnected();

    void serialReadyRead();
    void serialErrorOccurred(QSerialPort::SerialPortError error);

    void socketStateUpdate(QAbstractSocket::SocketState state);
    void socketErrorOccurred(QAbstractSocket::SocketError error);
    void socketReadyRead();

    void bytesWritten(qint64 bytes);
    void writeTimeout();

private:
    void serialStateUpdate();
    void updateStatus(QString &status);
    void showWriteError(const QString &message);

    void readSettings();
    void writeSettings();

    Ui::MainWindow *m_ui = nullptr;
    Console *m_console = nullptr;
    DialogFind *m_find = nullptr;

    QLabel *m_labelStatus = nullptr;

    DialogSettings::Settings m_settings;
    qint64 m_bytesToWrite = 0;
    QTimer *m_timerWrite = nullptr;
    QTimer *m_timerAddr = nullptr;
    QSerialPort *m_serial = nullptr;
    QTcpSocket *m_socket = nullptr;


    typedef struct {
        QLabel *labelNum;
        QLineEdit *lineEditCommand;
        ActionButton *actionButtonSend;
        QAction *actionSend;
        QSpinBox *spinBoxInterval;
        ActionButton *actionButtonSendInterval;
        QAction *actionSendInterval;
        QTimer *timer;
    } CommandControls;

    QVector<CommandControls> m_commandControls;

    QByteArray strToCmd(QString value);
    QByteArray convertData(const QByteArray &data);

    int m_addr;
    QByteArray addrToCmd(QString value);
    QByteArray addrToBin(int value, int digits, QDataStream::ByteOrder byteOrder);
    void addrRangeUpdate(int type, int digits);
    void addrStart(bool value);

    void setToolStatusTip(QAction *widget, QString tip = "");
};

#endif // MAINWINDOW_H
