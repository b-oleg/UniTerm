#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settings.h"

#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QSpinBox>
#include <QAction>
#include <QKeySequence>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <QClipboard>
#include <QMimeData>
#include <QFontDialog>
#include <QDataStream>

#define DEFAULT_TIMEOUT_WRITE               5000
#define DEFAULT_HOST                        "localhost"
#define DEFAULT_PORT                        2000
#define DEFAULT_SERIAL_SIGNALS_INTERVAL     100
#define DEFAULT_LINEFEED_CHAR               13

#define COMMAND_HOT_COUNT                   10

const char* defaultCommand[COMMAND_HOT_COUNT] = {"AT\\0d", "ATI1\\0d", "ATI2\\0d", ":04G0\\0d", ":05G0\\0d", ":06G0\\0d", ":07G0\\0d", ":08G0\\0d", ":09G0\\0d", ":10G0\\0d"};
const char* defaultShortcutSendKey  = "Alt+%1";
const char* defaultShortcutLoopKey  = "Shift+%1";
const char* defaultAddrFormat = ":\\#G0\\0d";

// settings keys
const char* strFind = "Find";
const char* strGeometry = "Geometry";
const char* strOpacity = "Opacity";
const char* strEnumerate = "Enumerate";
const char* strFormat = "Format";
const char* strType = "Type";
const char* strDigits = "Digits";
const char* strFrom = "From";
const char* strTo = "To";
const char* strInterval = "Interval";
const char* strCrc = "Crc";
const char* strCommands = "Commands";
const char* strCount = "Count";
const char* strValueNum = "Value%1";
const char* strCrcNum = "Crc%1";
const char* strIntervalNum = "Interval%1";
const char* strSettings = "Settings";
const char* strTimeoutWrite = "TimeoutWrite";
const char* strName = "Name";
const char* strBaud = "Baud";
const char* strDataBits = "DataBits";
const char* strParity = "Parity";
const char* strStopBits = "StopBits";
const char* strFlowControl = "FlowControl";
const char* strDtr = "DTR";
const char* strRts = "RTS";
const char* strHost = "Host";
const char* strPort = "Port";
const char* strHexLog = "HexLog";
const char* strHexAll = "HexAll";
const char* strLinefeed = "Linefeed";
const char* strLinefeedChar = "LinefeedChar";
const char* strLocalEcho = "LocalEcho";
const char* strTimeStamp = "TimeStamp";
const char* strWindow = "Window";
const char* strState = "State";
const char* strFont = "Font";
const char* strDirectory = "Directory";
const char* strConnected = "Connected";

