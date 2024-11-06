#include "settings.h"
#include "ui_settings.h"

#include <QIntValidator>
#include <QLineEdit>
#include <QSerialPortInfo>

static const char blankString[] = QT_TR_NOOP("N/A");

DialogSettings::DialogSettings(Settings &settings, QWidget *parent):
    QDialog(parent),
    m_ui(new Ui::DialogSettings),
    m_intValidator(new QIntValidator(0, 12000000, this))
{
    m_ui->setupUi(this);

    connect(m_ui->pushButtonOpen, &QPushButton::clicked, this, &DialogSettings::apply);
    connect(m_ui->pushButtonCancel, &QPushButton::clicked, this, &QDialog::close);
    connect(m_ui->pushButtonDefault, &QPushButton::clicked, this, &DialogSettings::setDefault);

    connect(m_ui->comboBoxPort, &QComboBox::currentIndexChanged, this, &DialogSettings::showPortInfo);
    connect(m_ui->comboBoxPort, &QComboBox::activated, this, &DialogSettings::checkCustomDevicePathPolicy);
    connect(m_ui->comboBoxBaudRate,  &QComboBox::activated, this, &DialogSettings::checkCustomBaudRatePolicy);

    connect(m_ui->comboBoxType,  &QComboBox::currentIndexChanged, this, &DialogSettings::setType);

    fillParameters();
    fillPortsInfo();
    setSettings(settings);
}

DialogSettings::~DialogSettings() {
    delete m_ui;
}

DialogSettings::Settings DialogSettings::settings() const {
    return m_currentSettings;
}

void DialogSettings::showPortInfo(int idx) {
    if (idx < 0) return;
    const QString blankString = tr(::blankString);
    const QStringList list = m_ui->comboBoxPort->itemData(idx).toStringList();

    m_ui->lineEditDescription->setText(list.value(1, blankString));
    m_ui->lineEditManufacturer->setText(list.value(2, blankString));
    m_ui->lineEditSerialNumber->setText(list.value(3, blankString));
    m_ui->lineEditLocation->setText(list.value(4, blankString));
    m_ui->lineEditVid->setText(list.value(5, blankString));
    m_ui->lineEditPid->setText(list.value(6, blankString));
}

void DialogSettings::apply() {
    updateSettings();
    accept();
}

void DialogSettings::checkCustomDevicePathPolicy(int idx) {
    const bool isCustomPath = !m_ui->comboBoxPort->itemData(idx).isValid();
    m_ui->comboBoxPort->setEditable(isCustomPath);
    if (isCustomPath) m_ui->comboBoxPort->clearEditText();
}

void DialogSettings::checkCustomBaudRatePolicy(int idx) {
    const bool isCustomBaudRate = !m_ui->comboBoxBaudRate->itemData(idx).isValid();
    m_ui->comboBoxBaudRate->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        m_ui->comboBoxBaudRate->clearEditText();
        m_ui->comboBoxBaudRate->setValidator(m_intValidator);
    }
}

void DialogSettings::setDefault() {
    m_currentSettings.type = ConnectionType::Serial;
    //m_currentSettings.name = m_currentSettings.name;
    m_currentSettings.baudRate = QSerialPort::Baud38400;
    m_currentSettings.dataBits = QSerialPort::Data8;
    m_currentSettings.parity = QSerialPort::NoParity;
    m_currentSettings.stopBits = QSerialPort::OneStop;
    m_currentSettings.flowControl = QSerialPort::NoFlowControl;
    m_currentSettings.dtr = false;
    m_currentSettings.rts = false;
    m_currentSettings.localEcho = true;
    m_currentSettings.timeStamp = false;
    m_currentSettings.hexLog = false;
    m_currentSettings.hexAll = false;
    setSettings(m_currentSettings);
}

void DialogSettings::setType(int idx) {
    bool enable = (idx == 0);
    m_ui->comboBoxPort->setEnabled(enable);
    m_ui->comboBoxBaudRate->setEnabled(enable);
    m_ui->comboBoxDataBits->setEnabled(enable);
    m_ui->comboBoxParity->setEnabled(enable);
    m_ui->comboBoxStopBits->setEnabled(enable);
    m_ui->comboBoxFlowControl->setEnabled(enable);

    m_ui->lineEditTcpHost->setEnabled(!enable && (idx!=3)); // Broadcast
    m_ui->spinBoxTcpPort->setEnabled(!enable);

}

void DialogSettings::fillPortsInfo() {
    m_ui->comboBoxPort->clear();
    m_ui->comboBoxPort->setInsertPolicy(QComboBox::NoInsert);

    const QString blankString = tr(::blankString);
    const auto infos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        const QString description = info.description();
        const QString manufacturer = info.manufacturer();
        const QString serialNumber = info.serialNumber();
        const auto vendorId = info.vendorIdentifier();
        const auto productId = info.productIdentifier();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (vendorId ? QString::number(vendorId, 16) : blankString)
             << (productId ? QString::number(productId, 16) : blankString);

        m_ui->comboBoxPort->addItem(list.constFirst(), list);
    }

    m_ui->comboBoxPort->addItem(tr("Другой"));
}

