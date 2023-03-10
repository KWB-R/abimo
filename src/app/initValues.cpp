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
#include "initValues.h"
#include "saxHandler.h"

InitValues::InitValues():

    // Infiltrationsfaktoren (0 = roof, 1-4 = surface classes 1-4)
    infiltrationFactors({0.00F, 0.10F, 0.30F, 0.60F, 0.90F}),

    // Bagrovwerte (0 = roof, 1-4 = surface classes 1-4)
    bagrovValues({0.05F, 0.11F, 0.11F, 0.25F, 0.40F}),

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

// Update initial values with values read from config.xml. Returns error message
QString InitValues::updateFromConfig(InitValues &initValues, QString configFileName)
{
    QString prefix = helpers::singleQuote(configFileName) + ": ";

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

void InitValues::setInfiltrationFactor(int index, float value) {
    infiltrationFactors[index] = value;
    countSets |= 1 << (0 + index);
}

void InitValues::setBagrovValue(int index, float value) {
    bagrovValues[index] = value;
    countSets |= 1 << (5 + index);
}

void InitValues::setDigitsTotalRunoff(int v) {
    digitsTotalRunoff = v;
    countSets |= 1 << 10;
}

void InitValues::setDigitsRunoff(int v) {
    digitsRunoff = v;
    countSets |= 1 << 11;
}

void InitValues::setDigitsInfiltrationRate(int v) {
    digitsInfiltrationRate = v;
    countSets |= 1 << 12;
}

void InitValues::setDigitsTotalRunoffFlow(int v) {
    digitsTotalRunoffFlow = v;
    countSets |= 1 << 13;
}

void InitValues::setDigitsRainwaterRunoff(int v) {
    digitsRainwaterRunoff = v;
    countSets |= 1 << 14;
}

void InitValues::setDigitsTotalSubsurfaceFlow(int v) {
    digitsTotalSubsurfaceFlow = v;
    countSets |= 1 << 15;
}

void InitValues::setDigitsTotalArea(int v) {
    digitsTotalArea = v;
    countSets |= 1 << 16;
}

void InitValues::setDigitsEvaporation(int v) {
    digitsEvaporation = v;
    countSets |= 1 << 17;
}

void InitValues::setIrrigationToZero(bool v) {
    irrigationToZero = v;
    countSets |= 1 << 18;
}

void InitValues::setPrecipitationCorrectionFactor(float v) {
    precipitationCorrectionFactor = v;
    countSets |= 1 << 19;
}

float InitValues::getInfiltrationFactor(int index) const {
    return infiltrationFactors.at(index);
}

float InitValues::getBagrovValue(int index) const {
    return bagrovValues.at(index);
}

int InitValues::getDigitsTotalRunoff() const {
    return digitsTotalRunoff;
}

int InitValues::getDigitsRunoff() const {
    return digitsRunoff;
}

int InitValues::getDigitsInfiltrationRate() const {
    return digitsInfiltrationRate;
}

int InitValues::getDigitsTotalRunoffFlow() const {
    return digitsTotalRunoffFlow;
}

int InitValues::getDigitsRainwaterRunoff() const {
    return digitsRainwaterRunoff;
}

int InitValues::getDigitsTotalSubsurfaceFlow() const {
    return digitsTotalSubsurfaceFlow;
}

int InitValues::getDigitsTotalArea() const {
    return digitsTotalArea;
}

int InitValues::getDigitsEvaporation() const {
    return digitsEvaporation;
}

bool InitValues::getIrrigationToZero() const {
    return irrigationToZero;
}

float InitValues::getPrecipitationCorrectionFactor() const {
    return precipitationCorrectionFactor;
}

bool InitValues::allSet() const {
    return countSets == 1048575;
}

int InitValues::getCountSets() const {
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
