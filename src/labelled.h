#ifndef LABELLED_H
#define LABELLED_H

#include <QLabel>

class LabelLed : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(bool led READ led WRITE setLed NOTIFY ledChanged FINAL)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)

public:
    explicit LabelLed(QWidget *parent=nullptr, const QString &text="", const bool value=false);

    bool led() const;
    void setLed(bool value);

    QString text() const;
    void setText(const QString &value);

signals:
    void ledChanged();
    void textChanged();

private:
    bool m_led;
    QString m_text;
    void change();

};

#endif // LABELLED_H