const QString statusSeparator = QStringLiteral(" - ");


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_console(new Console(this)),
    m_find(new DialogFind(m_console, this)),
    m_labelStatus(new QLabel(this)),
    m_labelLedDtr(new LabelLed(this, "DTR", false)),
    m_labelLedRts(new LabelLed(this, "RTS", false)),
    m_labelLedCts(new LabelLed(this, "CTS", false)),
    m_labelLedDsr(new LabelLed(this, "DSR", false)),
    m_labelLedCd(new LabelLed(this, "CD", false)),
    m_labelLedRi(new LabelLed(this, "RI", false)),
    m_labelLedStd(new LabelLed(this, "ST", false)),
    m_labelLedSrd(new LabelLed(this, "SR", false)),
    m_timerSerialSignals(new QTimer(this)),
    m_timerWrite(new QTimer(this)),
    m_timerAddr(new QTimer(this)),
    m_serial(new QSerialPort(this)),
    m_tcp(new QTcpSocket(this)),
    m_udp(new QUdpSocket(this)),
    m_crc(new Crc(this))
{
    m_ui->setupUi(this);
    setCentralWidget(m_console);

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);

    // status
    m_ui->statusBar->addPermanentWidget(m_labelStatus);
    m_ui->statusBar->addPermanentWidget(m_labelLedDtr);
    m_ui->statusBar->addPermanentWidget(m_labelLedRts);
    m_ui->statusBar->addPermanentWidget(m_labelLedCts);
    m_ui->statusBar->addPermanentWidget(m_labelLedDsr);
    m_ui->statusBar->addPermanentWidget(m_labelLedCd);
    m_ui->statusBar->addPermanentWidget(m_labelLedRi);
    m_ui->statusBar->addPermanentWidget(m_labelLedStd);
    m_ui->statusBar->addPermanentWidget(m_labelLedSrd);

    m_labelLedDtr->setVisible(false);
    m_labelLedRts->setVisible(false);
    m_labelLedCts->setVisible(false);
    m_labelLedDsr->setVisible(false);
    m_labelLedCd->setVisible(false);
    m_labelLedRi->setVisible(false);
    m_labelLedStd->setVisible(false);
    m_labelLedSrd->setVisible(false);

    m_labelLedDtr->setStatusTip(tr("Сигнал 'Data Terminal Ready'"));
    m_labelLedRts->setStatusTip(tr("Сигнал 'Request To Send'"));
    m_labelLedCts->setStatusTip(tr("Сигнал 'Clear To Send'"));
    m_labelLedDsr->setStatusTip(tr("Сигнал 'Data Set Ready'"));
    m_labelLedCd->setStatusTip(tr("Сигнал 'Data Carrier Detect'"));
    m_labelLedRi->setStatusTip(tr("Сигнал 'Ring Indicator'"));
    m_labelLedStd->setStatusTip(tr("Сигнал 'Secondary Transmitted Data'"));
    m_labelLedSrd->setStatusTip(tr("Сигнал 'Secondary Received Data'"));

    connect(m_serial, &QSerialPort::dataTerminalReadyChanged, m_labelLedDtr, &LabelLed::setLed);
    connect(m_serial, &QSerialPort::dataTerminalReadyChanged, this, &MainWindow::readSerialSignals);
    connect(m_serial, &QSerialPort::requestToSendChanged, m_labelLedRts, &LabelLed::setLed);
    connect(m_serial, &QSerialPort::requestToSendChanged, this, &MainWindow::readSerialSignals);
    connect(m_timerSerialSignals, &QTimer::timeout, this, &MainWindow::readSerialSignals);
    m_timerSerialSignals->setInterval(DEFAULT_SERIAL_SIGNALS_INTERVAL);

    // dock
    m_ui->dockWidgetEnumerate->toggleViewAction()->setIcon(QIcon(":/ico/enumeration.ico"));
    m_ui->dockWidgetEnumerate->toggleViewAction()->setShortcut(QKeySequence("F5"));
    m_ui->dockWidgetEnumerate->toggleViewAction()->setToolTip(QString(tr("Отобразить/скрыть панель перебора значений (%1)")).arg(m_ui->dockWidgetEnumerate->toggleViewAction()->shortcut().toString()));
    m_ui->dockWidgetEnumerate->toggleViewAction()->setStatusTip(m_ui->dockWidgetEnumerate->toggleViewAction()->toolTip());

    m_ui->dockWidgetCommands->toggleViewAction()->setIcon(QIcon(":/ico/commands.ico"));
    m_ui->dockWidgetCommands->toggleViewAction()->setShortcut(QKeySequence("F6"));
    m_ui->dockWidgetCommands->toggleViewAction()->setToolTip(QString(tr("Отобразить/скрыть панель команд (%1)")).arg(m_ui->dockWidgetCommands->toggleViewAction()->shortcut().toString()));
    m_ui->dockWidgetCommands->toggleViewAction()->setStatusTip(m_ui->dockWidgetCommands->toggleViewAction()->toolTip());

    // toolbar
    m_ui->toolBar->addAction(m_ui->dockWidgetEnumerate->toggleViewAction());
    m_ui->toolBar->addAction(m_ui->dockWidgetCommands->toggleViewAction());

    // menu
    m_ui->toolBar->toggleViewAction()->setText(tr("Панель инструментов"));
    m_ui->toolBar->toggleViewAction()->setToolTip(tr("Показать/скрыть панель инструментов"));
    m_ui->toolBar->toggleViewAction()->setShortcut(QKeySequence("F2"));
    setToolStatusTip(m_ui->toolBarCommand->toggleViewAction());
    m_ui->menuView->addAction(m_ui->toolBar->toggleViewAction());

    m_ui->toolBarCommand->toggleViewAction()->setText(tr("Панель команд"));
    m_ui->toolBarCommand->toggleViewAction()->setToolTip(tr("Показать/скрыть панель команд"));
    m_ui->toolBarCommand->toggleViewAction()->setShortcut(QKeySequence("F3"));
    setToolStatusTip(m_ui->toolBarCommand->toggleViewAction());
    m_ui->menuView->addAction(m_ui->toolBarCommand->toggleViewAction());

    m_ui->toolBarCommandLoop->toggleViewAction()->setText(tr("Панель циклических команд"));
    m_ui->toolBarCommandLoop->toggleViewAction()->setToolTip(tr("Показать/скрыть панель циклических команд"));
    m_ui->toolBarCommandLoop->toggleViewAction()->setShortcut(QKeySequence("F4"));
    setToolStatusTip(m_ui->toolBarCommandLoop->toggleViewAction());
    m_ui->menuView->addAction(m_ui->toolBarCommandLoop->toggleViewAction());

    m_ui->menuView->addSeparator();
    m_ui->menuView->addAction(m_ui->dockWidgetEnumerate->toggleViewAction());
    m_ui->menuView->addAction(m_ui->dockWidgetCommands->toggleViewAction());
    m_ui->menuView->addSeparator();
    m_ui->menuView->addAction(m_ui->actionSelectFont);

    // ui
    setToolStatusTip(m_ui->actionOpen);
    setToolStatusTip(m_ui->actionSaveAs);
    setToolStatusTip(m_ui->actionQuit);
    setToolStatusTip(m_ui->actionCopy);
    setToolStatusTip(m_ui->actionPaste);
    setToolStatusTip(m_ui->actionSelectAll);
    setToolStatusTip(m_ui->actionFind);
    setToolStatusTip(m_ui->actionClear);
    setToolStatusTip(m_ui->actionSelectFont);
    setToolStatusTip(m_ui->actionConnect);
    setToolStatusTip(m_ui->actionDisconnect);
    setToolStatusTip(m_ui->actionSettings);
    setToolStatusTip(m_ui->actionSendFile);
    setToolStatusTip(m_ui->actionSendBreak);
    setToolStatusTip(m_ui->actionDtr);
    setToolStatusTip(m_ui->actionRts);
    setToolStatusTip(m_ui->actionAbout);

    connect(m_ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(m_ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);

    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::open);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui->actionSettings, &QAction::triggered, this, &MainWindow::showSettings);
    connect(m_ui->actionSendFile, &QAction::triggered, this, &MainWindow::sendFile);

    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);

    connect(m_ui->actionClear, &QAction::triggered, m_console, &Console::clear);
    connect(m_ui->actionSelectFont, &QAction::triggered, this, &MainWindow::selectFont);
    connect(m_ui->actionSelectAll, &QAction::triggered, m_console, &Console::selectAll);
    connect(m_ui->actionFind, &QAction::triggered, m_find, &DialogFind::show);
    connect(m_console, &Console::textChanged, this, [=]() {
        bool consoleIsEmpty = m_console->document()->isEmpty();
        m_ui->actionSelectAll->setEnabled(!consoleIsEmpty);
        m_ui->actionFind->setEnabled(!consoleIsEmpty);
        m_ui->actionClear->setEnabled(!consoleIsEmpty);
        m_ui->actionSaveAs->setEnabled(!consoleIsEmpty);
    });

    // copy
    connect(m_console, &QPlainTextEdit::copyAvailable, m_ui->actionCopy, &QAction::setEnabled);
    connect(m_ui->actionCopy, &QAction::triggered, m_console, &Console::copy);

    // paste
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [=]() {
        m_ui->actionPaste->setEnabled(QApplication::clipboard()->mimeData()->hasText());
    });
    connect(m_ui->actionPaste, &QAction::triggered, this, [=]() {
        if (isOpen()) {
            writeData(QApplication::clipboard()->mimeData()->text().toLocal8Bit());
        } else {
            showSettings();
            return;
        }
    });

    // console
    connect(m_console, &Console::getData, this, &MainWindow::writeData);

    // serial
    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::serialErrorOccurred);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::serialReadyRead);
    connect(m_serial, &QSerialPort::bytesWritten, this, &MainWindow::bytesWritten);

    connect(m_serial, &QSerialPort::dataTerminalReadyChanged, m_ui->actionDtr, &QAction::setChecked);
    connect(m_ui->actionDtr, &QAction::toggled, m_serial, &QSerialPort::setDataTerminalReady);
    connect(m_serial, &QSerialPort::requestToSendChanged, m_ui->actionRts, &QAction::setChecked);
    connect(m_ui->actionRts, &QAction::toggled, m_serial, &QSerialPort::setRequestToSend);
    connect(m_ui->actionSendBreak, &QAction::triggered, this, [=](){
        if (m_serial->isOpen()) writeData(QByteArray(1,0));
    });

    // tcp
    connect(m_tcp, &QTcpSocket::connected, this, &MainWindow::connected);
    connect(m_tcp, &QTcpSocket::disconnected, this, &MainWindow::disconnected);
    connect(m_tcp, &QTcpSocket::stateChanged, this, &MainWindow::socketStateUpdate);
    connect(m_tcp, &QTcpSocket::errorOccurred, this, &MainWindow::socketErrorOccurred);
    connect(m_tcp, &QTcpSocket::readyRead, this, &MainWindow::socketReadyRead);
    connect(m_tcp, &QTcpSocket::bytesWritten, this, &MainWindow::bytesWritten);

    // udp
    connect(m_udp, &QUdpSocket::connected, this, &MainWindow::connected);
    connect(m_udp, &QUdpSocket::disconnected, this, &MainWindow::disconnected);
    connect(m_udp, &QUdpSocket::stateChanged, this, &MainWindow::socketStateUpdate);
    //connect(m_udp, &QUdpSocket::errorOccurred, this, &MainWindow::socketErrorOccurred);
    connect(m_udp, &QUdpSocket::readyRead, this, &MainWindow::udpReadyRead);
    connect(m_udp, &QUdpSocket::bytesWritten, this, &MainWindow::bytesWritten);

    // write timer
    connect(m_timerWrite, &QTimer::timeout, this, &MainWindow::writeTimeout);
    m_timerWrite->setSingleShot(true);

    // dock commands
    m_ui->labelNum->setToolTip(tr("Номер команды"));
    m_ui->labelNum->setStatusTip(m_ui->labelNum->toolTip());
    m_ui->labelCommand->setToolTip(tr("Команда"));
    m_ui->labelCommand->setStatusTip(m_ui->labelCommand->toolTip());
    m_ui->labelCrc->setToolTip(tr("Контрольная сумма"));
    m_ui->labelCrc->setStatusTip(m_ui->labelCrc->toolTip());
    m_ui->labelSend->setToolTip(tr("Отправить команду"));
    m_ui->labelSend->setStatusTip(m_ui->labelSend->toolTip());
    m_ui->labelInterval->setToolTip(tr("Интервал отправки команды в циклическом режиме"));
    m_ui->labelInterval->setStatusTip(m_ui->labelInterval->toolTip());
    m_ui->labelSendInterval->setToolTip(tr("Отправлять команду циклически"));
    m_ui->labelSendInterval->setStatusTip(m_ui->labelSendInterval->toolTip());

    m_ui->checkBoxCrc->setToolTip(tr("Показать/скрыть параметры добавления контрольной суммы"));
    m_ui->checkBoxCrc->setStatusTip(m_ui->checkBoxCrc->toolTip());
    m_ui->checkBoxInterval->setToolTip(tr("Показать/скрыть параметры циклической отправки команд"));
    m_ui->checkBoxInterval->setStatusTip(m_ui->checkBoxInterval->toolTip());

    // commands
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.beginGroup(strCommands);
    m_commandControls.resize(settings.value(strCount, COMMAND_HOT_COUNT).toUInt());
    settings.endGroup();
    for (int i = 0; i < m_commandControls.size(); ++i) {

        // dock
        m_commandControls[i].labelNum = new QLabel(this);
        m_commandControls[i].labelNum->setText(QString::number(i+1));
        m_commandControls[i].labelNum->setToolTip(m_ui->labelNum->toolTip());
        m_commandControls[i].labelNum->setStatusTip(m_ui->labelNum->toolTip());

        m_commandControls[i].lineEditCommand = new QLineEdit(this);
        m_commandControls[i].lineEditCommand->setStatusTip(QString(tr("Команда №%1")).arg(i+1));
        m_commandControls[i].lineEditCommand->setToolTip(m_commandControls[i].lineEditCommand->statusTip() + "\n" + m_ui->labelCommandsInfo->text());

        m_commandControls[i].comboBoxCrc = new QComboBox(this);
        m_commandControls[i].comboBoxCrc->addItems(m_crc->list());

        m_commandControls[i].actionButtonSend = new ActionButton(this);
        m_commandControls[i].actionButtonSend->setIcon(QIcon(QStringLiteral(":/ico/send.ico")));

        m_commandControls[i].spinBoxInterval = new QSpinBox(this);
        m_commandControls[i].spinBoxInterval->setRange(10, 5000);
        m_commandControls[i].spinBoxInterval->setValue(1000);
        m_commandControls[i].spinBoxInterval->setSuffix(tr("мс"));
        m_commandControls[i].spinBoxInterval->setToolTip(QString(tr("Интервал отправки команды №%1 в циклическом режиме")).arg(i+1));
        m_commandControls[i].spinBoxInterval->setStatusTip(m_ui->labelInterval->toolTip());

        m_commandControls[i].actionButtonSendInterval = new ActionButton(this);
        m_commandControls[i].actionButtonSendInterval->setIcon(QIcon(QStringLiteral(":/ico/loop.png")));
        m_commandControls[i].actionButtonSendInterval->setToolTip(m_ui->labelSendInterval->toolTip());
        m_commandControls[i].actionButtonSendInterval->setStatusTip(m_ui->labelSendInterval->toolTip());

        m_ui->gridLayoutCommands->addWidget(m_commandControls[i].labelNum, i+1, 0);
        m_ui->gridLayoutCommands->addWidget(m_commandControls[i].lineEditCommand, i+1, 1);
        m_ui->gridLayoutCommands->addWidget(m_commandControls[i].comboBoxCrc, i+1, 2);
        m_ui->gridLayoutCommands->addWidget(m_commandControls[i].actionButtonSend, i+1, 3);
        m_ui->gridLayoutCommands->addWidget(m_commandControls[i].spinBoxInterval, i+1, 4);
        m_ui->gridLayoutCommands->addWidget(m_commandControls[i].actionButtonSendInterval, i+1, 5);

        // action send
        m_commandControls[i].actionSend = new QAction(QString(tr("Команда №%1")).arg(i+1), this);
        m_commandControls[i].actionSend->setEnabled(false);
        if (i < COMMAND_HOT_COUNT) {
            m_commandControls[i].actionSend->setIcon(QIcon(QStringLiteral(":/ico/%1.png").arg(i+1)));
            m_commandControls[i].actionSend->setShortcut(QKeySequence(QString(defaultShortcutSendKey).arg((i+1)%10)));
            m_commandControls[i].actionSend->setToolTip(QString(tr("Отправить команду №%1 (%2)")).arg(i+1).arg(m_commandControls[i].actionSend->shortcut().toString()));
            m_ui->toolBarCommand->addAction(m_commandControls[i].actionSend);
        } else {
            m_commandControls[i].actionSend->setToolTip(QString(tr("Отправить команду №%1")).arg(i+1));
        }
        m_commandControls[i].actionSend->setStatusTip(m_commandControls[i].actionSend->toolTip());
        m_ui->menuSend->addAction(m_commandControls[i].actionSend);

        connect(m_commandControls[i].actionSend, &QAction::triggered, this, [=]() {
            QByteArray cmd = strToCmd(m_commandControls[i].lineEditCommand->text());
            writeData(m_crc->addCrc(cmd, m_commandControls[i].comboBoxCrc->currentIndex()));
        });
        m_commandControls[i].actionButtonSend->setAction(m_commandControls[i].actionSend);
        connect(m_commandControls[i].lineEditCommand, &QLineEdit::returnPressed, m_commandControls[i].actionSend, &QAction::trigger);

        // timer
        m_commandControls[i].timer = new QTimer(this);
        connect(m_commandControls[i].timer, &QTimer::timeout, m_commandControls[i].actionSend, &QAction::trigger);
        connect(m_commandControls[i].spinBoxInterval, &QSpinBox::valueChanged, [=](int value) {
            m_commandControls[i].timer->setInterval(value);
        });

        // action loop
        m_commandControls[i].actionSendInterval = new QAction(QString(tr("Команда №%1")).arg(i+1), this);
        m_commandControls[i].actionSendInterval->setEnabled(false);
        m_commandControls[i].actionSendInterval->setCheckable(true);
        if (i < COMMAND_HOT_COUNT) {
            m_commandControls[i].actionSendInterval->setIcon(QIcon(QString(":/ico/%1-loop.png").arg(i+1)));
            m_commandControls[i].actionSendInterval->setShortcut(QKeySequence(QString(defaultShortcutLoopKey).arg((i+1)%10)));
            m_commandControls[i].actionSendInterval->setToolTip(QString(tr("Циклически отправлять команду №%1 (%2)")).arg(i+1).arg(m_commandControls[i].actionSendInterval->shortcut().toString()));
            m_ui->toolBarCommandLoop->addAction(m_commandControls[i].actionSendInterval);
        } else {
            m_commandControls[i].actionSendInterval->setToolTip(QString(tr("Циклически отправлять команду №%1")).arg(i+1));
        }
        m_commandControls[i].actionSendInterval->setStatusTip(m_commandControls[i].actionSendInterval->toolTip());
        m_ui->menuLoop->addAction(m_commandControls[i].actionSendInterval);
        connect(m_commandControls[i].actionSendInterval, &QAction::toggled, this, [=](bool checked) {
            if (checked) {
                m_commandControls[i].timer->start(m_commandControls[i].spinBoxInterval->value());
            } else {
                m_commandControls[i].timer->stop();
            }
        });
        m_commandControls[i].actionButtonSendInterval->setAction(m_commandControls[i].actionSendInterval);

    }
    //
    connect(m_ui->checkBoxCrc, &QCheckBox::toggled, this, [=](bool checked) {
        m_ui->labelCrc->setVisible(checked);
        for (int i = 0; i < m_commandControls.size(); ++i) m_commandControls[i].comboBoxCrc->setVisible(checked);
    });
    connect(m_ui->checkBoxInterval, &QCheckBox::toggled, this, [=](bool checked) {
        m_ui->labelInterval->setVisible(checked);
        m_ui->labelSendInterval->setVisible(checked);
        for (int i = 0; i < m_commandControls.size(); ++i) {
            m_commandControls[i].spinBoxInterval->setVisible(checked);
            m_commandControls[i].actionButtonSendInterval->setVisible(checked);
        }
    });

    // Enumeration
    m_ui->groupBoxEnumerateFormat->setStatusTip(m_ui->lineEditEnumerateFormat->toolTip());
    m_ui->lineEditEnumerateFormat->setStatusTip(m_ui->lineEditEnumerateFormat->toolTip());
    m_ui->lineEditEnumerateFormat->setToolTip(m_ui->lineEditEnumerateFormat->toolTip() + "\n" + m_ui->labelEnumerateFormatInfo->text());

    m_ui->comboBoxEnumerateType->setToolTip(m_ui->labelEnumerateType->statusTip());
    m_ui->comboBoxEnumerateType->setStatusTip(m_ui->labelEnumerateType->statusTip());
    m_ui->spinBoxEnumerateFrom->setToolTip(m_ui->labelEnumerateFrom->statusTip());
    m_ui->spinBoxEnumerateFrom->setStatusTip(m_ui->labelEnumerateFrom->statusTip());
    m_ui->spinBoxEnumerateTo->setToolTip(m_ui->labelEnumerateTo->statusTip());
    m_ui->spinBoxEnumerateTo->setStatusTip(m_ui->labelEnumerateTo->statusTip());
    m_ui->spinBoxEnumerateDigits->setToolTip(m_ui->labelEnumerateDigits->statusTip());
    m_ui->spinBoxEnumerateDigits->setStatusTip(m_ui->labelEnumerateDigits->statusTip());
    m_ui->spinBoxEnumerateInterval->setToolTip(m_ui->labelEnumerateInterval->statusTip());
    m_ui->spinBoxEnumerateInterval->setStatusTip(m_ui->labelEnumerateInterval->statusTip());
    m_ui->comboBoxEnumerateCrc->setToolTip(m_ui->labelEnumerateCrc->statusTip());
    m_ui->comboBoxEnumerateCrc->setStatusTip(m_ui->labelEnumerateCrc->statusTip());
    m_ui->pushButtonStart->setToolTip(m_ui->pushButtonStart->statusTip());
    m_ui->pushButtonStop->setToolTip(m_ui->pushButtonStop->statusTip());

    m_ui->comboBoxEnumerateCrc->addItems(m_crc->list());

    connect(m_ui->comboBoxEnumerateType, &QComboBox::currentIndexChanged, this, [=](int index){
        addrRangeUpdate(index, m_ui->spinBoxEnumerateDigits->value());
    });
    connect(m_ui->spinBoxEnumerateDigits, &QSpinBox::valueChanged, this, [=](int value) {
        addrRangeUpdate(m_ui->comboBoxEnumerateType->currentIndex(), value);
    });
    connect(m_ui->spinBoxEnumerateFrom, &QSpinBox::valueChanged, this, [=](int value) {
        m_ui->spinBoxEnumerateTo->setMinimum(value+1);
    });
    connect(m_ui->spinBoxEnumerateTo, &QSpinBox::valueChanged, this, [=](int value) {
        m_ui->spinBoxEnumerateFrom->setMaximum(value-1);
    });
    connect(m_ui->pushButtonStart, &QPushButton::clicked, this, [=]() {
        addrStart(true);
    });
    connect(m_ui->pushButtonStop, &QPushButton::clicked, this, [=]() {
        addrStart(false);
    });
    connect(m_timerAddr, &QTimer::timeout, this, [=](){
        if (m_addr <= m_ui->spinBoxEnumerateTo->value()) {
            QByteArray cmd = addrToCmd(m_ui->lineEditEnumerateFormat->text());
            writeData(m_crc->addCrc(cmd, m_ui->comboBoxEnumerateCrc->currentIndex()));
            m_addr++;
        } else {
            m_ui->pushButtonStop->click();
        }
    });

    // context menu
    connect(m_console, &Console::customContextMenuRequested, this, &MainWindow::consoleContextMenu);
    m_console->setContextMenuPolicy(Qt::CustomContextMenu);

    // console selected text in inactive
    QPalette p = m_console->palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText, p.color(QPalette::Active, QPalette::HighlightedText));
    m_console->setPalette(p);
    m_console->setBackgroundRole(QPalette::Window); // inactive color

    // settings
    readSettings();

    // status
    switch (m_settings.type) {
    case DialogSettings::Tcp: socketStateUpdate(m_tcp->state()); break;
    case DialogSettings::UdpUnicast:
    case DialogSettings::UdpBroadcast: socketStateUpdate(m_udp->state()); break;
    default:
        serialStateUpdate();
    }
}

