/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/
#include <math.h> // for pow(), round()

#include <QDebug>
#include <QVector>

#include "dbaseFile.h"
#include "dbaseWriter.h"
#include "helpers.h"

DbaseWriter::DbaseWriter(QString& filePath) :
    DbaseFile(filePath),
    m_record()
{
}

bool DbaseWriter::write()
{
    QByteArray data;

    m_header.headerLength = calculateHeaderLength(getNumberOfFields());

    // m_fields must be set to calculate the record length!
    assert(m_fields.size() > 0);

    m_header.recordLength = calculateRecordLength();

    // Write the file header containing e.g. names and types of fields
    writeFileHeader(data);

    // Append the actual data
    writeFileData(data);

    if (!m_file.open(QIODevice::WriteOnly)) {
        m_error.textShort = QString(
            "kann Out-Datei: '%1' nicht oeffnen\n Grund: %2"
        ).arg(
            m_file.fileName(),
            m_file.error()
        );
        return false;
    }

    m_file.write(data);
    m_file.close();

    return true;
}

int DbaseWriter::writeFileHeader(QByteArray& data)
{
    data.resize(m_header.headerLength);

    // Index into the byte array
    int index = 0;

    // Write first part of header (without field specifications)
    index = writeFileHeaderBase(data, index, m_header);

    // Write second part of header (field specifications)
    //const QVector<DbaseField>& fields = getFieldDefinitions();
    assert(getNumberOfFields() > 0);

    for (int i = 0; i < m_fields.size(); i++) {
        index = writeFileHeaderField(data, index, m_fields.at(i));
    }

    // Indicate the end of the header by writing a special byte
    index = writeBytes(data, index, 0x0D, 1);

    return index;
}

int DbaseWriter::writeFileHeaderBase(
    QByteArray& data, int index, DbaseFileHeader& header
)
{
    // Write start byte at byte 0
    index = writeBytes(data, index, 0x03, 1);

    // Write date at bytes 1 to 3
    index = writeThreeByteDate(data, index, header.date);

    // Write record number at bytes 4 to 7
    index = writeFourByteInteger(data, index, header.numberOfRecords);

    // Write length of header at bytes 8 to 9
    index = writeTwoByteInteger(data, index, header.headerLength);

    // Write length of data row at bytes 10, 11
    index = writeTwoByteInteger(data, index, header.recordLength);

    // Write zeros at bytes 12 .. 28
    index = writeBytes(data, index, 0x00, 17);

    // wie in der input-Datei ??? byte 29 = language driver code (0x57 = ANSI)
    index = writeBytes(data, index, 0x57, 1); // byte 29
    index = writeBytes(data, index, 0x00, 2); // bytes 30, 31

    return index;
}

int DbaseWriter::writeFileHeaderField(
    QByteArray& data, int index, DbaseField field
)
{
    int value;

    // Write name to data, fill up with zeros
    QString name = field.getName();

    // Write field name to bytes 0 .. 9
    for (int i = 0; i < 11; i++) {
        value = ((i < name.size())? name[i].toLatin1() : 0x00);
        index = writeBytes(data, index, value, 1);
    }

    // Write field type to byte 10
    QString type = field.getType();
    index = writeBytes(data, index, type[0].toLatin1(), 1);

    // Write 0 to bytes 11 .. 14
    index = writeBytes(data, index, 0x00, 4);

    // Write field length to byte 15
    index = writeBytes(data, index, field.getFieldLength(), 1);

    // Write number of decimal places to byte 16
    index = writeBytes(data, index, field.getDecimalCount(), 1);

    // Write 14-times 0 to bytes 17 .. 31
    index = writeBytes(data, index, 0x00, 14);

    return index;
}

void DbaseWriter::writeFileData(QByteArray& data)
{
    const QVector<DbaseField>& fields = getFieldDefinitions();

    QVector<QString> strings;
    QString string;
    QChar fill('0');

    for (int i = 0; i < m_header.numberOfRecords; i++) {

        strings = m_record.at(i);

        data.append(QChar(0x20));

        for (int j = 0; j < fields.size(); j++) {

            // Desired length of the string
            int length = fields[j].getFieldLength();

            string = strings.at(j);

            // Format the string so that it has the desired length
            string = (fields[j].getDecimalCount() > 0) ?
                helpers::rightJustifiedNumber(string, length, fill) :
                string.rightJustified(length, fill, true);

            assert(string.length() == length);

            data.append(string);
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
    QVector<QString> v(getFieldDefinitions().size());
    m_record.append(v);
    m_header.numberOfRecords ++;
}

void DbaseWriter::setRecordField(int i, QString& value)
{
    m_record.last()[i] = QString(value);

    // Update the maximum field length
    int length = value.size();

    if (length > m_fields[i].getFieldLength()) {
        m_fields[i].setFieldLength(length);
    }
}

void DbaseWriter::setRecordField(int i, float value)
{
    int digits = m_fields[i].getDecimalCount();
    QString string;

    string.setNum(helpers::roundFloat(value, digits), 'f', digits);

    setRecordField(i, string);
}

void DbaseWriter::setRecordField(const char* name, QString value)
{
    int fieldPosition = getFieldPosition(QString(name));

    if (fieldPosition >= 0) {
        setRecordField(fieldPosition, value);
    }
}

void DbaseWriter::setRecordField(const char* name, int value)
{
    int fieldPosition = getFieldPosition(name);

    if (fieldPosition >= 0) {
        setRecordField(fieldPosition, value);
    }
}

void DbaseWriter::setRecordField(const char* name, float value)
{
    int fieldPosition = getFieldPosition(name);

    if (fieldPosition >= 0) {
        setRecordField(fieldPosition, value);
    }
}
