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

#define COMMAND_COUNT   10

const char* defaultCommand[COMMAND_COUNT]  = { ":01G0\\0d", ":02G0\\0d", ":03G0\\0d", ":04G0\\0d", ":05G0\\0d", ":06G0\\0d", ":07G0\\0d", ":08G0\\0d", ":09G0\\0d", ":10G0\\0d"};
const char* defaultShortcutSend[COMMAND_COUNT]  = { "Alt+1", "Alt+2", "Alt+3", "Alt+4", "Alt+5", "Alt+6", "Alt+7", "Alt+8", "Alt+9", "Alt+0" };
const char* defaultShortcutLoop[COMMAND_COUNT]  = { "Shift+1", "Shift+2", "Shift+3", "Shift+4", "Shift+5", "Shift+6", "Shift+7", "Shift+8", "Shift+9", "Shift+0" };
const char* defaultAddrFormat = ":\\#G0\\0d";

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_console(new Console(this)),
    m_find(new DialogFind(m_console, this)),
    m_labelStatus(new QLabel(this)),
    m_timer(new QTimer(this)),
    m_timerAddr(new QTimer(this)),
    m_serial(new QSerialPort(this))
{
    m_ui->setupUi(this);
    setCentralWidget(m_console);

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);

    m_ui->statusBar->addPermanentWidget(m_labelStatus);

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
    setToolStatusTip(m_ui->actionSendBreak);
    setToolStatusTip(m_ui->actionDtr);
    setToolStatusTip(m_ui->actionRts);
    setToolStatusTip(m_ui->actionAbout);

    connect(m_ui->actionOpen, &QAction::triggered, this, &MainWindow::open);
    connect(m_ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);

    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionSettings, &QAction::triggered, this, &MainWindow::showSettings);

    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    //connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

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
        if (m_serial->isOpen()) {
            writeData(QApplication::clipboard()->mimeData()->text().toLocal8Bit());
        } else {
            showSettings();
            return;
        }
    });

    // serial
    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::handleWriteTimeout);
    m_timer->setSingleShot(true);

    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(m_serial, &QSerialPort::bytesWritten, this, &MainWindow::handleBytesWritten);
    connect(m_console, &Console::getData, this, &MainWindow::writeData);

    connect(m_serial, &QSerialPort::dataTerminalReadyChanged, m_ui->actionDtr, &QAction::setChecked);
    connect(m_ui->actionDtr, &QAction::toggled, m_serial, &QSerialPort::setDataTerminalReady);
    connect(m_serial, &QSerialPort::requestToSendChanged, m_ui->actionRts, &QAction::setChecked);
    connect(m_ui->actionRts, &QAction::toggled, m_serial, &QSerialPort::setRequestToSend);
    connect(m_ui->actionSendBreak, &QAction::triggered, this, [=](){
        if (m_serial->isOpen()) writeData(QByteArray(1,0));
    });

    // dock commands
    m_ui->labelNum->setToolTip(tr("Номер команды"));
    m_ui->labelNum->setStatusTip(m_ui->labelNum->toolTip());
    m_ui->labelCommand->setToolTip(tr("Команда"));
    m_ui->labelCommand->setStatusTip(m_ui->labelCommand->toolTip());
    m_ui->labelSend->setToolTip(tr("Отправить команду"));
    m_ui->labelSend->setStatusTip(m_ui->labelSend->toolTip());
    m_ui->labelInterval->setToolTip(tr("Интервал отправки команды в циклическом режиме"));
    m_ui->labelInterval->setStatusTip(m_ui->labelInterval->toolTip());
    m_ui->labelSendInterval->setToolTip(tr("Отправлять команду циклически"));
    m_ui->labelSendInterval->setStatusTip(m_ui->labelSendInterval->toolTip());

    // commands
    m_commandControls.resize(COMMAND_COUNT);
    for (int i = 0; i < COMMAND_COUNT; ++i) {

        // dock
        m_commandControls[i].labelNum = new QLabel(this);
        m_commandControls[i].labelNum->setText(QString::number(i+1));
        m_commandControls[i].labelNum->setToolTip(m_ui->labelNum->toolTip());
        m_commandControls[i].labelNum->setStatusTip(m_ui->labelNum->toolTip());

        m_commandControls[i].lineEditCommand = new QLineEdit(this);
        m_commandControls[i].lineEditCommand->setStatusTip(QString(tr("Команда №%1")).arg(i+1));
        m_commandControls[i].lineEditCommand->setToolTip(m_commandControls[i].lineEditCommand->statusTip() + "\n" + m_ui->labelCommandsInfo->text());

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
        m_ui->gridLayoutCommands->addWidget(m_commandControls[i].actionButtonSend, i+1, 2);
        m_ui->gridLayoutCommands->addWidget(m_commandControls[i].spinBoxInterval, i+1, 3);
        m_ui->gridLayoutCommands->addWidget(m_commandControls[i].actionButtonSendInterval, i+1, 4);

        // action send
        m_commandControls[i].actionSend = new QAction(QString(tr("Команда №%1")).arg(i+1), this);
        m_commandControls[i].actionSend->setIcon(QIcon(QStringLiteral(":/ico/%1.png").arg(i+1)));
        m_commandControls[i].actionSend->setShortcut(QKeySequence(defaultShortcutSend[i]));
        m_commandControls[i].actionSend->setEnabled(false);
        m_commandControls[i].actionSend->setToolTip(QString(tr("Отправить команду №%1 (%2)")).arg(i+1).arg(m_commandControls[i].actionSend->shortcut().toString()));
        m_commandControls[i].actionSend->setStatusTip(m_commandControls[i].actionSend->toolTip());
        m_ui->menuSend->addAction(m_commandControls[i].actionSend);
        m_ui->toolBarCommand->addAction(m_commandControls[i].actionSend);

        connect(m_commandControls[i].actionSend, &QAction::triggered, this, [=]() {
            QByteArray cmd = strToCmd(m_commandControls[i].lineEditCommand->text());
            writeData(cmd);
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
        m_commandControls[i].actionSendInterval->setIcon(QIcon(QString(":/ico/%1-loop.png").arg(i+1)));
        m_commandControls[i].actionSendInterval->setShortcut(QKeySequence(defaultShortcutLoop[i]));
        m_commandControls[i].actionSendInterval->setEnabled(false);
        m_commandControls[i].actionSendInterval->setCheckable(true);
        m_commandControls[i].actionSendInterval->setToolTip(QString(tr("Циклически отправлять команду №%1 (%2)")).arg(i+1).arg(m_commandControls[i].actionSendInterval->shortcut().toString()));
        m_commandControls[i].actionSendInterval->setStatusTip(m_commandControls[i].actionSendInterval->toolTip());
        m_ui->menuLoop->addAction(m_commandControls[i].actionSendInterval);
        m_ui->toolBarCommandLoop->addAction(m_commandControls[i].actionSendInterval);
        connect(m_commandControls[i].actionSendInterval, &QAction::toggled, this, [=](bool checked) {
            if (checked) {
                m_commandControls[i].timer->start(m_commandControls[i].spinBoxInterval->value());
            } else {
                m_commandControls[i].timer->stop();
            }
        });
        m_commandControls[i].actionButtonSendInterval->setAction(m_commandControls[i].actionSendInterval);
    }

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
    m_ui->pushButtonStart->setToolTip(m_ui->pushButtonStart->statusTip());
    m_ui->pushButtonStop->setToolTip(m_ui->pushButtonStop->statusTip());

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
            writeData(addrToCmd(m_ui->lineEditEnumerateFormat->text()));
            m_addr++;
        } else {
            m_ui->pushButtonStop->click();
        }
    });

    // context menu
    connect(m_console, &Console::customContextMenuRequested, this, &MainWindow::consoleContextMenu);
    m_console->setContextMenuPolicy(Qt::CustomContextMenu);

    //
    readSettings();
    updateStatus();
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

    qDebug()<<bin;
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