MainWindow::~MainWindow() {
    writeSettings();
    delete m_ui;
}

void MainWindow::setToolStatusTip(QAction *widget, QString tip) {
    widget->setToolTip(QString("%1 (%2)").arg(tip.isEmpty() ? widget->toolTip() : tip, widget->shortcut().toString()));
    widget->setStatusTip(widget->toolTip());
}

QByteArray MainWindow::strToCmd(QString value) {
    QByteArray result;
    int idx=0;
    while (idx < value.length()) {
        if (value.at(idx) == '\\') {
            if (value.at(idx+1) == '\\') {
                result += '\\';
                idx+=1;
            } else {
                result += value.mid(idx+1,2).toInt(nullptr, 16);
                idx+=2;
            }
        } else {
            result.append(QString(value.at(idx)).toLocal8Bit());
        }
        idx++;
    }
    return result;
}

QByteArray MainWindow::addrToCmd(QString value) {
    QByteArray result;
    int idx=0;
    while (idx < value.length()) {
        if (value.at(idx) == '\\') {
            if (value.at(idx+1) == '\\') {
                result += '\\';
                idx+=1;
            } else if (value.at(idx+1) == '#') {
                switch (m_ui->comboBoxEnumerateType->currentIndex()) {
                case 1: result.append(QStringLiteral("%1").arg(m_addr, m_ui->spinBoxEnumerateDigits->value(), 16, QLatin1Char('0')).toLocal8Bit()); break; // Hex
                case 2: result.append(addrToBin(m_addr, m_ui->spinBoxEnumerateDigits->value(), QDataStream::LittleEndian)); break; // Bin little-endian
                case 3: result.append(addrToBin(m_addr, m_ui->spinBoxEnumerateDigits->value(), QDataStream::BigEndian)); break; // Bin big-endian
                default: result.append(QString("%1").arg(m_addr, m_ui->spinBoxEnumerateDigits->value(), 10, QLatin1Char('0')).toLocal8Bit()); break; // Dec
                }
                idx+=1;
            } else {
                result += value.mid(idx+1,2).toInt(nullptr, 16);
                idx+=2;
            }
        } else {
            result.append(QString(value.at(idx)).toLocal8Bit());
        }
        idx++;
    }
    return result;
}

