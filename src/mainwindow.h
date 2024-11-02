#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
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

private slots:
    void open();
    void saveAs();
    void openSerialPort();
    void closeSerialPort();
    void about();
    QByteArray convertData(const QByteArray &data);
    void writeData(const QByteArray &data);
    void readData();

    void handleError(QSerialPort::SerialPortError error);
    void handleBytesWritten(qint64 bytes);
    void handleWriteTimeout();

    void showSettings();
    void selectFont();

    void consoleContextMenu(const QPoint &pos);

private:
    void updateStatus();
    void showWriteError(const QString &message);

    void readSettings();
    void writeSettings();

    Ui::MainWindow *m_ui = nullptr;
    Console *m_console = nullptr;
    DialogFind *m_find = nullptr;

    QLabel *m_labelStatus = nullptr;

    DialogSettings::Settings m_settings;
    qint64 m_bytesToWrite = 0;
    QTimer *m_timer = nullptr;
    QTimer *m_timerAddr = nullptr;
    QSerialPort *m_serial = nullptr;


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

    int m_addr;
    QByteArray addrToCmd(QString value);
    QByteArray addrToBin(int value, int digits, QDataStream::ByteOrder byteOrder);
    void addrRangeUpdate(int type, int digits);
    void addrStart(bool value);

    void setToolStatusTip(QAction *widget, QString tip = "");
};

#endif // MAINWINDOW_H