void DialogSettings::fillParameters() {
    // type
    m_ui->comboBoxType->addItem(tr("Последовательный порт"), ConnectionType::Serial);
    m_ui->comboBoxType->addItem(tr("TCP-сокет"), ConnectionType::Tcp);
    m_ui->comboBoxType->addItem(tr("UDP Unicast"), ConnectionType::UdpUnicast);
    m_ui->comboBoxType->addItem(tr("UDP Broadcast"), ConnectionType::UdpBroadcast);

    // serial
    m_ui->comboBoxBaudRate->addItem(QString::number(QSerialPort::Baud1200), QSerialPort::Baud1200);
    m_ui->comboBoxBaudRate->addItem(QString::number(QSerialPort::Baud2400), QSerialPort::Baud2400);
    m_ui->comboBoxBaudRate->addItem(QString::number(QSerialPort::Baud4800), QSerialPort::Baud4800);
    m_ui->comboBoxBaudRate->addItem(QString::number(QSerialPort::Baud9600), QSerialPort::Baud9600);
    m_ui->comboBoxBaudRate->addItem(QString::number(QSerialPort::Baud19200), QSerialPort::Baud19200);
    m_ui->comboBoxBaudRate->addItem(QString::number(QSerialPort::Baud38400), QSerialPort::Baud38400);
    m_ui->comboBoxBaudRate->addItem(QString::number(QSerialPort::Baud57600), QSerialPort::Baud57600);
    m_ui->comboBoxBaudRate->addItem(QString::number(QSerialPort::Baud115200), QSerialPort::Baud115200);
    m_ui->comboBoxBaudRate->addItem(tr("Другой"));
    m_ui->comboBoxBaudRate->setInsertPolicy(QComboBox::NoInsert);

    m_ui->comboBoxDataBits->addItem(QStringLiteral("5"), QSerialPort::Data5);
    m_ui->comboBoxDataBits->addItem(QStringLiteral("6"), QSerialPort::Data6);
    m_ui->comboBoxDataBits->addItem(QStringLiteral("7"), QSerialPort::Data7);
    m_ui->comboBoxDataBits->addItem(QStringLiteral("8"), QSerialPort::Data8);

    m_ui->comboBoxParity->addItem(tr("Нет"), QSerialPort::NoParity);
    m_ui->comboBoxParity->addItem(tr("Чётная"), QSerialPort::EvenParity);
    m_ui->comboBoxParity->addItem(tr("Нечётная"), QSerialPort::OddParity);
    m_ui->comboBoxParity->addItem(tr("Пробел"), QSerialPort::SpaceParity);
    m_ui->comboBoxParity->addItem(tr("Маркер"), QSerialPort::MarkParity);

    m_ui->comboBoxStopBits->addItem(QStringLiteral("1"), QSerialPort::OneStop);
    m_ui->comboBoxStopBits->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
    m_ui->comboBoxStopBits->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    m_ui->comboBoxFlowControl->addItem(tr("Нет"), QSerialPort::NoFlowControl);
    m_ui->comboBoxFlowControl->addItem(tr("Аппаратное"), QSerialPort::HardwareControl);
    m_ui->comboBoxFlowControl->addItem(tr("Программное"), QSerialPort::SoftwareControl);
}

