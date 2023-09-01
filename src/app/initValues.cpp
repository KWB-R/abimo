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
#include "structs.h" // IntegerRange

InitValues::InitValues():

    infiltrationFactors({
        0.00F, // [0] roof
        0.10F, // [1] surface class 1
        0.30F, // [2] surface class 2
        0.60F, // [3] surface class 3
        0.90F  // [4] surface class 4
    }),

    bagrovValues({
        0.05F, // [0] roof
        0.11F, // [1] surface class 1
        0.11F, // [2] surface class 2
        0.25F, // [3] surface class 3
        0.40F  // [4] surface class 4
    }),

    resultDigits({
        {(int) OutputColumn::TotalRunoff_R, 3},
        {(int) OutputColumn::SurfaceRunoff_ROW, 3},
        {(int) OutputColumn::Infiltration_RI, 3},
        {(int) OutputColumn::TotalRunoffFlow_RVOL, 3},
        {(int) OutputColumn::SurfaceRunoffFlow_ROWVOL, 3},
        {(int) OutputColumn::InfiltrationFlow_RIVOL, 3},
        {(int) OutputColumn::TotalArea_FLAECHE, 0},
        {(int) OutputColumn::Evaporation_VERDUNSTUN, 3},
    }),

    irrigationToZero(false),

    precipitationCorrectionFactor(1.09F),

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
    else if (!initValues.allSet()) {
        errorMessage = prefix + "fehlende Werte.\n" + "Ergaenze mit Standardwerten.";
    }

    initFile.close();

    return errorMessage;
}

void InitValues::setInfiltrationFactor(int index, float value)
{
    infiltrationFactors[index] = value;
    countSets |= 1 << (0 + index);
}

void InitValues::setBagrovValue(int index, float value)
{
    bagrovValues[index] = value;
    countSets |= 1 << (5 + index);
}

void InitValues::setResultDigits(OutputColumn column, int n)
{
    resultDigits[(int) column] = n;
    countSets |= 1 << ((int) column); // 10 - 17
}

void InitValues::setIrrigationToZero(bool v)
{
    irrigationToZero = v;
    countSets |= 1 << 18;
}

void InitValues::setPrecipitationCorrectionFactor(float v)
{
    precipitationCorrectionFactor = v;
    countSets |= 1 << 19;
}

float InitValues::getInfiltrationFactor(int index) const
{
    return infiltrationFactors.at(index);
}

float InitValues::getRunoffFactor(int index) const
{
    return 1.0F - infiltrationFactors.at(index);
}

float InitValues::getBagrovValue(int index) const
{
    return bagrovValues.at(index);
}

int InitValues::getResultDigits(OutputColumn column) const
{
    return resultDigits[(int) column];
}

bool InitValues::getIrrigationToZero() const
{
    return irrigationToZero;
}

float InitValues::getPrecipitationCorrectionFactor() const
{
    return precipitationCorrectionFactor;
}

bool InitValues::allSet() const
{
    return countSets == ((1 << 20) - 1); //1048575;
}

void InitValues::putToHashOfType(QString districts, int value, int hashType)
{
    QHash<int, QHash<int, int>*> hashPointers;

    hashPointers[11] = &hashETP;
    hashPointers[12] = &hashETPS;
    hashPointers[13] = &hashEG;

    QVector<int> numbers;

    if (districts.length() == 0) {
        numbers.append(0);
    }
    else {
        numbers = helpers::rangesStringToIntegerSequence(districts);
    }

    for (int i = 0; i < numbers.size(); i++) {
        (*hashPointers[hashType])[numbers.at(i)] = value;
    }
}
