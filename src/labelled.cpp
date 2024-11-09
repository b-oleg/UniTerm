#include "labelled.h"

LabelLed::LabelLed(QWidget *parent, const QString &text, const bool value):
    QLabel{text, parent},
    m_led(value),
    m_text(text)
{
    setTextFormat(Qt::RichText);
    change();
}

bool LabelLed::led() const {
    return m_led;
}

void LabelLed::setLed(bool value) {
    if (m_led == value) return;
    m_led = value;
    change();
    emit ledChanged();
}

QString LabelLed::text() const {
    return m_text;
}

void LabelLed::setText(const QString &value) {
    if (m_text == value) return;
    m_text = value;
    change();
    emit textChanged();
}

void LabelLed::change() {
    QLabel::setText(QString("<img src=':/ico/led-%1.png'>%2").arg(m_led ? QStringLiteral("on") : QStringLiteral("off")).arg(m_text));
}
