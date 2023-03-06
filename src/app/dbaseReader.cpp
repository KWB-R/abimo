/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <QBuffer>
#include <QDebug>
#include <QHash>
#include <QIODevice>
#include <QStringList>
#include <QtGlobal>
#include <QVector>

#include "dbaseField.h"
#include "dbaseReader.h"
#include "helpers.h"

DbaseReader::DbaseReader(const QString &file):
    file(file),
    values(0),
    numberOfRecords(0),
    lengthOfHeader(0),
    lengthOfEachRecord(0),
    countFields(0)
{}

DbaseReader::~DbaseReader()
{
    if (values != 0) {
        delete[] values;
    }
}

QString DbaseReader::getError()
{
    return error;
}

QString DbaseReader::getFullError()
{
    return fullError;
}

QString* DbaseReader::getValues()
{
    return values;
}

QStringList DbaseReader::requiredFields()
{
    // The conversion function used in calculation.cpp to convert string to
    // numeric is given as a comment

    return {
        "NUTZUNG",    // toInt()
        "CODE",       // none?
        "REGENJA",    // toInt()
        "REGENSO",    // toInt()
        "FLUR",       // toFloat()
        "TYP",        // toInt()
        "FELD_30",    // toInt()
        "FELD_150",   // toInt()
        "BEZIRK",     // toInt()
        "PROBAU",     // toFloat()
        "PROVGU",     // toFloat()
        "VGSTRASSE",  // toFloat()
        "KAN_BEB",    // toFloat()
        "KAN_VGU",    // toFloat()
        "KAN_STR",    // toFloat()
        "BELAG1",     // toFloat()
        "BELAG2",     // toFloat()
        "BELAG3",     // toFloat()
        "BELAG4",     // toFloat()
        "STR_BELAG1", // toFloat()
        "STR_BELAG2", // toFloat()
        "STR_BELAG3", // toFloat()
        "STR_BELAG4", // toFloat()
        "FLGES",      // toFloat()
        "STR_FLGES"   // toFloat()
      };
}

bool DbaseReader::isAbimoFile()
{
    return Helpers::containsAll(hash, requiredFields());
}

bool DbaseReader::checkAndRead()
{
    QString name = file.fileName();
    QString text;

    if (!read()) {
        text = "Problem beim Oeffnen der Datei: '%1' aufgetreten.\nGrund: %2";
        fullError = text.arg(name, error);
        return false;
    }

    if (!isAbimoFile()) {
        text = "Die Datei '%1' ist kein valider 'Input File',\n";
        text += "Ueberpruefen Sie die Spaltennamen und die Vollstaendigkeit.";
        fullError = text.arg(name);
        return false;
    }

    fullError = "";
    return true;
}

bool DbaseReader::read()
{
    if (!file.open(QIODevice::ReadOnly)) {
        error = "Kann die Datei nicht oeffnen\n" + file.errorString();
        return false;
    }

    if (file.size() < 32) {
        error = "Datei unbekannten Formats.";
        return false;
    }

    QByteArray info = file.read(32);

    version = checkVersion(info[0], false);
    date = checkDate(info[1], info[2], info[3]);
    numberOfRecords = check32(info[4], info[5], info[6], info[7]);
    lengthOfHeader = check16(info[8], info[9]);
    lengthOfEachRecord = check16(info[10], info[11]);

    // info[12], info[13] reserved - filled with '00h'
    // info[14] - transaction flag
    // info[15] - encryption flag
    // info[16 to 19] - free record thread reserved for LAN
    // info[20 to 27] - reserved for multiuser dbase
    // info[28] MDX-flag

    languageDriver = checkLanguageDriver(info[29], false);

    // info[30 - 31] reserved

    countFields = computeCountFields(lengthOfHeader);

    if (file.size() != expectedFileSize()) {
        error = "Datei unbekannten Formats, falsche Groesse.\nSoll: %1\nIst: %2";
        error = error.arg(
            QString::number(expectedFileSize()),
            QString::number(file.size())
        );
        return false;
    }

    if (numberOfRecords <= 0) {
        error = "keine Records in der datei vorhanden.";
        return false;
    }

    if (lengthOfEachRecord <= 1 || lengthOfEachRecord > 4000) {
        error = "Groesse der records ist zu klein oder zu gross.";
        return false;
    }

    if (countFields < 1) {
        error = "keine Felder gefunden.";
        return false;
    }

    //rest of header are field information
    QVector<DbaseField> fields;
    fields.resize(countFields);

    for (int i = 0; i < countFields; i++) {
        fields[i] = DbaseField(file.read(32));
        hash[fields[i].getName()] = i;
    }

    //Terminator
    file.read(2);

    QByteArray arr = file.read(lengthOfEachRecord * numberOfRecords);
    file.close();

    QBuffer buffer(&arr);
    buffer.open(QIODevice::ReadOnly);

    values = new QString[numberOfRecords * countFields];

    for (int i = 0; i < numberOfRecords; i++) {
        for (int j = 0; j < countFields; j++) {
            QString s = buffer.read(fields[j].getFieldLength()).trimmed();
            values[i * countFields + j] = ((s.size() > 0) ? s : "0");
        }
        buffer.read(1);
    }

    buffer.close();
    return true;
}

