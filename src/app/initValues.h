/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef INITVALUES_H
#define INITVALUES_H

#include <array>

#include <QHash>
#include <QString>

class InitValues
{

public:
    InitValues();
    static QString updateFromConfig(InitValues &initValues, QString configFileName);
    void setInfiltrationFactor(int index, float v);
    void setBagrovValue(int index, float value);
    void setDigitsTotalRunoff_R(int v);
    void setDigitsSurfaceRunoff_ROW(int v);
    void setDigitsInfiltration_RI(int v);
    void setDigitsTotalRunoffFlow_RVOL(int v);
    void setDigitsSurfaceRunoffFlow_ROWVOL(int v);
    void setDigitsInfiltrationFlow_RIVOL(int v);
    void setDigitsTotalArea_FLAECHE(int v);
    void setDigitsEvaporation_VERDUNSTUN(int v);
    void setIrrigationToZero(bool v);
    void setPrecipitationCorrectionFactor(float v);
    float getInfiltrationFactor(int index) const;
    float getBagrovValue(int index) const;
    int getDigitsTotalRunoff_R() const;
    int getDigitsSurfaceRunoff_ROW() const;
    int getDigitsInfiltration_RI() const;
    int getDigitsTotalRunoffFlow_RVOL() const;
    int getDigitsSurfaceRunoffFlow_ROWVOL() const;
    int getDigitsInfiltrationFlow_RIVOL() const;
    int getDigitsTotalArea_FLAECHE() const;
    int getDigitsEvaporation_VERDUNSTUN() const;
    bool getIrrigationToZero() const;
    float getPrecipitationCorrectionFactor() const;
    bool allSet() const;
    void putToHashOfType(QString districts, int value, int hashType);
    QHash<int, int> hashETP;
    QHash<int, int> hashETPS;
    QHash<int, int> hashEG;
    int getCountSets() const;

private:
    // Infiltrationsfaktoren
    // index 0: roof, indices 1-4: surface classes 1-4
    // - infdach: Infiltrationsparameter Dachfl.
    // - infbelx: Infiltrationsparameter Belagsfl. x
    std::array<float,5> infiltrationFactors; // old: infdach, infbel1 - infbel4

    // Bagrovwerte
    // index 0: roof, indices 1-4: surface classes 1-4
    std::array<float,5> bagrovValues;

    /*
    writer.setRecordField("R", record.totalRunoff_R);
    writer.setRecordField("ROW", record.surfaceRunoff_ROW);
    writer.setRecordField("RI", record.infiltration_RI);
    writer.setRecordField("RVOL", record.totalRunoffFlow_RVOL);
    writer.setRecordField("ROWVOL", record.surfaceRunoffFlow_ROWVOL);
    writer.setRecordField("RIVOL", record.infiltrationFlow_RIVOL);
    writer.setRecordField("FLAECHE", record.totalArea_FLAECHE);
    writer.setRecordField("VERDUNSTUN", record.evaporation_VERDUNSTUN);
*/
    // Nachkomma
    int digitsTotalRunoff_R; // old: decR
    int digitsSurfaceRunoff_ROW; // old: decROW
    int digitsInfiltration_RI; // old: decRI
    int digitsTotalRunoffFlow_RVOL; // old: decRVOL
    int digitsSurfaceRunoffFlow_ROWVOL; // old: decROWVOL
    int digitsInfiltrationFlow_RIVOL; // old: decRIVOL
    int digitsTotalArea_FLAECHE; // old: decFLAECHE
    int digitsEvaporation_VERDUNSTUN; // old: decVERDUNSTUNG

    // BER to Zero hack
    bool irrigationToZero; // old: BERtoZero

    // Niederschlags-Korrekturfaktor
    float precipitationCorrectionFactor; // old: niedKorrF

    int countSets;

    void putToReferencedHash(QString districts, int value, QHash<int, int> &hash);
};

#endif
