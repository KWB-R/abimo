/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <QBuffer>
#include <QDebug>
#include <QHash>
#include <QIODevice>
#include <QVector>

#include "dbaseField.h"
#include "dbaseFile.h"
#include "dbaseReader.h"

DbaseReader::DbaseReader(const QString& file) :
    DbaseFile(file),
    m_stringValues()
{
}

DbaseReader::~DbaseReader()
{
}

bool DbaseReader::checkAndRead()
{
    QString fileName = m_file.fileName();
    QString text;

    if (!read()) {
        text = "Problem beim Oeffnen der Datei: '%1' aufgetreten.\nGrund: %2";
        m_error.textLong = text.arg(fileName, m_error.textShort);
        return false;
    }

    m_error.textLong = "";
    return true;
}

bool DbaseReader::read()
{
    if (!m_file.open(QIODevice::ReadOnly)) {
        m_error.textShort = "Kann die Datei nicht oeffnen\n" + m_file.errorString();
        return false;
    }

    if (m_file.size() < 32) {
        m_error.textShort = "Datei unbekannten Formats.";
        return false;
    }

    QByteArray info = m_file.read(32);

    m_header.version = byteToVersion(info[0], false);
    m_header.date = bytesToDate(info[1], info[2], info[3]);
    m_header.numberOfRecords = bytesToInteger(info[4], info[5], info[6], info[7]);
    m_header.headerLength = bytesToInteger(info[8], info[9]);
    m_header.recordLength = bytesToInteger(info[10], info[11]);

    // info[12], info[13] reserved - filled with '00h'
    // info[14] - transaction flag
    // info[15] - encryption flag
    // info[16 to 19] - free record thread reserved for LAN
    // info[20 to 27] - reserved for multiuser dbase
    // info[28] MDX-flag

    m_header.languageDriver = byteToLanguageDriver(info[29], false);

    // info[30 - 31] reserved

    int numberOfFields = calculateNumberOfFields(m_header.headerLength);

    int expectedSize = DbaseFile::calculateFileSize(
        m_header.headerLength,
        m_header.numberOfRecords,
        m_header.recordLength
    );

    if (m_file.size() != expectedSize) {
        m_error.textShort = "Datei unbekannten Formats, falsche Groesse.\nSoll: %1\nIst: %2";
        m_error.textShort = m_error.textShort.arg(
            QString::number(expectedSize),
            QString::number(m_file.size())
        );
        return false;
    }

    if (m_header.numberOfRecords <= 0) {
        m_error.textShort = "keine Records in der datei vorhanden.";
        return false;
    }

    if (m_header.recordLength <= 1 || m_header.recordLength > 4000) {
        m_error.textShort = "Groesse der records ist zu klein oder zu gross.";
        return false;
    }

    if (numberOfFields < 1) {
        m_error.textShort = "keine Felder gefunden.";
        return false;
    }

    // rest of header are field information

    //m_fields.resize(m_numberofFields);

    for (int i = 0; i < numberOfFields; i++) {

        DbaseField field(m_file.read(32));
        defineField(field);

        //m_fields[i] = DbaseField(m_file.read(32));
        //m_fieldPositionMap[m_fields[i].getName()] = i;
    }

    // Terminator
    m_file.read(2);

    QByteArray arr = m_file.read(
        m_header.recordLength * m_header.numberOfRecords
    );

    m_file.close();

    QBuffer buffer(&arr);

    buffer.open(QIODevice::ReadOnly);

    const QVector<DbaseField>& fieldDefinitions = getFieldDefinitions();

    assert(fieldDefinitions.size() > 0);
    assert(fieldDefinitions.size() == getNumberOfFields());

    copyBufferToStringVector(
        buffer,
        m_header.numberOfRecords,
        fieldDefinitions,
        m_stringValues
    );

    buffer.close();

    return true;
}

const QVector<QString>& DbaseReader::getStringValues() const
{
    return m_stringValues;
}

void DbaseReader::copyBufferToStringVector(
    QBuffer& buffer,
    int numberOfRecords,
    const QVector<DbaseField>& fieldDefinitions,
    QVector<QString>& stringVector
)
{
    QString s;

    int numberOfFields = fieldDefinitions.length();

    stringVector.resize(numberOfRecords * numberOfFields);

    for (int i = 0; i < numberOfRecords; i++) {
        for (int j = 0; j < numberOfFields; j++) {
            s = buffer.read(fieldDefinitions[j].getFieldLength()).trimmed();
            stringVector[i * numberOfFields + j] = ((s.size() > 0) ? s : "0");
        }
        buffer.read(1);
    }
}

