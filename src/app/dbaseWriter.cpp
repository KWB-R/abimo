/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <math.h>
#include <QByteArray>
#include <QChar>
#include <QDateTime>
#include <QFile>
#include <QHash>
#include <QIODevice>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVector>

#include "dbaseWriter.h"
#include "initvalues.h"

DbaseWriter::DbaseWriter(QString &file, InitValues &initValues):
    fileName(file),
    numberOfRecords(0)
{
    // Felder mit Namen, Typ, Nachkommastellen
    fields[0].set("CODE", "C", 0);
    fields[1].set("R", "N", initValues.getDigitsTotalRunoff());
    fields[2].set("ROW", "N", initValues.getDigitsRunoff());
    fields[3].set("RI", "N", initValues.getDigitsInfiltrationRate());
    fields[4].set("RVOL", "N", initValues.getDigitsTotalRunoffFlow());
    fields[5].set("ROWVOL", "N", initValues.getDigitsRainwaterRunoff());
    fields[6].set("RIVOL", "N", initValues.getDigitsTotalSubsurfaceFlow());
    fields[7].set("FLAECHE", "N", initValues.getDigitsTotalArea());
    fields[8].set("VERDUNSTUN", "N", initValues.getDigitsEvaporation());

    this->date = QDateTime::currentDateTime().date();

    // Fill the hash that assigns field numbers to field names
    for (int i = 0; i < countFields; i++) {
        hash[fields[i].getName()] = i;
    }
}

QString DbaseWriter::getError()
{
    return error;
}

bool DbaseWriter::write()
{
    QByteArray data;

    data.resize(lengthOfHeader);

    // Write the file header containing e.g. names and types of fields
    writeFileHeader(data);

    // Append the actual data
    writeFileData(data);

    QFile outputFile(fileName);

    if (!outputFile.open(QIODevice::WriteOnly)) {
        error = "kann Out-Datei: '" + fileName + "' nicht oeffnen\n Grund: " + outputFile.error();
        return false;
    }

    outputFile.write(data);
    outputFile.close();

    return true;
}

int DbaseWriter::writeFileHeader(QByteArray &data)
{
    int index = 0;

    // Write start byte
    index = writeBytes(data, index, 0x03, 1);

    // Write date at bytes 1 to 3
    index = writeThreeByteDate(data, index, date);

    // Write record number at bytes 4 to 7
    index = writeFourByteInteger(data, index, numberOfRecords);

    // Write length of header at bytes 8 to 9
    index = writeTwoByteInteger(data, index, lengthOfHeader);

    // Calculate the length of one data row in bytes (1 byte separator?)
    lengthOfEachRecord = 1;

    for (int i = 0; i < countFields; i++) {
        lengthOfEachRecord += fields[i].getFieldLength();
    }

    // Write length of data row at bytes 10, 11
    index = writeTwoByteInteger(data, index, lengthOfEachRecord);

    // Write zeros at bytes 12 .. 28
    index = writeBytes(data, index, 0x00, 17);

    // wie in der input-Datei ??? byte 29 = language driver code (0x57 = ANSI)
    index = writeBytes(data, index, 0x57, 1); // byte 29
    index = writeBytes(data, index, 0x00, 2); // bytes 30, 31

    for (int i = 0; i < countFields; i++) {

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

void DbaseWriter::writeFileData(QByteArray &data)
{
    QVector<QString> strings;

    for (int i = 0; i < numberOfRecords; i++) {

        strings = record.at(i);
        data.append(QChar(0x20));

        for (int j = 0; j < countFields; j++) {

            int fieldLength = fields[j].getFieldLength();

            if (fields[j].getDecimalCount() > 0) {
                QString str = strings.at(j);
                QStringList strlist = str.split(".");
                int frontLength = fieldLength - 1 - fields[j].getDecimalCount();
                if (strlist.at(0).contains('-')) {
                    data.append(QString("-"));
                    data.append(strlist.at(0).right(strlist.at(0).length() - 1).rightJustified(frontLength-1, QChar(0x30)));
                }
                else {
                    data.append(strlist.at(0).rightJustified(frontLength, QChar(0x30)));
                }
                data.append(".");
                data.append(strlist.at(1).leftJustified(
                    fields[j].getDecimalCount(),
                    QChar(0x30)
                ));
            }
            else {
                data.append(strings.at(j).rightJustified(
                  fieldLength, QChar(0x30)
                ));
            }
        }
    }

    data.append(QChar(0x1A));
}

int DbaseWriter::writeBytes(QByteArray &data, int index, int value, int n_values)
{
    for (int i = index; i < index + n_values; i++) {
        data[i] = (quint8) value;
    }

    return index + n_values;
}

int DbaseWriter::writeThreeByteDate(QByteArray &data, int index, QDate date)
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

int DbaseWriter::writeFourByteInteger(QByteArray &data, int index, int value)
{
    data[index] = (quint8) value;
    data[index + 1] = (quint8) (value >> 8);
    data[index + 2] = (quint8) (value >> 16);
    data[index + 3] = (quint8) (value >> 24);

    return index + 4;
}

int DbaseWriter::writeTwoByteInteger(QByteArray &data, int index, int value)
{
    data[index] = (quint8) value;
    data[index + 1] = (quint8)(value >> 8);

    return index + 2;
}

void DbaseWriter::addRecord()
{
    QVector<QString> v(countFields);
    record.append(v);
    numberOfRecords ++;
}

void DbaseWriter::setRecordField(int i, QString value)
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

void DbaseWriter::setRecordField(QString name, QString value)
{
    if (hash.contains(name)) {
        setRecordField(hash[name], value);
    }
}

void DbaseWriter::setRecordField(QString name, float value)
{
    if (hash.contains(name)) {
        setRecordField(hash[name], value);
    }
}