int DbaseReader::expectedFileSize()
{
    return lengthOfHeader + (numberOfRecords * lengthOfEachRecord) + 1;
}

QString DbaseReader::getRecord(int num, const QString & name)
{
    if (!hash.contains(name)) {
        return 0;
    }

    return getRecord(num, hash[name]);
}

QString DbaseReader::getRecord(int num, int field)
{
    if (num >= numberOfRecords || field >= countFields) {
        return 0;
    }

    return values[num * countFields + field];
}

int DbaseReader::getCountFields()
{
    return countFields;
}

QDate DbaseReader::getDate()
{
    return date;
}

QString DbaseReader::getLanguageDriver()
{
    return languageDriver;
}

QString DbaseReader::getVersion()
{
    return version;
}

int DbaseReader::getNumberOfRecords()
{
    return numberOfRecords;
}

int DbaseReader::getLengthOfHeader()
{
    return lengthOfHeader;
}

int DbaseReader::getLengthOfEachRecord()
{
    return lengthOfEachRecord;
}

int DbaseReader::check16(quint8 i1, quint8 i2)
{
    return (int) (i1 + (i2 << 8));
}

int DbaseReader::check32(quint8 i1, quint8 i2, quint8 i3, quint8 i4)
{
    return (int) (i1 + (i2 << 8) + (i3 << 16) + (i4 << 24));
}

QDate DbaseReader::checkDate(quint8 i_year, quint8 i_month, quint8 i_day)
{
    int year = (int) i_year;

    if (year >= 100) {
        year = 1900 + year;
    }

    return QDate(year, (int) i_month, (int) i_day);
}

QString DbaseReader::checkVersion(quint8 i_byte, bool debug)
{
    QHash<QString, quint8> hash;

    hash["FoxBase"] = 0x02;
    hash["File without DBT (dBASE III w/o memo file)"] = 0x03;
    hash["dBASE IV w/o memo file"] = 0x04;
    hash["dBASE V w/o memo file"] = 0x05;
    hash["Visual FoxPro w. DBC"] = 0x30;
    hash["Visual FoxPro w. AutoIncrement field"] = 0x31;
    hash[".dbv memo var size (Flagship)"] = 0x43;
    hash["dBASE IV with memo"] = 0x7B;
    hash["dBASE III+ with memo file"] = 0x83;
    hash["dBASE IV w. memo"] = 0x8B;
    hash["dBASE IV w. SQL table"] = 0x8E;
    hash[".dbv and .dbt memo (Flagship)"] = 0xB3;
    hash["Clipper SIX driver w. SMT memo file"] = 0xE5;
    hash["FoxPro w. memo file"] = 0xF5;
    hash["FoxPro ???"] = 0xFB;

    QString result = hash.key(i_byte, "unknown version");

    if (debug) {
        qDebug() << "dbf file version: " << result;
    }

    return result;
}