QString DbaseReader::getRecord(int num, const QString& name)
{
    int fieldPosition = getFieldPosition(name);

    if (fieldPosition < 0) {
        return 0;
    }

    return getRecord(num, fieldPosition);
}

QString DbaseReader::getRecord(int num, int field)
{
    int numberOfFields = getNumberOfFields();

    if (num >= m_header.numberOfRecords || field >= numberOfFields) {
        return 0;
    }

    return m_stringValues[num * numberOfFields + field];
}

QString DbaseReader::byteToVersion(quint8 byte, bool debug)
{
    QHash<QString, quint8> versionIDs;

    versionIDs["FoxBase"] = 0x02;
    versionIDs["File without DBT (dBASE III w/o memo file)"] = 0x03;
    versionIDs["dBASE IV w/o memo file"] = 0x04;
    versionIDs["dBASE V w/o memo file"] = 0x05;
    versionIDs["Visual FoxPro w. DBC"] = 0x30;
    versionIDs["Visual FoxPro w. AutoIncrement field"] = 0x31;
    versionIDs[".dbv memo var size (Flagship)"] = 0x43;
    versionIDs["dBASE IV with memo"] = 0x7B;
    versionIDs["dBASE III+ with memo file"] = 0x83;
    versionIDs["dBASE IV w. memo"] = 0x8B;
    versionIDs["dBASE IV w. SQL table"] = 0x8E;
    versionIDs[".dbv and .dbt memo (Flagship)"] = 0xB3;
    versionIDs["Clipper SIX driver w. SMT memo file"] = 0xE5;
    versionIDs["FoxPro w. memo file"] = 0xF5;
    versionIDs["FoxPro ???"] = 0xFB;

    QString result = versionIDs.key(byte, "unknown version");

    if (debug) {
        qDebug() << "dbf file version: " << result;
    }

    return result;
}

// https://stackoverflow.com/questions/52590941/how-to-interpret-the-language-driver-name-in-a-dbase-dbf-file
QString DbaseReader::byteToLanguageDriver(quint8 byte, bool debug)
{
    QHash<QString, quint8> driverIDs;

    driverIDs["DOS USA code page 437"] = 0x01;
    driverIDs["DOS Multilingual code page 850"] = 0x02;
    driverIDs["Windows ANSI code page 1252"] = 0x03;
    driverIDs["Standard Macintosh"] = 0x04;
    driverIDs["ANSI"] = 0x57;
    driverIDs["EE MS-DOS code page 852"] = 0x64;
    driverIDs["Nordic MS-DOS code page 865"] = 0x65;
    driverIDs["Russian MS-DOS code page 866"] = 0x66;
    driverIDs["Icelandic MS-DOS"] = 0x67;
    driverIDs["Kamenicky (Czech) MS-DOS"] = 0x68;
    driverIDs["Mazovia (Polish) MS-DOS"] = 0x69;
    driverIDs["Greek MS-DOS (437G)"] = 0x6A;
    driverIDs["Turkish MS-DOS"] = 0x6B;
    driverIDs["Russian Macintosh"] = 0x96;
    driverIDs["Eastern European Macintosh"] = 0x97;
    driverIDs["Greek Macintosh"] = 0x98;
    driverIDs["Windows EE code page 1250"] = 0xC8;
    driverIDs["Russian Windows"] = 0xC9;
    driverIDs["Turkish Windows"] = 0xCA;
    driverIDs["Greek Windows"] = 0xCB;

    QString result = driverIDs.key(byte, "unknown language driver");

    if (debug) {
        qDebug() << "dbf language driver: " << result << " (id: " << byte << ")";
    }

    return result;
}

QDate DbaseReader::bytesToDate(
        quint8 byteYear, quint8 byteMonth, quint8 byteDay)
{
    int year = (int) byteYear;

    if (year >= 100) {
        year += 1900;
    }

    return QDate(year, (int) byteMonth, (int) byteDay);
}

int DbaseReader::bytesToInteger(quint8 byte1, quint8 byte2)
{
    return (int) (byte1 + (byte2 << 8));
}

int DbaseReader::bytesToInteger(
    quint8 byte1, quint8 byte2, quint8 byte3, quint8 byte4
)
{
    return (int) (byte1 + (byte2 << 8) + (byte3 << 16) + (byte4 << 24));
}
