#ifndef CRC_H
#define CRC_H

#include <QObject>

class Crc : public QObject
{
    Q_OBJECT
public:
    explicit Crc(QObject *parent = nullptr);

    QStringList list();                             // список доступных

    QByteArray addCrc(QByteArray &data, uint idx);  // добавить crc

};

#endif // CRC_H
