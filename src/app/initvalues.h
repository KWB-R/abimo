/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef INITVALUES_H
#define INITVALUES_H

#include <QHash>
#include <QString>

class InitValues
{

public:
    InitValues();
    ~InitValues();
    static QString updateFromConfig(InitValues &initValues, QString configFileName);
    void setInfiltrationFactorRoof(float v);
    void setInfiltrationFactorSurface1(float v);
    void setInfiltrationFactorSurface2(float v);
    void setInfiltrationFactorSurface3(float v);
    void setInfiltrationFactorSurface4(float v);
    void setBagrovValueRoof(float v);
    void setBagrovValueSuface1(float v);
    void setBagrovValueSuface2(float v);
    void setBagrovValueSuface3(float v);
    void setBagrovSufaceClass4(float v);
    void setDigitsTotalRunoff(int v);
    void setDigitsRunoff(int v);
    void setDigitsInfiltrationRate(int v);
    void setDigitsTotalRunoffFlow(int v);
    void setDigitsRainwaterRunoff(int v);
    void setDigitsTotalSubsurfaceFlow(int v);
    void setDigitsTotalArea(int v);
    void setDigitsEvaporation(int v);
    void setIrrigationToZero(bool v);
    void setPrecipitationCorrectionFactor(float v);
    float getInfiltrationFactorRoof();
    float getInfiltrationFactorSurface1();
    float getInfiltrationFactorSurface2();
    float getInfiltrationFactorSurface3();
    float getInfiltrationFactorSurface4();
    float getBagrovValueRoof();
    float getBagrovValueSuface1();
    float getBagrovValueSuface2();
    float getBagrovValueSuface3();
    float getBagrovValueSuface4();
    int getDigitsTotalRunoff();
    int getDigitsRunoff();
    int getDigitsInfiltrationRate();
    int getDigitsTotalRunoffFlow();
    int getDigitsRainwaterRunoff();
    int getDigitsTotalSubsurfaceFlow();
    int getDigitsTotalArea();
    int getDigitsEvaporation();
    bool getIrrigationToZero();
    float getPrecipitationCorrectionFactor();
    bool allSet();
    void putToHashOfType(QString districts, int value, int hashType);
    QHash<int, int> hashETP;
    QHash<int, int> hashETPS;
    QHash<int, int> hashEG;
    int getCountSets();

private:
    // Infiltrationsfaktoren
    float infiltrationFactorRoof; // old: infdach
    float infiltrationFactorSurface1; // old: infbel1
    float infiltrationFactorSurface2; // old: infbel2
    float infiltrationFactorSurface3; // old: infbel3
    float infiltrationFactorSurface4; // old: infbel4

    // Bagrovwerte
    float bagrovValueRoof; // old: bagdach
    float bagrovValueSuface1; // old: bagbel1
    float bagrovValueSuface2; // old: bagbel2
    float bagrovValueSuface3; // old: bagbel3
    float bagrovValueSuface4; // old: bagbel4

    // Nachkomma
    int digitsTotalRunoff; // old: decR
    int digitsRunoff; // old: decROW
    int digitsInfiltrationRate; // old: decRI
    int digitsTotalRunoffFlow; // old: decRVOL
    int digitsRainwaterRunoff; // old: decROWVOL
    int digitsTotalSubsurfaceFlow; // old: decRIVOL
    int digitsTotalArea; // old: decFLAECHE
    int digitsEvaporation; // old: decVERDUNSTUNG

    // BER to Zero hack
    bool irrigationToZero; // old: BERtoZero

    // Niederschlags-Korrekturfaktor
    float precipitationCorrectionFactor; // old: niedKorrF

    int countSets;

    void putToReferencedHash(QString districts, int value, QHash<int, int> &hash);
};

#endif