QByteArray MainWindow::addrToBin(int value, int digits, QDataStream::ByteOrder byteOrder) {
    QByteArray bin, res;
    QDataStream ds(&bin, QIODevice::ReadWrite);
    ds.setByteOrder(QDataStream::BigEndian);
    ds << quint32(value);

    switch (digits) {
    case 1: switch (byteOrder) {
        case QDataStream::LittleEndian:
        case QDataStream::BigEndian:
            res.append(bin.at(3));
            break;
        }
        break;
    case 2: switch (byteOrder) {
        case QDataStream::LittleEndian:
            res.append(bin.at(3));
            res.append(bin.at(2));
            break;
        case QDataStream::BigEndian:
            res.append(bin.at(2));
            res.append(bin.at(3));
            break;
        }
        break;
    case 3: switch (byteOrder) {
        case QDataStream::LittleEndian:
            res.append(bin.at(3));
            res.append(bin.at(2));
            res.append(bin.at(1));
            break;
        case QDataStream::BigEndian:
            res.append(bin.at(1));
            res.append(bin.at(2));
            res.append(bin.at(3));
            break;
        }
        break;
    case 4: switch (byteOrder) {
        case QDataStream::LittleEndian:
            res.append(bin.at(3));
            res.append(bin.at(2));
            res.append(bin.at(1));
            res.append(bin.at(0));
            break;
        case QDataStream::BigEndian:
            res.append(bin);
            break;
        }
        break;
    }
    return res;
}

