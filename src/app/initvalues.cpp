/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <QFile>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QXmlInputSource>
#include <QXmlSimpleReader>

#include "helpers.h"
#include "initvalues.h"
#include "saxhandler.h"

InitValues::InitValues():

    // Infiltrationsfaktoren
    infiltrationFactorRoof(0.00F),
    infiltrationFactorSurface1(0.10F),
    infiltrationFactorSurface2(0.30F),
    infiltrationFactorSurface3(0.60F),
    infiltrationFactorSurface4(0.90F),

    // Bagrovwerte
    bagrovValueRoof(0.05F),
    bagrovValueSuface1(0.11F),
    bagrovValueSuface2(0.11F),
    bagrovValueSuface3(0.25F),
    bagrovValueSuface4(0.40F),

    // Nachkomma der Ergebnisse
    digitsTotalRunoff(3),
    digitsRunoff(3),
    digitsInfiltrationRate(3),
    digitsTotalRunoffFlow(3),
    digitsRainwaterRunoff(3),
    digitsTotalSubsurfaceFlow(3),
    digitsTotalArea(0),
    digitsEvaporation(3),

    irrigationToZero(false),
    precipitationCorrectionFactor(1.09f),
    countSets(0)
{
}

InitValues::~InitValues()
{
}

// Update initial values with values read from config.xml. Returns error message
QString InitValues::updateFromConfig(InitValues &initValues, QString configFileName)
{
    QString prefix = Helpers::singleQuote(configFileName) + ": ";

    QFile initFile(configFileName);

    if (! initFile.exists()) {
        return "Keine " + prefix + "gefunden.\nNutze Standardwerte.";
    }

    QXmlSimpleReader xmlReader;
    QXmlInputSource data(&initFile);

    SaxHandler handler(initValues);

    xmlReader.setContentHandler(&handler);
    xmlReader.setErrorHandler(&handler);

    // Empty error message (means success)
    QString errorMessage = QString();

    if (!xmlReader.parse(&data)) {
        errorMessage = prefix + "korrupte Datei.\n" + "Nutze Standardwerte.";
    }
    else if (! initValues.allSet()) {
        errorMessage = prefix + "fehlende Werte.\n" + "Ergaenze mit Standardwerten.";
    }

    initFile.close();

    return errorMessage;
}

void InitValues::setInfiltrationFactorRoof(float v) {
    infiltrationFactorRoof = v;
    countSets |= 1;
}

void InitValues::setInfiltrationFactorSurface1(float v) {
    infiltrationFactorSurface1 = v;
    countSets |= 2;
}

void InitValues::setInfiltrationFactorSurface2(float v) {
    infiltrationFactorSurface2 = v;
    countSets |= 4;
}

void InitValues::setInfiltrationFactorSurface3(float v) {
    infiltrationFactorSurface3 = v;
    countSets |= 8;
}

void InitValues::setInfiltrationFactorSurface4(float v) {
    infiltrationFactorSurface4 = v;
    countSets |= 16;
}

void InitValues::setBagrovValueRoof(float v) {
    bagrovValueRoof = v;
    countSets |= 32;
}

void InitValues::setBagrovValueSuface1(float v) {
    bagrovValueSuface1 = v;
    countSets |= 64;
}

void InitValues::setBagrovValueSuface2(float v) {
    bagrovValueSuface2 = v;
    countSets |= 128;
}

void InitValues::setBagrovValueSuface3(float v) {
    bagrovValueSuface3 = v;
    countSets |= 256;
}

void InitValues::setBagrovSufaceClass4(float v) {
    bagrovValueSuface4 = v;
    countSets |= 512;
}

void InitValues::setDigitsTotalRunoff(int v) {
    digitsTotalRunoff = v;
    countSets |= 1024;
}

void InitValues::setDigitsRunoff(int v) {
    digitsRunoff = v;
    countSets |= 2048;
}

void InitValues::setDigitsInfiltrationRate(int v) {
    digitsInfiltrationRate = v;
    countSets |= 4096;
}

