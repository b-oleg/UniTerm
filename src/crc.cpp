#include "crc.h"
#include <QDataStream>
#include <QIODevice>

#define IDX_NONE                        0
#define IDX_CRC8_BIN                    1
#define IDX_CRC8_HEX                    2
#define IDX_CRC8_HEX_CR                 3
#define IDX_CRC16_BIN_LITTLE_ENDIAN     4
#define IDX_CRC16_BIN_BIG_ENDIAN        5
#define IDX_CRC16_HEX_LITTLE_ENDIAN     6
#define IDX_CRC16_HEX_LITTLE_ENDIAN_CR  7
#define IDX_CRC16_HEX_BIG_ENDIAN        8
#define IDX_CRC16_HEX_BIG_ENDIAN_CR     9
#define IDX_CRC32_BIN_LITTLE_ENDIAN     10
#define IDX_CRC32_BIN_BIG_ENDIAN        11
#define IDX_CRC32_HEX_LITTLE_ENDIAN     12
#define IDX_CRC32_HEX_LITTLE_ENDIAN_CR  13
#define IDX_CRC32_HEX_BIG_ENDIAN        14
#define IDX_CRC32_HEX_BIG_ENDIAN_CR     15
#define IDX_SUM_MOD256_BIN              16
#define IDX_SUM_MOD256_HEX              17
#define IDX_SUM_MOD256_HEX_CR           18

#define CR  0x0D

const QStringList crcList = QStringList() << "Нет"
                                          << "CRC8-BIN"
                                          << "CRC8-HEX"
                                          << "CRC8-HEX-CR"
                                          << "CRC16-BIN-LE"
                                          << "CRC16-BIN-BE"
                                          << "CRC16-HEX-LE"
                                          << "CRC16-HEX-LE-CR"
                                          << "CRC16-HEX-BE"
                                          << "CRC16-HEX-BE-CR"
                                          << "CRC32-BIN-LE"
                                          << "CRC32-BIN-BE"
                                          << "CRC32-HEX-LE"
                                          << "CRC32-HEX-LE-CR"
                                          << "CRC32-HEX-BE"
                                          << "CRC32-HEX-BE-CR"
                                          << "SUM8-BIN"
                                          << "SUM8-HEX"
                                          << "SUM8-HEX-CR"
    ;

// ru.wikibooks.org/wiki/Реализации_алгоритмов/Циклический_избыточный_код