void DialogSettings::setSettings(Settings value) {
    m_currentSettings = value;

    // type
    switch (m_currentSettings.type) {
    case ConnectionType::Tcp: m_ui->comboBoxType->setCurrentIndex(1); break;
    case ConnectionType::UdpUnicast: m_ui->comboBoxType->setCurrentIndex(2); break;
    case ConnectionType::UdpBroadcast: m_ui->comboBoxType->setCurrentIndex(3); break;
    default: m_ui->comboBoxType->setCurrentIndex(0); break;
    }

    //serial
    int index;
    if (m_currentSettings.name.isEmpty()) {
        m_ui->comboBoxPort->setCurrentIndex(m_ui->comboBoxPort->count() - 1);
        m_ui->comboBoxPort->setEditable(true);
        m_ui->comboBoxPort->clearEditText();
    } else {
        index = m_ui->comboBoxPort->findText(m_currentSettings.name);
        if ( index >= 0 ) {
            m_ui->comboBoxPort->setCurrentIndex(index);
        } else {
            m_ui->comboBoxPort->setCurrentIndex(m_ui->comboBoxPort->count() - 1);
            m_ui->comboBoxPort->setEditable(true);
            m_ui->comboBoxPort->setCurrentText(m_currentSettings.name);
        }
    }

    if (m_currentSettings.baudRate <= 0) {          // если не допустимый
        m_ui->comboBoxBaudRate->setCurrentIndex(5); // установим 38400
    } else {
        index = m_ui->comboBoxBaudRate->findData(m_currentSettings.baudRate);
        if ( index >= 0 ) {
            m_ui->comboBoxBaudRate->setCurrentIndex(index);
        } else {
            m_ui->comboBoxBaudRate->setCurrentIndex(m_ui->comboBoxBaudRate->count() - 1);
            m_ui->comboBoxBaudRate->setEditable(true);
            m_ui->comboBoxBaudRate->setCurrentText(QString::number(m_currentSettings.baudRate));
        }
    }

    switch (m_currentSettings.dataBits) {
    case QSerialPort::Data5: m_ui->comboBoxDataBits->setCurrentIndex(0); break;
    case QSerialPort::Data6: m_ui->comboBoxDataBits->setCurrentIndex(1); break;
    case QSerialPort::Data7: m_ui->comboBoxDataBits->setCurrentIndex(2); break;
    default: m_ui->comboBoxDataBits->setCurrentIndex(3); break;
    }

    switch (m_currentSettings.parity) {
    case QSerialPort::EvenParity: m_ui->comboBoxParity->setCurrentIndex(1); break;
    case QSerialPort::OddParity: m_ui->comboBoxParity->setCurrentIndex(2); break;
    case QSerialPort::SpaceParity: m_ui->comboBoxParity->setCurrentIndex(3); break;
    case QSerialPort::MarkParity: m_ui->comboBoxParity->setCurrentIndex(4); break;
    default: m_ui->comboBoxParity->setCurrentIndex(0); break;
    }

    switch (m_currentSettings.stopBits) {
    case QSerialPort::OneAndHalfStop: m_ui->comboBoxStopBits->setCurrentIndex(1); break;
    case QSerialPort::TwoStop: m_ui->comboBoxStopBits->setCurrentIndex(2); break;
    default: m_ui->comboBoxStopBits->setCurrentIndex(0); break;
    }

    switch (m_currentSettings.flowControl) {
    case QSerialPort::HardwareControl: m_ui->comboBoxFlowControl->setCurrentIndex(1); break;
    case QSerialPort::SoftwareControl: m_ui->comboBoxFlowControl->setCurrentIndex(2); break;
    default: m_ui->comboBoxFlowControl->setCurrentIndex(0); break;
    }

    m_ui->checkBoxDtr->setChecked(m_currentSettings.dtr);
    m_ui->checkBoxRts->setChecked(m_currentSettings.rts);

    //tcp
    m_ui->lineEditTcpHost->setText(m_currentSettings.host);
    m_ui->spinBoxTcpPort->setValue(m_currentSettings.port);

    // terminal
    m_ui->checkBoxLocalEcho->setChecked(m_currentSettings.localEcho);
    m_ui->checkBoxTimeStamp->setChecked(m_currentSettings.timeStamp);
    m_ui->groupBoxHexLog->setChecked(m_currentSettings.hexLog);
    if (m_currentSettings.hexAll) {
        m_ui->radioButtonHexAll->setChecked(true);
    } else {
        m_ui->radioButtonHexUnprint->setChecked(true);
    }
}

void DialogSettings::updateSettings() {
    // type
    const auto type = m_ui->comboBoxType->currentData();
    m_currentSettings.type = type.value<ConnectionType>();

    // serial
    m_currentSettings.name = m_ui->comboBoxPort->currentText();

    if (m_ui->comboBoxBaudRate->currentIndex() == (m_ui->comboBoxBaudRate->count()-1)) {
        m_currentSettings.baudRate = m_ui->comboBoxBaudRate->currentText().toInt();
    } else {
        const auto baudRateData = m_ui->comboBoxBaudRate->currentData();
        m_currentSettings.baudRate = baudRateData.value<QSerialPort::BaudRate>();
    }

    const auto dataBitsData = m_ui->comboBoxDataBits->currentData();
    m_currentSettings.dataBits = dataBitsData.value<QSerialPort::DataBits>();

    const auto parityData = m_ui->comboBoxParity->currentData();
    m_currentSettings.parity = parityData.value<QSerialPort::Parity>();

    const auto stopBitsData = m_ui->comboBoxStopBits->currentData();
    m_currentSettings.stopBits = stopBitsData.value<QSerialPort::StopBits>();

    const auto flowControlData = m_ui->comboBoxFlowControl->currentData();
    m_currentSettings.flowControl = flowControlData.value<QSerialPort::FlowControl>();

    m_currentSettings.dtr = m_ui->checkBoxDtr->isChecked();
    m_currentSettings.rts = m_ui->checkBoxRts->isChecked();

    // tcp
    m_currentSettings.host = m_ui->lineEditTcpHost->text();
    m_currentSettings.port = m_ui->spinBoxTcpPort->value();

    // terminal
    m_currentSettings.localEcho = m_ui->checkBoxLocalEcho->isChecked();
    m_currentSettings.timeStamp = m_ui->checkBoxTimeStamp->isChecked();
    m_currentSettings.hexLog = m_ui->groupBoxHexLog->isChecked();
    m_currentSettings.hexAll = m_ui->radioButtonHexAll->isChecked();
}