void MainWindow::addrRangeUpdate(int type, int digits) {
    switch (type) {
    case 1: // Hex
        m_ui->spinBoxEnumerateFrom->setRange(0, qPow(16, digits) - 2);
        m_ui->spinBoxEnumerateTo->setRange(1, qPow(16, digits) - 1);
        m_ui->spinBoxEnumerateFrom->setDisplayIntegerBase(16);
        m_ui->spinBoxEnumerateTo->setDisplayIntegerBase(16);
        break;
    case 2: // Bin little-endian
    case 3: // Bin big-endian
        if (digits < 4) { // spinBox не умеет uint
            m_ui->spinBoxEnumerateFrom->setRange(0, qPow(256, digits) - 2);
            m_ui->spinBoxEnumerateTo->setRange(1, qPow(256, digits) - 1);
        } else {
            m_ui->spinBoxEnumerateFrom->setRange(INT_MIN + 1, INT_MAX - 1);
            m_ui->spinBoxEnumerateTo->setRange(INT_MIN + 2, INT_MAX);
        }
        m_ui->spinBoxEnumerateFrom->setDisplayIntegerBase(16);
        m_ui->spinBoxEnumerateTo->setDisplayIntegerBase(16);
        break;
    default: // Dec
        m_ui->spinBoxEnumerateFrom->setRange(0, qPow(10, digits) - 2);
        m_ui->spinBoxEnumerateTo->setRange(1, qPow(10, digits) - 1);
        m_ui->spinBoxEnumerateFrom->setDisplayIntegerBase(10);
        m_ui->spinBoxEnumerateTo->setDisplayIntegerBase(10);
        break;
    }
    m_ui->spinBoxEnumerateFrom->setValue(m_ui->spinBoxEnumerateFrom->minimum());
    m_ui->spinBoxEnumerateTo->setValue(m_ui->spinBoxEnumerateTo->maximum());
}

void MainWindow::addrStart(bool value) {
    m_ui->pushButtonStart->setEnabled(!value);
    m_ui->pushButtonStop->setEnabled(value);
    m_ui->lineEditEnumerateFormat->setEnabled(!value);
    m_ui->comboBoxEnumerateType->setEnabled(!value);
    m_ui->spinBoxEnumerateFrom->setEnabled(!value);
    m_ui->spinBoxEnumerateTo->setEnabled(!value);
    m_ui->spinBoxEnumerateDigits->setEnabled(!value);
    m_ui->spinBoxEnumerateInterval->setEnabled(!value);
    if (value) {
        m_addr = m_ui->spinBoxEnumerateFrom->value();
        m_timerAddr->start(m_ui->spinBoxEnumerateInterval->value());
    } else {
        m_timerAddr->stop();
    }
}