void InitValues::setDigitsTotalRunoffFlow(int v) {
    digitsTotalRunoffFlow = v;
    countSets |= 8192;
}

void InitValues::setDigitsRainwaterRunoff(int v) {
    digitsRainwaterRunoff = v;
    countSets |= 16384;
}

void InitValues::setDigitsTotalSubsurfaceFlow(int v) {
    digitsTotalSubsurfaceFlow = v;
    countSets |= 32768;
}

void InitValues::setDigitsTotalArea(int v) {
    digitsTotalArea = v;
    countSets |= 65536;
}

void InitValues::setDigitsEvaporation(int v) {
    digitsEvaporation = v;
    countSets |= 131072;
}

void InitValues::setIrrigationToZero(bool v) {
    irrigationToZero = v;
    countSets |= 262144;
}

void InitValues::setPrecipitationCorrectionFactor(float v) {
    precipitationCorrectionFactor = v;
    countSets |= 524288;
}

float InitValues::getInfiltrationFactorRoof()
{
    return infiltrationFactorRoof;
}

float InitValues::getInfiltrationFactorSurface1() {
    return infiltrationFactorSurface1;
}

float InitValues::getInfiltrationFactorSurface2() {
    return infiltrationFactorSurface2;
}

float InitValues::getInfiltrationFactorSurface3() {
    return infiltrationFactorSurface3;
}

float InitValues::getInfiltrationFactorSurface4() {
    return infiltrationFactorSurface4;
}

float InitValues::getBagrovValueRoof() {
    return bagrovValueRoof;
}

float InitValues::getBagrovValueSuface1() {
    return bagrovValueSuface1;
}

float InitValues::getBagrovValueSuface2() {
    return bagrovValueSuface2;
}

float InitValues::getBagrovValueSuface3() {
    return bagrovValueSuface3;
}

float InitValues::getBagrovValueSuface4() {
    return bagrovValueSuface4;
}

int InitValues::getDigitsTotalRunoff() {
    return digitsTotalRunoff;
}

int InitValues::getDigitsRunoff() {
    return digitsRunoff;
}

int InitValues::getDigitsInfiltrationRate() {
    return digitsInfiltrationRate;
}

int InitValues::getDigitsTotalRunoffFlow() {
    return digitsTotalRunoffFlow;
}

int InitValues::getDigitsRainwaterRunoff() {
    return digitsRainwaterRunoff;
}

int InitValues::getDigitsTotalSubsurfaceFlow() {
    return digitsTotalSubsurfaceFlow;
}

int InitValues::getDigitsTotalArea() {
    return digitsTotalArea;
}

int InitValues::getDigitsEvaporation() {
    return digitsEvaporation;
}

bool InitValues::getIrrigationToZero() {
    return irrigationToZero;
}

float InitValues::getPrecipitationCorrectionFactor() {
    return precipitationCorrectionFactor;
}

bool InitValues::allSet() {
    return countSets == 1048575;
}

int InitValues::getCountSets() {
    return countSets;
}

void InitValues::putToHashOfType(QString districts, int value, int hashType) {

    if (hashType == 11) {
        putToReferencedHash(districts, value, hashETP);
    }
    else if (hashType == 12) {
        putToReferencedHash(districts, value, hashETPS);
    }
    else if (hashType == 13) {
        putToReferencedHash(districts, value, hashEG);
    }
}

void InitValues::putToReferencedHash(QString districts, int value, QHash<int, int> &hash) {

    if (districts.length() == 0) {
        districts = "0";
    }

    QStringList districtList = districts.split(",", QString::SkipEmptyParts);

    for (int i = 0; i < districtList.size(); ++i) {
        QString districtRange = districtList.at(i).trimmed();
        if (districtRange.contains("-")) {
            QStringList rangeLimits = districtRange.split("-", QString::SkipEmptyParts);
            int minId = rangeLimits.at(0).trimmed().toInt();
            int maxId = rangeLimits.at(1).trimmed().toInt();
            for (int id = minId; id <= maxId; ++id) {
                hash[id] = value;
            }
        }
        else {
            hash[districtRange.toInt()] = value;
        }
    }
}
