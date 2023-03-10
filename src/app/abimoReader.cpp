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

QStringList AbimoReader::requiredFields()
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

bool AbimoReader::checkAndRead()
{
    QString fileName = m_file.fileName();
    QString text;

    bool succeeded = DbaseReader::checkAndRead();

    if (!succeeded) {
        return false;
    }

    if (!isAbimoFile()) {
        text = "Die Datei '%1' ist kein valider 'Input File',\n";
        text += "Ueberpruefen Sie die Spaltennamen und die Vollstaendigkeit.";
        m_fullError = text.arg(fileName);
        return false;
    }

    m_fullError = "";
    return true;
}

bool AbimoReader::isAbimoFile()
{
    return helpers::containsAll(m_fieldPositionMap, requiredFields());
}

void AbimoReader::fillRecord(int rowIndex, AbimoRecord& record, bool debug)
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
