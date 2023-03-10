/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/
#include <math.h> // for pow(), round()

#include <QDebug>

#include "dbaseFile.h"
#include "dbaseWriter.h"
#include "helpers.h"
#include "initValues.h"

DbaseWriter::DbaseWriter(const QString& filePath, const InitValues& initValues) :
    DbaseFile(filePath)
{
    // Felder mit Namen, Typ, Nachkommastellen
    fields.push_back(DbaseField("CODE", "C", 0));
    fields.push_back(DbaseField("R", "N", initValues.getDigitsTotalRunoff()));
    fields.push_back(DbaseField("ROW", "N", initValues.getDigitsRunoff()));
    fields.push_back(DbaseField("RI", "N", initValues.getDigitsInfiltrationRate()));
    fields.push_back(DbaseField("RVOL", "N", initValues.getDigitsTotalRunoffFlow()));
    fields.push_back(DbaseField("ROWVOL", "N", initValues.getDigitsRainwaterRunoff()));
    fields.push_back(DbaseField("RIVOL", "N", initValues.getDigitsTotalSubsurfaceFlow()));
    fields.push_back(DbaseField("FLAECHE", "N", initValues.getDigitsTotalArea()));
    fields.push_back(DbaseField("VERDUNSTUN", "N", initValues.getDigitsEvaporation()));

    m_headerLength = calculateHeaderLength(fields.size());

    m_date = QDateTime::currentDateTime().date();

    // Fill the hash that assigns field numbers to field names
    for (int i = 0; i < fields.size(); i++) {
        m_fieldPositionMap[fields[i].getName()] = i;
    }
}

bool DbaseWriter::write()
{
    QByteArray data;

    data.resize(m_headerLength);

    // Write the file header containing e.g. names and types of fields
    writeFileHeader(data);

    // Append the actual data
    writeFileData(data);

    if (!m_file.open(QIODevice::WriteOnly)) {
        m_error = QString(
            "kann Out-Datei: '%1' nicht oeffnen\n Grund: %2"
        ).arg(m_file.fileName(), m_file.error());
        return false;
    }

    m_file.write(data);
    m_file.close();

    return true;
}

int DbaseWriter::writeFileHeader(QByteArray& data)
{
    int index = 0;

    // Write start byte
    index = writeBytes(data, index, 0x03, 1);

    // Write date at bytes 1 to 3
    index = writeThreeByteDate(data, index, m_date);

    // Write record number at bytes 4 to 7
    index = writeFourByteInteger(data, index, m_numberOfRecords);

    // Write length of header at bytes 8 to 9
    index = writeTwoByteInteger(data, index, m_headerLength);

    // Calculate the length of one data row in bytes (1 byte separator?)
    m_recordLength = 1;

    for (int i = 0; i < fields.size(); i++) {
        m_recordLength += fields[i].getFieldLength();
    }

    // Write length of data row at bytes 10, 11
    index = writeTwoByteInteger(data, index, m_recordLength);

    // Write zeros at bytes 12 .. 28
    index = writeBytes(data, index, 0x00, 17);

    // wie in der input-Datei ??? byte 29 = language driver code (0x57 = ANSI)
    index = writeBytes(data, index, 0x57, 1); // byte 29
    index = writeBytes(data, index, 0x00, 2); // bytes 30, 31

    for (int i = 0; i < fields.size(); i++) {

        // Write name to data, fill up with zeros
        QString name = fields[i].getName();
        for (int k = 0; k < 11; k++) {
            int value = ((k < name.size())? name[k].toLatin1() : 0x00);
            index = writeBytes(data, index, value, 1);
        }

        // Write type to data, fill up with zeros
        QString type = fields[i].getType();
        index = writeBytes(data, index, type[0].toLatin1(), 1);
        index = writeBytes(data, index, 0x00, 4);

        // Write field length and decimal count to data (one byte each)
        index = writeBytes(data, index, fields[i].getFieldLength(), 1);
        index = writeBytes(data, index, fields[i].getDecimalCount(), 1);

        // Write 14 zeros
        index = writeBytes(data, index, 0x00, 14);
    }

    // Indicate the end of the header by writing a special byte
    index = writeBytes(data, index, 0x0D, 1);

    return index;
}

void DbaseWriter::writeFileData(QByteArray& data)
{
    QVector<QString> strings;

    for (int i = 0; i < m_numberOfRecords; i++) {

        strings = record.at(i);

        data.append(QChar(0x20));

        for (int j = 0; j < fields.size(); j++) {
            data.append(DbaseField::formatNumericString(
                strings.at(j),
                fields[j].getFieldLength(),
                fields[j].getDecimalCount()
            ));
        }
    }

    data.append(QChar(0x1A));
}

int DbaseWriter::writeBytes(QByteArray&data, int index, int value, int n_values)
{
    for (int i = index; i < index + n_values; i++) {
        data[i] = (quint8) value;
    }

    return index + n_values;
}

int DbaseWriter::writeThreeByteDate(QByteArray& data, int index, QDate date)
{
    int year = date.year();
    int year2 = year % 100;

    if (year > 2000) {
        year2 += 100;
    }

    int month = date.month();
    int day = date.day();

    data[index] = (quint8) year2;
    data[index + 1] = (quint8) month;
    data[index + 2] = (quint8) day;

    return index + 3;
}

int DbaseWriter::writeFourByteInteger(QByteArray& data, int index, int value)
{
    data[index] = (quint8) value;
    data[index + 1] = (quint8) (value >> 8);
    data[index + 2] = (quint8) (value >> 16);
    data[index + 3] = (quint8) (value >> 24);

    return index + 4;
}

int DbaseWriter::writeTwoByteInteger(QByteArray& data, int index, int value)
{
    data[index] = (quint8) value;
    data[index + 1] = (quint8)(value >> 8);

    return index + 2;
}

void DbaseWriter::addRecord()
{
    QVector<QString> v(fields.size());
    record.append(v);
    m_numberOfRecords ++;
}

void DbaseWriter::setRecordField(int i, const QString& value)
{
    record.last()[i] = QString(value);

    if (value.size() > fields[i].getFieldLength()) {
        fields[i].setFieldLength(value.size());
    }
}

void DbaseWriter::setRecordField(int i, float value)
{    
    int decimalCount = fields[i].getDecimalCount();

    // round
    value *= pow(10, decimalCount);
    value = round(value);
    value *= pow(10, -decimalCount);

    QString valueStr;
    valueStr.setNum(value, 'f', decimalCount);
    setRecordField(i, valueStr);
}

void DbaseWriter::setRecordField(const QString& name, const QString& value)
{
    if (m_fieldPositionMap.contains(name)) {
        setRecordField(m_fieldPositionMap[name], value);
    }
}

void DbaseWriter::setRecordField(const QString& name, float value)
{
    if (m_fieldPositionMap.contains(name)) {
        setRecordField(m_fieldPositionMap[name], value);
    }
}