// https://stackoverflow.com/questions/52590941/how-to-interpret-the-language-driver-name-in-a-dbase-dbf-file
QString DbaseReader::checkLanguageDriver(quint8 i_byte, bool debug)
{
    QHash<QString, quint8> hash;

    hash["DOS USA code page 437"] = 0x01;
    hash["DOS Multilingual code page 850"] = 0x02;
    hash["Windows ANSI code page 1252"] = 0x03;
    hash["Standard Macintosh"] = 0x04;
    hash["ANSI"] = 0x57;
    hash["EE MS-DOS code page 852"] = 0x64;
    hash["Nordic MS-DOS code page 865"] = 0x65;
    hash["Russian MS-DOS code page 866"] = 0x66;
    hash["Icelandic MS-DOS"] = 0x67;
    hash["Kamenicky (Czech) MS-DOS"] = 0x68;
    hash["Mazovia (Polish) MS-DOS"] = 0x69;
    hash["Greek MS-DOS (437G)"] = 0x6A;
    hash["Turkish MS-DOS"] = 0x6B;
    hash["Russian Macintosh"] = 0x96;
    hash["Eastern European Macintosh"] = 0x97;
    hash["Greek Macintosh"] = 0x98;
    hash["Windows EE code page 1250"] = 0xC8;
    hash["Russian Windows"] = 0xC9;
    hash["Turkish Windows"] = 0xCA;
    hash["Greek Windows"] = 0xCB;

    QString result = hash.key(i_byte, "unknown language driver");

    if (debug) {
        qDebug() << "dbf language driver: " << result << " (id: " << i_byte << ")";
    }

    return result;
}

int DbaseReader::computeCountFields(int headerLength)
{
    //32 byte file information, 1 byte Terminator (0Dh), und jedes Feld hat 32 byte im header
    return (headerLength - 32 - 1)/32;
}

void DbaseReader::fillRecord(int k, abimoRecord& record, bool debug)
{
    record.unbuiltSealedFractionSurface1 = floatFraction(getRecord(k, "BELAG1"));
    record.unbuiltSealedFractionSurface2 = floatFraction(getRecord(k, "BELAG2"));
    record.unbuiltSealedFractionSurface3 = floatFraction(getRecord(k, "BELAG3"));
    record.unbuiltSealedFractionSurface4 = floatFraction(getRecord(k, "BELAG4"));
    record.district = getRecord(k, "BEZIRK").toInt();
    record.code = getRecord(k, "CODE");
    record.fieldCapacity_150 = getRecord(k, "FELD_150").toInt();
    record.fieldCapacity_30 = getRecord(k, "FELD_30").toInt();
    record.mainArea = getRecord(k, "FLGES").toFloat();
    record.depthToWaterTable = getRecord(k, "FLUR").toFloat();
    record.builtSealedFractionConnected = floatFraction(getRecord(k, "KAN_BEB"));
    record.roadSealedFractionConnected = floatFraction(getRecord(k, "KAN_STR"));
    record.unbuiltSealedFractionConnected = floatFraction(getRecord(k, "KAN_VGU"));
    record.usage = Helpers::stringToInt(
        getRecord(k, "NUTZUNG"),
        QString("k: %1, NUTZUNG = ").arg(QString::number(k)),
        debug
    );
    record.mainFractionBuiltSealed = Helpers::stringToFloat(
        getRecord(k, "PROBAU"),
        QString("k: %1, PROBAU = ").arg(QString::number(k)),
        debug
    ) / 100.0F;
    record.mainFractionUnbuiltSealed = floatFraction(getRecord(k, "PROVGU"));
    record.precipitationYear = getRecord(k, "REGENJA").toInt();
    record.precipitationSummer = getRecord(k, "REGENSO").toInt();
    record.roadSealedFractionSurface1 = floatFraction(getRecord(k, "STR_BELAG1"));
    record.roadSealedFractionSurface2 = floatFraction(getRecord(k, "STR_BELAG2"));
    record.roadSealedFractionSurface3 = floatFraction(getRecord(k, "STR_BELAG3"));
    record.roadSealedFractionSurface4 = floatFraction(getRecord(k, "STR_BELAG4"));
    record.type = getRecord(k, "TYP").toInt();
    record.roadFractionSealed = floatFraction(getRecord(k, "VGSTRASSE"));
    record.roadArea = getRecord(k, "STR_FLGES").toFloat();
}

float DbaseReader::floatFraction(QString string)
{
    return (string.toFloat() / 100.0);
}
