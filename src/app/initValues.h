/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef INITVALUES_H
#define INITVALUES_H

#include <array>

#include <QHash>
#include <QString>

#include "structs.h"

class InitValues
{

public:
    InitValues();

    static QString updateFromConfig(InitValues &initValues, QString configFileName);

    void setInfiltrationFactor(int index, float v);
    void setBagrovValue(int index, float value);
    void setResultDigits(OutputColumn column, int n);
    void setIrrigationToZero(bool v);
    void setPrecipitationCorrectionFactor(float v);

    float getInfiltrationFactor(int index) const;
    float getRunoffFactor(int index) const;
    float getBagrovValue(int index) const;
    int getResultDigits(OutputColumn column) const;
    bool getIrrigationToZero() const;
    float getPrecipitationCorrectionFactor() const;

    bool allSet() const;
    void putToHashOfType(QString districts, int value, int hashType);

    QHash<int, int> hashETP;
    QHash<int, int> hashETPS;
    QHash<int, int> hashEG;

private:
    // Infiltrationsfaktoren
    // index 0: roof, indices 1-4: surface classes 1-4
    // - infdach: Infiltrationsparameter Dachfl.
    // - infbelx: Infiltrationsparameter Belagsfl. x
    std::array<float,5> infiltrationFactors; // old: infdach, infbel1 - infbel4

    // Bagrovwerte
    // index 0: roof, indices 1-4: surface classes 1-4
    std::array<float,5> bagrovValues;

    // Nachkomma
    QHash<int, int> resultDigits;

    // BER to Zero hack
    bool irrigationToZero; // old: BERtoZero

    // Niederschlags-Korrekturfaktor
    float precipitationCorrectionFactor; // old: niedKorrF

    int countSets;
};

#endif
