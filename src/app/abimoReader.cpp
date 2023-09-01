#include <QString>

#include "abimoReader.h"
#include "dbaseReader.h"
#include "helpers.h"

AbimoReader::AbimoReader(const QString& file) : DbaseReader(file),
    m_rowIndex(0),
    m_debug(false)
{
    // Child constructor code here
}

AbimoReader::~AbimoReader()
{

};

QStringList AbimoReader::requiredFields()
{
    return {
        "NUTZUNG",    // int
        "CODE",       // QString
        "REGENJA",    // int
        "REGENSO",    // int
        "FLUR",       // float
        "TYP",        // int
        "FELD_30",    // int
        "FELD_150",   // int
        "BEZIRK",     // int
        "PROBAU",     // float
        "PROVGU",     // float
        "VGSTRASSE",  // float
        "KAN_BEB",    // float
        "KAN_VGU",    // float
        "KAN_STR",    // float
        "BELAG1",     // float
        "BELAG2",     // float
        "BELAG3",     // float
        "BELAG4",     // float
        "STR_BELAG1", // float
        "STR_BELAG2", // float
        "STR_BELAG3", // float
        "STR_BELAG4", // float
        "FLGES",      // float
        "STR_FLGES"   // float
      };
}

bool AbimoReader::checkAndRead()
{
    QString fileName = m_file.fileName();
    QString text;

    // Call the checkAndRead() function of the parent class first
    bool succeeded = DbaseReader::checkAndRead();

    if (!succeeded) {
        return false;
    }

    if (!isAbimoFile()) {
        text = "Die Datei '%1' ist kein valider 'Input File',\n";
        text += "Ueberpruefen Sie die Spaltennamen und die Vollstaendigkeit.";
        m_error.textLong = text.arg(fileName);
        return false;
    }

    m_error.textLong = "";
    return true;
}

bool AbimoReader::isAbimoFile()
{
    QStringList fieldNames = AbimoReader::requiredFields();

    return hasAllOfTheseFields(fieldNames);
}

void AbimoReader::fillRecord(int rowIndex, AbimoInputRecord& record, bool debug)
{
    m_rowIndex = rowIndex;
    m_debug = debug;

    record.usage = getAsInteger("NUTZUNG");
    record.code = getAsString("CODE");

    record.precipitationYear = getAsInteger("REGENJA");
    record.precipitationSummer = getAsInteger("REGENSO");

    record.depthToWaterTable = getAsFloat("FLUR");
    record.type = getAsInteger("TYP");

    record.fieldCapacity_30 = getAsInteger("FELD_30");
    record.fieldCapacity_150 = getAsInteger("FELD_150");

    record.district = getAsInteger("BEZIRK");

    record.mainFractionBuiltSealed = getAsFloatFraction("PROBAU");
    record.mainFractionUnbuiltSealed = getAsFloatFraction("PROVGU");
    record.roadFractionSealed = getAsFloatFraction("VGSTRASSE");

    record.builtSealedFractionConnected = getAsFloatFraction("KAN_BEB");
    record.unbuiltSealedFractionConnected = getAsFloatFraction("KAN_VGU");
    record.roadSealedFractionConnected = getAsFloatFraction("KAN_STR");

    record.unbuiltSealedFractionSurface[1] = getAsFloatFraction("BELAG1");
    record.unbuiltSealedFractionSurface[2] = getAsFloatFraction("BELAG2");
    record.unbuiltSealedFractionSurface[3] = getAsFloatFraction("BELAG3");
    record.unbuiltSealedFractionSurface[4] = getAsFloatFraction("BELAG4");

    record.roadSealedFractionSurface[1] = getAsFloatFraction("STR_BELAG1");
    record.roadSealedFractionSurface[2] = getAsFloatFraction("STR_BELAG2");
    record.roadSealedFractionSurface[3] = getAsFloatFraction("STR_BELAG3");
    record.roadSealedFractionSurface[4] = getAsFloatFraction("STR_BELAG4");

    record.mainArea = getAsFloat("FLGES");
    record.roadArea = getAsFloat("STR_FLGES");
}

QString AbimoReader::getAsString(const char* fieldName)
{
    return getRecord(m_rowIndex, QString(fieldName));
}

int AbimoReader::getAsInteger(const char* fieldName)
{
    QString name = QString(fieldName);
    QString value = getRecord(m_rowIndex, name);

    return (m_debug) ?
        helpers::stringToInt(value, rowFieldIndicator(name), m_debug) :
        value.toInt();
}

float AbimoReader::getAsFloat(const char* fieldName)
{
    QString name = QString(fieldName);
    QString value = getRecord(m_rowIndex, name);

    return (m_debug) ?
        helpers::stringToFloat(value, rowFieldIndicator(name), m_debug) :
        value.toFloat();
}

float AbimoReader::getAsFloatFraction(const char* fieldName)
{
    return getAsFloat(fieldName) / 100.0;
}

QString AbimoReader::rowFieldIndicator(QString& fieldName)
{
    return QString("rowIndex: %1, %s = ").arg(
        QString::number(m_rowIndex),
        fieldName
    );
}
