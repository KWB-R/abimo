#ifndef ABIMORECORD_H
#define ABIMORECORD_H

#include <array>
#include <QString>

// Fraction indicates numbers between 0 and 1 (instead of percentages)
class AbimoRecord
{
public:
    AbimoRecord();
    int usage; // old: NUTZUNG
    QString code; // old: CODE
    int precipitationYear; // old: REGENJA
    int precipitationSummer; // old: REGENSO
    float depthToWaterTable; // old: FLUR
    int type; // old: TYP
    int fieldCapacity_30; // old: FELD_30
    int fieldCapacity_150; // old: FELD_150
    int district; // old: BEZIRK

    //
    // degree of sealing (Versiegelungsgrad) of ...
    //

    // ... roof surfaces (Dachflaechen), share of roof area [%] 'PROBAU'
    float mainFractionBuiltSealed; // old: PROBAU_fraction, vgd

    // ... other sealed surfaces (sonst. versiegelte Flaechen)
    float mainFractionUnbuiltSealed; // old: PROVGU_fraction, vgb

    // ... roads (Strassen)
    float roadFractionSealed; // old: VGSTRASSE_fraction, vgs

    //
    // degree of canalization (Kanalisierungsgrad) for ...
    //

    // ... roof surfaces (Dachflaechen)
    float builtSealedFractionConnected; // old: KAN_BEB_fraction, kd

    // ... other sealed surfaces (sonst. versiegelte Flaechen)
    float unbuiltSealedFractionConnected; // old: KAN_VGU_fraction, kb

    // ... sealed roads (Strassen)
    float roadSealedFractionConnected; // old: KAN_STR_fraction, ks

    //
    // share of each pavement class for surfaces except roads of block area
    // Anteil der jeweiligen Belagsklasse
    //

    // old: BELAG1_fraction - BELAG4_fraction, bl1 - bl4
    // take care: index 0 not used (would represent roofs)
    // indices 1-4 represent surface classes 1-4
    std::array<float,5> unbuiltSealedFractionSurface;

    //
    // share of each road pavement class for roads of block area
    // Anteil der jeweiligen Strassenbelagsklasse
    //

    // old: STR_BELAG1_fraction - STR_BELAG4_fraction , bls1 - bls4
    std::array<float,5> roadSealedFractionSurface;

    // total area within city block, except roads
    float mainArea; // old: FLGES, fb;

    // area of roads within city block
    float roadArea; // old: STR_FLGES, fs;

    inline float totalArea() {
        return mainArea + roadArea;
    }

    inline float fractionOfTotalArea(float area) {
        float total = totalArea();
        assert(total > 0);
        assert(area <= total);
        return area / total;
    }

    inline float yearlyHeightToVolumeFlow(float height) {
        return height * 3.171F * totalArea() / 100000.0F;
    }

};

#endif // ABIMORECORD_H