/*
  Name  : CRC-8
  Poly  : 0x31    x^8 + x^5 + x^4 + 1
  Init  : 0xFF
  Revert: false
  XorOut: 0x00
  Check : 0xF7 ("123456789")
  MaxLen: 15 байт(127 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/
unsigned char crc8(unsigned char *pcBlock, unsigned int len) {
    unsigned char crc = 0xFF;
    unsigned int i;
    while (len--)     {
        crc ^= *pcBlock++;
        for (i = 0; i < 8; i++) crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }
    return crc;
}

/*
  Name  : CRC-16 CCITT
  Poly  : 0x1021    x^16 + x^12 + x^5 + 1
  Init  : 0xFFFF
  Revert: false
  XorOut: 0x0000
  Check : 0x29B1 ("123456789")
  MaxLen: 4095 байт (32767 бит) - обнаружение
    одинарных, двойных, тройных и всех нечетных ошибок
*/
unsigned short crc16(unsigned char *pcBlock, unsigned short len) {
    unsigned short crc = 0xFFFF;
    unsigned char i;
    while (len--)     {
        crc ^= *pcBlock++ << 8;
        for (i = 0; i < 8; i++) crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}

/*
  Name  : CRC-32
  Poly  : 0x04C11DB7    x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11
                       + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
  Init  : 0xFFFFFFFF
  Revert: true
  XorOut: 0xFFFFFFFF
  Check : 0xCBF43926 ("123456789")
  MaxLen: 268 435 455 байт (2 147 483 647 бит) - обнаружение
   одинарных, двойных, пакетных и всех нечетных ошибок
*/
uint_least32_t crc32(unsigned char *buf, size_t len) {
    uint_least32_t crc_table[256];
    uint_least32_t crc; int i, j;
    for (i = 0; i < 256; i++)     {
        crc = i;
        for (j = 0; j < 8; j++) crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
        crc_table[i] = crc;
    };
    crc = 0xFFFFFFFFUL;
    while (len--) crc = crc_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFUL;
}

QByteArray ba8(const QByteArray &data) {
    return QByteArray(1, (crc8((unsigned char*)data.data(), data.size())));
}

QByteArray ba16(const QByteArray &data, QDataStream::ByteOrder ba) {
    QByteArray crc;
    QDataStream ds(&crc, QIODevice::WriteOnly);
    ds.setByteOrder(ba);
    ds << crc16((unsigned char*)data.data(), data.size());
    return crc;
}

QByteArray ba32(const QByteArray &data, QDataStream::ByteOrder ba) {
    QByteArray crc;
    QDataStream ds(&crc, QIODevice::WriteOnly);
    ds.setByteOrder(ba);
    ds << crc32((unsigned char*)data.data(), data.size());
    return crc;
}

uint sum(const QByteArray &data) {
    uint sum = 0;
    for (int i = 0; i < data.size(); i++) sum += static_cast<uint>(data.at(i));
    return sum;
}

Crc::Crc(QObject *parent): QObject{parent}{}

QStringList Crc::list() {
    return crcList;
}

QByteArray Crc::addCrc(QByteArray &data, uint idx) {
    switch (idx) {
    case IDX_CRC8_BIN: return data.append(ba8(data));
    case IDX_CRC8_HEX: return data.append(QString(ba8(data).toHex()).toUpper().toLatin1());
    case IDX_CRC8_HEX_CR: return data.append(QString(ba8(data).toHex()).toUpper().toLatin1().append(CR));
    case IDX_CRC16_BIN_LITTLE_ENDIAN: return data.append(ba16(data, QDataStream::LittleEndian));
    case IDX_CRC16_BIN_BIG_ENDIAN: return data.append(ba16(data, QDataStream::BigEndian));
    case IDX_CRC16_HEX_LITTLE_ENDIAN: return data.append(QString(ba16(data, QDataStream::LittleEndian).toHex()).toUpper().toLatin1());
    case IDX_CRC16_HEX_LITTLE_ENDIAN_CR: return data.append(QString(ba16(data, QDataStream::LittleEndian).toHex()).toUpper().toLatin1().append(CR));
    case IDX_CRC16_HEX_BIG_ENDIAN: return data.append(QString(ba16(data, QDataStream::BigEndian).toHex()).toUpper().toLatin1());
    case IDX_CRC16_HEX_BIG_ENDIAN_CR: return data.append(QString(ba16(data, QDataStream::BigEndian).toHex()).toUpper().toLatin1().append(CR));
    case IDX_CRC32_BIN_LITTLE_ENDIAN: return data.append(ba32(data, QDataStream::LittleEndian));
    case IDX_CRC32_BIN_BIG_ENDIAN: return data.append(ba32(data, QDataStream::BigEndian));
    case IDX_CRC32_HEX_LITTLE_ENDIAN: return data.append(QString(ba32(data, QDataStream::LittleEndian).toHex()).toUpper().toLatin1());
    case IDX_CRC32_HEX_LITTLE_ENDIAN_CR: return data.append(QString(ba32(data, QDataStream::LittleEndian).toHex()).toUpper().toLatin1().append(CR));
    case IDX_CRC32_HEX_BIG_ENDIAN: return data.append(QString(ba32(data, QDataStream::BigEndian).toHex()).toUpper().toLatin1());
    case IDX_CRC32_HEX_BIG_ENDIAN_CR: return data.append(QString(ba32(data, QDataStream::BigEndian).toHex()).toUpper().toLatin1().append(CR));
    case IDX_SUM_MOD256_BIN: return data.append(1, static_cast<quint8>(sum(data) % 0x0100));
    case IDX_SUM_MOD256_HEX: return data.append(QString(QByteArray(1, static_cast<char>(sum(data) % 0x0100)).toHex()).toUpper().toLatin1());
    case IDX_SUM_MOD256_HEX_CR: return data.append(QString(QByteArray(1, static_cast<char>(sum(data) % 0x0100)).toHex()).toUpper().toLatin1().append(CR));
    default: return data;
    }
}