void MainWindow::open() {
    QFileDialog dialog(this, tr("Открыть"), ".", tr("Текстовый документ (*.txt)"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (dialog.exec() == QDialog::Accepted) {
        QFile file(dialog.selectedFiles().constFirst());
        if (file.open(QIODevice::ReadOnly)) {
            m_console->clear();
            QString content = QString::fromLocal8Bit(file.readAll());
            m_console->setPlainText(content);
            file.close();
        }
    }
}

void MainWindow::saveAs() {
    QFileDialog dialog(this, tr("Сохранить как..."), ".", tr("Текстовый документ (*.txt)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.selectFile(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss"));
    dialog.setFileMode(QFileDialog::AnyFile);
    if (dialog.exec() == QDialog::Accepted) {
        QFile file(dialog.selectedFiles().constFirst());
        if (file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            out << m_console->toPlainText();
            file.close();
        }
    }
}

void MainWindow::openSerialPort() {
    m_serial->setPortName(m_settings.name);
    m_serial->setBaudRate(m_settings.baudRate);
    m_serial->setDataBits(m_settings.dataBits);
    m_serial->setParity(m_settings.parity);
    m_serial->setStopBits(m_settings.stopBits);
    m_serial->setFlowControl(m_settings.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        m_serial->setDataTerminalReady(m_settings.dtr);
        m_ui->actionDtr->setChecked(m_settings.dtr);
        m_serial->setRequestToSend(m_settings.rts);
        m_ui->actionRts->setChecked(m_settings.rts);
        //m_console->setEnabled(true);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionDtr->setEnabled(true);
        m_ui->actionRts->setEnabled(true);
        m_ui->actionSendBreak->setEnabled(true);
        //showStatusMessage(tr("Подключен %1: %2, %3, %4, %5, %6").arg(m_settings.name, m_settings.stringBaudRate, m_settings.stringDataBits, m_settings.stringParity, m_settings.stringStopBits, m_settings.stringFlowControl));
        for (int i = 0; i < COMMAND_COUNT; ++i) {
            m_commandControls[i].actionButtonSend->setEnabled(true);
            m_commandControls[i].actionButtonSendInterval->setEnabled(true);
            m_commandControls[i].actionSend->setEnabled(true);
            m_commandControls[i].actionSendInterval->setEnabled(true);
        }
    } else {
        QMessageBox::critical(this, tr("Ошибка открытия порта!"), m_serial->errorString());
    }
    updateStatus();
}

void MainWindow::closeSerialPort() {
    if (m_serial->isOpen()) m_serial->close();
    //m_console->setEnabled(false);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionDtr->setEnabled(false);
    m_ui->actionRts->setEnabled(false);
    m_ui->actionSendBreak->setEnabled(false);
    for (int i = 0; i < COMMAND_COUNT; ++i) {
        m_commandControls[i].actionSend->setEnabled(false);
        m_commandControls[i].actionSendInterval->setChecked(false);
        m_commandControls[i].actionSendInterval->setEnabled(false);
        m_commandControls[i].actionButtonSend->setEnabled(false);
        m_commandControls[i].actionButtonSendInterval->setEnabled(false);
    }
    updateStatus();
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
               "<p>Программа предназначена для работы с COM-портом.</p>"
               "<p>Основана на кроссплатформенной библиотеке разработки <a href=\"https://www.qt.io\"><span style=\"text-decoration:underline;color:#0000ff;\">'Qt'</span></a>.</p>"
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
    for (int i=0; i<data.size(); ++i) {
        char c = data.at(i);
        if (m_settings.timeStamp) {
            if (c == 0x0D) {
                quint64 ms = QDateTime::currentMSecsSinceEpoch();
                QByteArray ts = QDateTime::fromMSecsSinceEpoch(ms).toString("\n[hh:mm:ss.zzz] - ").toLocal8Bit();
                res.append(ts);
                if (!m_settings.hexLog) continue;
            }
        }
        if (m_settings.hexLog) {
            if (m_settings.hexAll || (c < ' ')) {
                res.append(QString("<%1>").arg(QByteArray(1, c).toHex().toUpper()).toLocal8Bit());
            } else {
                res.append(c);
            }
        } else {
            res.append(c);
        }
    }
    return res;
}

void MainWindow::writeData(const QByteArray &data) {
    if (!m_serial->isOpen()) {
        showSettings();
        return;
    }
    const qint64 written = m_serial->write(data);
    qDebug()<<"serial->write"<<data.size() << written;
    if (written == data.size()) {
        if (m_settings.localEcho) m_console->putData(convertData(data));
        m_bytesToWrite += written;
        m_timer->start(5000);
    } else {
        const QString error = tr("Ошибка записи в порт '%1'!\nError: '%2'").arg(m_serial->portName(), m_serial->errorString());
        showWriteError(error);
    }
}

void MainWindow::readData() {
    const QByteArray data = m_serial->readAll();
    m_console->putData(convertData(data));
}

void MainWindow::handleError(QSerialPort::SerialPortError error) {
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Критическая ошибка"), m_serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::handleBytesWritten(qint64 bytes) {
    qDebug()<<"handleBytesWritten"<<bytes;
    m_bytesToWrite -= bytes;
    if (m_bytesToWrite == 0) m_timer->stop();
}

void MainWindow::handleWriteTimeout() {
    const QString error = tr("Таймаут записи в порт '%1'.\nОшибка: %2").arg(m_serial->portName(), m_serial->errorString());
    showWriteError(error);
}

void MainWindow::showSettings() {
    if (m_serial->isOpen()) closeSerialPort();
    m_settings.dtr = m_ui->actionDtr->isChecked();
    m_settings.rts = m_ui->actionRts->isChecked();
    DialogSettings ds(m_settings, this);
    if (ds.exec() == QDialog::Accepted) {
        m_settings = ds.settings();
        openSerialPort();
    }
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

void MainWindow::updateStatus() {
    const QString separator = QStringLiteral(" - ");
    QString status = m_settings.name;
    if (m_serial->isOpen()) {
        status.append(separator).append(QString::number(m_serial->baudRate()));
        status.append(separator).append(QString::number(m_serial->dataBits()));
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
    } else {
        status.append(separator).append(tr("Закрыт"));
    }
    m_labelStatus->setText(status);
    setWindowTitle(QString("%1 - %2 v%3").arg(status, QCoreApplication::applicationName(), QCoreApplication::applicationVersion()));
}

void MainWindow::showWriteError(const QString &message) {
    QMessageBox::warning(this, tr("Ошибка записи"), message);
}

void MainWindow::readSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    settings.beginGroup("Window");
    restoreGeometry(settings.value("Geometry").toByteArray());
    restoreState(settings.value("State").toByteArray());
    QString s = settings.value("Font", m_console->font().toString()).toString();
    QFont f;
    if (f.fromString(s)) m_console->setFont(f);
    settings.endGroup();

    settings.beginGroup("Find");
    m_find->restoreGeometry(settings.value("Geometry").toByteArray());
    m_find->setOpacity(settings.value("Opacity", 1.0).toDouble());
    settings.endGroup();

    settings.beginGroup("Enumerate");
    m_ui->lineEditEnumerateFormat->setText(settings.value("Format", defaultAddrFormat).toString());
    m_ui->comboBoxEnumerateType->setCurrentIndex(settings.value("Type", 0).toInt());
    m_ui->spinBoxEnumerateDigits->setValue(settings.value("Digits", 2).toInt());
    m_ui->spinBoxEnumerateFrom->setValue(settings.value("From", 0).toInt());
    m_ui->spinBoxEnumerateTo->setValue(settings.value("To", 99).toInt());
    m_ui->spinBoxEnumerateInterval->setValue(settings.value("Interval", 50).toInt());
    settings.endGroup();

    settings.beginGroup("Commands");
    for (int i = 0; i < COMMAND_COUNT; ++i) {
        m_commandControls[i].lineEditCommand->setText(settings.value(QString("Value%1").arg(i+1), defaultCommand[i]).toString());
        m_commandControls[i].spinBoxInterval->setValue(settings.value(QString("Interval%1").arg(i+1), 1000).toInt());
    }
    settings.endGroup();

    settings.beginGroup("Settings");
    m_settings.name = settings.value("Port", "").toString();
    m_settings.baudRate = settings.value("Baud", QSerialPort::Baud38400).toInt();
    m_settings.dataBits = static_cast<QSerialPort::DataBits>(settings.value("DataBits", QSerialPort::Data8).toInt());
    m_settings.parity = static_cast<QSerialPort::Parity>(settings.value("Parity", QSerialPort::NoParity).toInt());
    m_settings.stopBits = static_cast<QSerialPort::StopBits>(settings.value("StopBits", QSerialPort::OneStop).toInt());
    m_settings.flowControl = static_cast<QSerialPort::FlowControl>(settings.value("FlowControl", QSerialPort::NoFlowControl).toInt());
    m_settings.dtr = settings.value("DTR", false).toBool();
    m_settings.rts = settings.value("RTS", false).toBool();
    m_settings.hexLog = settings.value("HexLog", false).toBool();
    m_settings.hexAll = settings.value("HexAll", false).toBool();
    m_settings.localEcho = settings.value("LocalEcho", true).toBool();
    m_settings.timeStamp = settings.value("TimeStamp", false).toBool();
    settings.endGroup();

    if (settings.value("Connected", false).toBool()) openSerialPort();
}

void MainWindow::writeSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

    settings.beginGroup("Window");
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("State", saveState());
    settings.setValue("Font", m_console->font().toString());
    settings.endGroup();

    settings.beginGroup("Find");
    settings.setValue("Geometry", m_find->saveGeometry());
    settings.setValue("Opacity", m_find->windowOpacity());
    settings.endGroup();

    settings.beginGroup("Enumerate");
    settings.setValue("Format", m_ui->lineEditEnumerateFormat->text());
    settings.setValue("Type", m_ui->comboBoxEnumerateType->currentIndex());
    settings.setValue("Digits", m_ui->spinBoxEnumerateDigits->value());
    settings.setValue("From", m_ui->spinBoxEnumerateFrom->value());
    settings.setValue("To", m_ui->spinBoxEnumerateTo->value());
    settings.setValue("Interval", m_ui->spinBoxEnumerateInterval->value());
    settings.endGroup();

    settings.beginGroup("Commands");
    for (int i = 0; i < COMMAND_COUNT; ++i) {
        settings.setValue(QString("Value%1").arg(i+1), m_commandControls[i].lineEditCommand->text());
        settings.setValue(QString("Interval%1").arg(i+1), m_commandControls[i].spinBoxInterval->value());
    }
    settings.endGroup();

    settings.beginGroup("Settings");
    settings.setValue("Port", m_settings.name);
    settings.setValue("Baud", m_settings.baudRate);
    settings.setValue("DataBits", m_settings.dataBits);
    settings.setValue("Parity", m_settings.parity);
    settings.setValue("StopBits", m_settings.stopBits);
    settings.setValue("FlowControl", m_settings.flowControl);
    settings.setValue("DTR", m_settings.dtr);
    settings.setValue("RTS", m_settings.rts);
    settings.setValue("HexLog", m_settings.hexLog);
    settings.setValue("HexAll", m_settings.hexAll);
    settings.setValue("LocalEcho", m_settings.localEcho);
    settings.setValue("TimeStamp", m_settings.timeStamp);
    settings.endGroup();

    settings.setValue("Connected", m_serial->isOpen());
}