void MainWindow::openFile() {
    QFileDialog dialog(this, tr("Открыть"), m_dir, tr("Текстовый документ (*.txt)"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (dialog.exec() == QDialog::Accepted) {
        QFile file(dialog.selectedFiles().constFirst());
        if (file.open(QIODevice::ReadOnly)) {
            m_console->clear();
            QString content = QString::fromLocal8Bit(file.readAll());
            m_console->setPlainText(content);
            m_dir = dialog.directory().absolutePath();
            file.close();
        }
    }
}

void MainWindow::saveFileAs() {
    QFileDialog dialog(this, tr("Сохранить как..."), m_dir, tr("Текстовый документ (*.txt)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.selectFile(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss"));
    dialog.setFileMode(QFileDialog::AnyFile);
    if (dialog.exec() == QDialog::Accepted) {
        QFile file(dialog.selectedFiles().constFirst());
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            out << m_console->toPlainText();
            m_dir = dialog.directory().absolutePath();
            file.close();
        }
    }
}

void MainWindow::sendFile() {
    QFileDialog dialog(this, tr("Отправить файл"), m_dir, tr("Все файлы (*.*)"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (dialog.exec() == QDialog::Accepted) {
        QFile file(dialog.selectedFiles().constFirst());
        if (file.open(QIODevice::ReadOnly)) {
            writeData(file.readAll());
            m_dir = dialog.directory().absolutePath();
            file.close();
        }
    }
}

void MainWindow::open() {
    switch (m_settings.type) {

    case DialogSettings::Tcp:
        m_tcp->connectToHost(m_settings.host, m_settings.port);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionSettings->setEnabled(false);
        break;

    case DialogSettings::UdpUnicast:
    case DialogSettings::UdpBroadcast:
        m_udp->bind(m_settings.port, QUdpSocket::ShareAddress);
        connected();
        break;

    default: // DialogSettings::Serial
        m_serial->setPortName(m_settings.name);
        m_serial->setBaudRate(m_settings.baudRate);
        m_serial->setDataBits(m_settings.dataBits);
        m_serial->setParity(m_settings.parity);
        m_serial->setStopBits(m_settings.stopBits);
        m_serial->setFlowControl(m_settings.flowControl);
        if (m_serial->open(QIODevice::ReadWrite)) {
            connected();
        } else {
            openError(QString(tr("Ошибка открытия порта '%1': %2")).
                      arg(m_serial->portName()).arg(m_serial->errorString()));
        }
    }
}

void MainWindow::openError(QString message) {
    m_ui->statusBar->showMessage(message);
    QMessageBox::critical(this, QString(tr("Подключение")), message);
}

void MainWindow::close() {
    switch (m_settings.type) {
    case DialogSettings::Tcp: if (m_tcp->isOpen()) m_tcp->close(); break;
    case DialogSettings::UdpUnicast:
    case DialogSettings::UdpBroadcast:
        if (m_udp->state() == QAbstractSocket::BoundState) {
            m_udp->close();//abort
        }
        disconnected();
        break;
    default: // DialogSettings::Serial
        if (m_serial->isOpen()) m_serial->close();
        disconnected();
    }
}

void MainWindow::connected() {
    m_ui->actionConnect->setEnabled(false);
    m_ui->actionDisconnect->setEnabled(true);
    m_ui->actionSettings->setEnabled(true);
    m_ui->actionSendFile->setEnabled(true);
    switch (m_settings.type) {

    case DialogSettings::Tcp:
    case DialogSettings::UdpUnicast:
    case DialogSettings::UdpBroadcast:
        m_ui->actionDtr->setEnabled(false);
        m_ui->actionRts->setEnabled(false);
        break;

    default: // DialogSettings::Serial
        m_serial->setDataTerminalReady(m_settings.dtr);
        m_ui->actionDtr->setChecked(m_settings.dtr);
        m_serial->setRequestToSend(m_settings.rts);
        m_ui->actionRts->setChecked(m_settings.rts);
        m_ui->actionDtr->setEnabled(true);
        m_ui->actionRts->setEnabled(true);
        m_labelLedDtr->setLed(m_settings.dtr);
        m_labelLedRts->setLed(m_settings.rts);
        m_labelLedDtr->setVisible(true);
        m_labelLedRts->setVisible(true);
        m_labelLedCts->setVisible(true);
        m_labelLedDsr->setVisible(true);
        m_labelLedCd->setVisible(true);
        m_labelLedRi->setVisible(true);
        m_labelLedStd->setVisible(true);
        m_labelLedSrd->setVisible(true);
        m_timerSerialSignals->start();
        serialStateUpdate();
    }
    m_ui->actionSendBreak->setEnabled(true);
    for (int i = 0; i < m_commandControls.size(); ++i) {
        m_commandControls[i].actionButtonSend->setEnabled(true);
        m_commandControls[i].actionButtonSendInterval->setEnabled(true);
        m_commandControls[i].actionSend->setEnabled(true);
        m_commandControls[i].actionSendInterval->setEnabled(true);
    }
    m_console->setBackgroundRole(QPalette::Base);
}

void MainWindow::disconnected() {
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionSettings->setEnabled(true);
    m_ui->actionSendFile->setEnabled(false);
    switch (m_settings.type) {
    case DialogSettings::Tcp: m_ui->statusBar->showMessage(tr("TCP-сокет отключен")); break;
    case DialogSettings::UdpUnicast: m_ui->statusBar->showMessage(tr("UDP Unicast отключен")); break;
    case DialogSettings::UdpBroadcast: m_ui->statusBar->showMessage(tr("UDP Broadcast отключен")); break;
    default: // DialogSettings::Serial
        m_ui->statusBar->showMessage(tr("Последовательный порт отключен"));
        m_labelLedDtr->setVisible(false);
        m_labelLedRts->setVisible(false);
        m_labelLedCts->setVisible(false);
        m_labelLedDsr->setVisible(false);
        m_labelLedCd->setVisible(false);
        m_labelLedRi->setVisible(false);
        m_labelLedStd->setVisible(false);
        m_labelLedSrd->setVisible(false);
        m_timerSerialSignals->stop();
        serialStateUpdate();
    }
    m_ui->actionDtr->setEnabled(false);
    m_ui->actionRts->setEnabled(false);
    m_ui->actionSendBreak->setEnabled(false);
    for (int i = 0; i < m_commandControls.size(); ++i) {
        m_commandControls[i].actionSend->setEnabled(false);
        m_commandControls[i].actionSendInterval->setChecked(false);
        m_commandControls[i].actionSendInterval->setEnabled(false);
        m_commandControls[i].actionButtonSend->setEnabled(false);
        m_commandControls[i].actionButtonSendInterval->setEnabled(false);
    }
    m_console->setBackgroundRole(QPalette::Window);
}


void MainWindow::about() {
    QMessageBox mb;
    mb.addButton("Ok", QMessageBox::AcceptRole);
    QPushButton* bqt =new QPushButton("About 'Qt'");
    mb.addButton(bqt, QMessageBox::ActionRole);
    connect(bqt, &QPushButton::pressed, this, &QApplication::aboutQt);
    mb.setIconPixmap(QPixmap(":/app").scaled(64, 64));
    mb.setWindowTitle(tr("О программе..."));
    mb.setText("<html><head/><body>"
               "<span style=\"font-weight:bold;\">" +
               QCoreApplication::applicationName() + " v" + QCoreApplication::applicationVersion() +
               "</span>"
               "<p>Программа предназначена для взаимодействия<br>с последовательным портом, UDP, TCP-сокетом.</p>"
               "<p>Основана на фреймворке для разработки<br>кроссплатформенного программного<br>обеспечения:<a href=\"https://www.qt.io\"><span style=\"text-decoration:underline;color:#0000ff;\">'Qt'</span></a>.</p>"
               "<p>Использованы изображения из:"
               "<br><a href=\"https://fatcow.com/free-icons\"><span style=\"text-decoration:underline;color:#0000ff;\">'Iconpacks by Fatcow Web Hosting'</span></a>."
               "<br><a href=\"https://icons8.com\"><span style=\"text-decoration:underline;color:#0000ff;\">'Icons8'</span></a>.</p>"
               "<p>Обновления:<br><a href=\"https://github.com/b-oleg/UniTerm/releases\"><span style=\"text-decoration:underline;color:#0000ff;\">'https://github.com/b-oleg/UniTerm'</span></a></p>"
               "<p>Олег Большаков<br/><a href=\"mailto:obolshakov@mail.ru\"><span style=\"text-decoration:underline;color:#0000ff;\">obolshakov@mail.ru</span></a></p>"
               "</body></html>");
    mb.exec();
}

QByteArray MainWindow::convertData(const QByteArray &data) {
    QByteArray res;
    if (m_settings.timeStamp) {
        quint64 ms = QDateTime::currentMSecsSinceEpoch();
        res.append(QDateTime::fromMSecsSinceEpoch(ms).toString("\n[hh:mm:ss.zzz] - ").toLocal8Bit());
    }
    for (int i=0; i<data.size(); ++i) {
        char c = data.at(i);
        if (m_settings.hexLog) {
            if (m_settings.hexAll || (c < ' ')) {
                res.append(QString("<%1>").arg(QByteArray(1, c).toHex().toUpper()).toLocal8Bit());
            } else {
                res.append(c);
            }
            if (m_settings.linefeed) {
                if (c == m_settings.linefeedChar) {
                    res.append('\n');
                }
            }
        } else {
            res.append(c);
        }
    }
    return res;
}

void MainWindow::writeData(const QByteArray &data) {
    if (!isOpen()) {
        showSettings();
        return;
    }
    qint64 written;
    switch (m_settings.type) {
    case DialogSettings::Tcp:
        written = m_tcp->write(data);
        if (written != data.size()) {
            const QString error = tr("Ошибка записи в 'TCP:%1:%2'!\nError: '%3'").arg(m_settings.host).arg(m_settings.port).arg(m_serial->errorString());
            showWriteError(error);
            return;
        }
        break;
    case DialogSettings::UdpUnicast:
        written = m_udp->writeDatagram(data, QHostAddress(m_settings.host), m_settings.port);
        if (written != data.size()) {
            const QString error = tr("Ошибка записи в 'UDP:%1:%2'!\nError: '%3'").arg(m_settings.host).arg(m_settings.port).arg(m_serial->errorString());
            showWriteError(error);
            return;
        }
        break;
    case DialogSettings::UdpBroadcast:
        written = m_udp->writeDatagram(data, QHostAddress::Broadcast, m_settings.port);
        if (written != data.size()) {
            const QString error = tr("Ошибка записи в 'UDP:%1'!\nError: '%2'").arg(m_settings.port).arg(m_serial->errorString());
            showWriteError(error);
            return;
        }
        break;
    default: // DialogSettings::Serial
        written = m_serial->write(data);
        if (written != data.size()) {
            const QString error = tr("Ошибка записи в порт '%1'!\nError: '%2'").arg(m_serial->portName(), m_serial->errorString());
            showWriteError(error);
            return;
        }
        m_bytesToWrite += written;
        m_timerWrite->start(m_settings.timeoutWrite);
    }
    if (m_settings.localEcho) m_console->putData(convertData(data));
}

void MainWindow::serialReadyRead() {
    const QByteArray data = m_serial->readAll();
    m_console->putData(convertData(data));
}

void MainWindow::serialErrorOccurred(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Критическая ошибка"), m_serial->errorString());
        close();
    }
}

void MainWindow::showSettings() {
    if (isOpen()) close();
    m_settings.dtr = m_ui->actionDtr->isChecked();
    m_settings.rts = m_ui->actionRts->isChecked();
    DialogSettings ds(m_settings, this);
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.beginGroup(strSettings);
    ds.restoreGeometry(settings.value(strGeometry).toByteArray());
    if (ds.exec() == QDialog::Accepted) {
        m_settings = ds.settings();
        settings.setValue(strGeometry, ds.saveGeometry());
        open();
    }
    settings.endGroup();
}

void MainWindow::selectFont() {
    m_console->setFont(QFontDialog::getFont(0, m_console->font()));
}

void MainWindow::consoleContextMenu(const QPoint &pos) {
    QMenu menu(this);
    menu.addAction(m_ui->actionCopy);
    menu.addAction(m_ui->actionPaste);
    menu.addAction(m_ui->actionClear);
    menu.addSeparator();
    menu.addAction(m_ui->actionConnect);
    menu.addAction(m_ui->actionDisconnect);
    menu.addAction(m_ui->actionSettings);
    menu.addSeparator();
    menu.addMenu(m_ui->menuSend);
    menu.addMenu(m_ui->menuLoop);
    menu.exec(m_console->mapToGlobal(pos));
}

void MainWindow::socketStateUpdate(QAbstractSocket::SocketState state) {
    QString status;
    switch (m_settings.type) {
    case DialogSettings::Tcp:
        status.append(QString("%1:%2").arg(m_settings.host).arg(m_settings.port));
        break;
    case DialogSettings::UdpUnicast:
        status.append(QString("UDP:%1:%2").arg(m_settings.host).arg(m_settings.port));
        break;
    case DialogSettings::UdpBroadcast:
        status.append(QString("UDP:Broadcast:%1").arg(m_settings.port));
        break;
    default:
        return;
    }
    switch (state) {
    case QAbstractSocket::UnconnectedState: // The socket is not connected.
        status.append(statusSeparator).append(tr("Отключен"));
        break;
    case QAbstractSocket::HostLookupState:  // The socket is performing a host name lookup.
        status.append(statusSeparator).append(tr("Поиск адреса"));
        break;
    case QAbstractSocket::ConnectingState:  // The socket has started establishing a connection.
        status.append(statusSeparator).append(tr("Подключение"));
        break;
    case QAbstractSocket::ConnectedState:   // A connection is established.
    case QAbstractSocket::ListeningState:   // For internal use only.
        status.append(statusSeparator).append(tr("Подключен"));
        break;
    case QAbstractSocket::BoundState:       // The socket is bound to an address and port.
        status.append(statusSeparator).append(tr("Привязан к адресу"));
        break;
    case QAbstractSocket::ClosingState:     // The socket is about to close (data may still be waiting to be written).
        status.append(statusSeparator).append(tr("Отключен"));
        break;
    }
    updateStatus(status);
}

void MainWindow::socketErrorOccurred(QAbstractSocket::SocketError error) {
    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        QMessageBox::warning(this, tr("TCP-сокет"), tr("TCP-сервер закрыл соединение."));
        break;
    case QAbstractSocket::HostNotFoundError:
        openError(tr("Адрес не найден. Проверьте настройки TCP-сокета."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        openError(tr("Соединение отклонено. Проверьте настройки TCP-сокета."));
        break;
    default:
        QMessageBox::warning(this, tr("TCP-сокет"), tr("Произошла ошибка: %1.").arg(m_tcp->errorString()));
    }
    disconnected();
}

void MainWindow::socketReadyRead() {
    const QByteArray data = m_tcp->readAll();
    m_console->putData(convertData(data));
}

void MainWindow::udpReadyRead() {
    QByteArray datagram;
    while (m_udp->hasPendingDatagrams()) {
        datagram.resize(qsizetype(m_udp->pendingDatagramSize()));
        m_udp->readDatagram(datagram.data(), datagram.size());
        m_console->putData(convertData(datagram));
    }
}

void MainWindow::bytesWritten(qint64 bytes) {
    m_bytesToWrite -= bytes;
    if (m_bytesToWrite == 0) m_timerWrite->stop();
}

void MainWindow::writeTimeout() {
    const QString error = tr("Таймаут записи в порт '%1'.\nОшибка: %2").arg(m_serial->portName(), m_serial->errorString());
    showWriteError(error);
}

void MainWindow::serialStateUpdate() {
    QString status = m_settings.name;
    status.append(statusSeparator).append(m_serial->isOpen() ? tr("Открыт") : tr("Закрыт"));
    status.append(statusSeparator).append(QString::number(m_serial->baudRate()));
    status.append(statusSeparator).append(QString::number(m_serial->dataBits()));
    switch (m_serial->parity()) {
    case QSerialPort::NoParity: status.append('N');break;
    case QSerialPort::EvenParity: status.append('E');break;
    case QSerialPort::OddParity: status.append('O');break;
    case QSerialPort::SpaceParity: status.append('S');break;
    case QSerialPort::MarkParity: status.append('M');break;
    }
    switch (m_serial->stopBits()) {
    case QSerialPort::OneStop: status.append(QStringLiteral("1"));break;
    case QSerialPort::OneAndHalfStop: status.append(QStringLiteral("1.5"));break;
    case QSerialPort::TwoStop: status.append(QStringLiteral("2"));break;
    }
    switch (m_serial->flowControl()) {
    case QSerialPort::NoFlowControl: status.append('N');break;
    case QSerialPort::HardwareControl: status.append('H');break;
    case QSerialPort::SoftwareControl: status.append('S');break;
    }
    updateStatus(status);
}

void MainWindow::updateStatus(QString &status) {
    m_labelStatus->setText(status);
    setWindowTitle(QString("%1 - %2 v%3").arg(status, QCoreApplication::applicationName(), QCoreApplication::applicationVersion()));
}

void MainWindow::showWriteError(const QString &message) {
    QMessageBox::warning(this, tr("Ошибка записи"), message);
}

void MainWindow::readSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    settings.beginGroup(strFind);
    m_find->restoreGeometry(settings.value(strGeometry).toByteArray());
    m_find->setOpacity(settings.value(strOpacity, 1.0).toDouble());
    settings.endGroup();

    settings.beginGroup(strEnumerate);
    m_ui->lineEditEnumerateFormat->setText(settings.value(strFormat, defaultAddrFormat).toString());
    m_ui->comboBoxEnumerateType->setCurrentIndex(settings.value(strType, 0).toInt());
    m_ui->spinBoxEnumerateDigits->setValue(settings.value(strDigits, 2).toInt());
    m_ui->spinBoxEnumerateFrom->setValue(settings.value(strFrom, 0).toInt());
    m_ui->spinBoxEnumerateTo->setValue(settings.value(strTo, 99).toInt());
    m_ui->spinBoxEnumerateInterval->setValue(settings.value(strInterval, 50).toInt());
    m_ui->comboBoxEnumerateCrc->setCurrentIndex(settings.value(strCrc, 0).toInt());
    settings.endGroup();

    settings.beginGroup(strCommands);
    for (int i = 0; i < m_commandControls.size(); ++i) {
        m_commandControls[i].lineEditCommand->setText(settings.value(QString(strValueNum).arg(i+1), defaultCommand[i%COMMAND_HOT_COUNT]).toString());
        m_commandControls[i].comboBoxCrc->setCurrentIndex(settings.value(QString(strCrcNum).arg(i+1), 0).toInt());
        m_commandControls[i].spinBoxInterval->setValue(settings.value(QString(strIntervalNum).arg(i+1), 1000).toInt());
    }
    m_ui->checkBoxCrc->setChecked(settings.value(strCrc, true).toBool());
    m_ui->checkBoxInterval->setChecked(settings.value("Interval", true).toBool());
    settings.endGroup();

    settings.beginGroup(strSettings);
    m_settings.type = static_cast<DialogSettings::ConnectionType>(settings.value(strType, DialogSettings::Serial).toInt());
    m_settings.timeoutWrite = settings.value(strTimeoutWrite, DEFAULT_TIMEOUT_WRITE).toInt();
    m_settings.name = settings.value(strName, "").toString();
    m_settings.baudRate = settings.value(strBaud, QSerialPort::Baud38400).toInt();
    m_settings.dataBits = static_cast<QSerialPort::DataBits>(settings.value(strDataBits, QSerialPort::Data8).toInt());
    m_settings.parity = static_cast<QSerialPort::Parity>(settings.value(strParity, QSerialPort::NoParity).toInt());
    m_settings.stopBits = static_cast<QSerialPort::StopBits>(settings.value(strStopBits, QSerialPort::OneStop).toInt());
    m_settings.flowControl = static_cast<QSerialPort::FlowControl>(settings.value(strFlowControl, QSerialPort::NoFlowControl).toInt());
    m_settings.dtr = settings.value(strDtr, false).toBool();
    m_settings.rts = settings.value(strRts, false).toBool();
    m_settings.host = settings.value(strHost, DEFAULT_HOST).toString();
    m_settings.port = settings.value(strPort, DEFAULT_PORT).toUInt();
    m_settings.hexLog = settings.value(strHexLog, false).toBool();
    m_settings.hexAll = settings.value(strHexAll, false).toBool();
    m_settings.linefeed = settings.value(strLinefeed, true).toBool();
    m_settings.linefeedChar = settings.value(strLinefeedChar, DEFAULT_LINEFEED_CHAR).toUInt();
    m_settings.localEcho = settings.value(strLocalEcho, true).toBool();
    m_settings.timeStamp = settings.value(strTimeStamp, false).toBool();
    settings.endGroup();

    settings.beginGroup(strWindow);
    restoreGeometry(settings.value(strGeometry).toByteArray());
    restoreState(settings.value(strState).toByteArray());
    QString s = settings.value(strFont, m_console->font().toString()).toString();
    QFont f;
    if (f.fromString(s)) m_console->setFont(f);
    settings.endGroup();

    m_dir = settings.value(strDirectory, false).toString();

    if (settings.value(strConnected, false).toBool()) open();
}

void MainWindow::writeSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    settings.beginGroup(strWindow);
    settings.setValue(strGeometry, saveGeometry());
    settings.setValue(strState, saveState());
    settings.setValue(strFont, m_console->font().toString());
    settings.endGroup();

    settings.beginGroup(strFind);
    settings.setValue(strGeometry, m_find->saveGeometry());
    settings.setValue(strOpacity, m_find->windowOpacity());
    settings.endGroup();

    settings.beginGroup(strEnumerate);
    settings.setValue(strFormat, m_ui->lineEditEnumerateFormat->text());
    settings.setValue(strType, m_ui->comboBoxEnumerateType->currentIndex());
    settings.setValue(strDigits, m_ui->spinBoxEnumerateDigits->value());
    settings.setValue(strFrom, m_ui->spinBoxEnumerateFrom->value());
    settings.setValue(strTo, m_ui->spinBoxEnumerateTo->value());
    settings.setValue(strInterval, m_ui->spinBoxEnumerateInterval->value());
    settings.setValue(strCrc, m_ui->comboBoxEnumerateCrc->currentIndex());
    settings.endGroup();

    settings.beginGroup(strCommands);
    settings.setValue(strCount, m_commandControls.size());
    for (int i = 0; i < m_commandControls.size(); ++i) {
        settings.setValue(QString(strValueNum).arg(i+1), m_commandControls[i].lineEditCommand->text());
        settings.setValue(QString(strCrcNum).arg(i+1), m_commandControls[i].comboBoxCrc->currentIndex());
        settings.setValue(QString(strIntervalNum).arg(i+1), m_commandControls[i].spinBoxInterval->value());
    }
    settings.setValue(strCrc, m_ui->checkBoxCrc->isChecked());
    settings.setValue(strInterval, m_ui->checkBoxInterval->isChecked());
    settings.endGroup();

    settings.beginGroup(strSettings);
    settings.setValue(strType, m_settings.type);
    settings.setValue(strTimeoutWrite, m_settings.timeoutWrite);
    settings.setValue(strName, m_settings.name);
    settings.setValue(strBaud, m_settings.baudRate);
    settings.setValue(strDataBits, m_settings.dataBits);
    settings.setValue(strParity, m_settings.parity);
    settings.setValue(strStopBits, m_settings.stopBits);
    settings.setValue(strFlowControl, m_settings.flowControl);
    settings.setValue(strDtr, m_settings.dtr);
    settings.setValue(strRts, m_settings.rts);
    settings.setValue(strHost, m_settings.host);
    settings.setValue(strPort, m_settings.port);
    settings.setValue(strHexLog, m_settings.hexLog);
    settings.setValue(strHexAll, m_settings.hexAll);
    settings.setValue(strLinefeed, m_settings.linefeed);
    settings.setValue(strLinefeedChar, m_settings.linefeedChar);
    settings.setValue(strLocalEcho, m_settings.localEcho);
    settings.setValue(strTimeStamp, m_settings.timeStamp);
    settings.endGroup();

    settings.setValue(strDirectory, m_dir);

    settings.setValue(strConnected, isOpen());
}

void MainWindow::readSerialSignals() {
    auto ps = m_serial->pinoutSignals();
    m_labelLedCts->setLed(ps & QSerialPort::ClearToSendSignal);
    m_labelLedDsr->setLed(ps & QSerialPort::DataSetReadySignal);
    m_labelLedCd->setLed(ps & QSerialPort::DataCarrierDetectSignal);
    m_labelLedRi->setLed(ps & QSerialPort::RingIndicatorSignal);
    m_labelLedStd->setLed(ps & QSerialPort::SecondaryTransmittedDataSignal);
    m_labelLedSrd->setLed(ps & QSerialPort::SecondaryReceivedDataSignal);
}

bool MainWindow::isOpen() const {
    switch (m_settings.type) {
    case DialogSettings::Tcp: return m_tcp->isOpen(); break;
    case DialogSettings::UdpUnicast:
    case DialogSettings::UdpBroadcast: return (m_udp->state() == QAbstractSocket::BoundState);
    default: return m_serial->isOpen();
    }
}
