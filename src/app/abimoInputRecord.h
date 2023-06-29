#ifndef ABIMOINPUTRECORD_H
#define ABIMOINPUTRECORD_H

#include <array>
#include <QString>

#include "helpers.h"

// Fraction indicates numbers between 0 and 1 (instead of percentages)
class AbimoInputRecord
{
public:
    AbimoInputRecord();

    // old: NUTZUNG
    int usage;

    // old: CODE
    QString code;

    // precipitation for entire year
    // Niederschlag <MD-Wert> [mm/a] ID_PMD 003 N
    // old: REGENJA, P1
    int precipitationYear;

    // precipitation for summer season only
    // Sommer-Niederschlag ID_PS neu
    // old: REGENSO, P1S
    int precipitationSummer;

    // depth to groundwater table
    // Flurabstandswert [m] ID_FLW 4.1 N
    // old: FLUR, FLW
    float depthToWaterTable;

    // old: TYP
    int type;

    // old: FELD_30
    int fieldCapacity_30;

    // old: FELD_150
    int fieldCapacity_150;

    // old: BEZIRK
    int district;

    //
    // degree of sealing (Versiegelungsgrad) of ...
    //

    // ... roof surfaces (Dachflaechen), share of roof area [%] 'PROBAU'
    // ... Anteil versiegelte Dachflaeche zu Gesamtblockteilflaeche
    // ... old: PROBAU_fraction, vgd
    float mainFractionBuiltSealed;

    // ... other sealed surfaces (sonst. versiegelte Flaechen)
    // ... Anteil sonstige versiegelte Flaeche zu Gesamtblockteilflaeche
    // ... old: PROVGU_fraction, vgb
    float mainFractionUnbuiltSealed;

    // ... roads (Strassen)
    // old: VGSTRASSE_fraction, vgs
    float roadFractionSealed;

    //
    // degree of canalization (Kanalisierungsgrad) for ...
    //

    // ... roof surfaces (Dachflaechen)
    // ... Grad der Kanalisierung Dachflaechen
    // ... old: KAN_BEB_fraction, kd
    float builtSealedFractionConnected;

    // ... other sealed surfaces (sonst. versiegelte Flaechen)
    // ... Grad der Kanalisierung sonstiger versiegelter Flaechen
    // ... old: KAN_VGU_fraction, kb
    float unbuiltSealedFractionConnected;

    // ... sealed roads (Strassen)
    // ... Grad der Kanalisierung Strassenflaechen
    // ... old: KAN_STR_fraction, ks
    float roadSealedFractionConnected;

    //
    // share of each pavement class for surfaces except roads of block area
    // Anteil der jeweiligen Belagsklasse
    // old: BELAG1_fraction - BELAG4_fraction, bl1 - bl4, bel1 - bel4
    //
    // take care: index 0 not used (would represent roofs)
    // indices 1-4 represent surface classes 1-4
    std::array<float,5> unbuiltSealedFractionSurface;

    //
    // share of each road pavement class for roads of block area
    // Anteil der jeweiligen Strassenbelagsklasse
    // old: STR_BELAG1_fraction - STR_BELAG4_fraction , bls1 - bls4
    //
    std::array<float,5> roadSealedFractionSurface;

    // total area within city block, except roads
    // old: FLGES, fb;
    float mainArea;

    // area of roads within city block
    // old: STR_FLGES, fs;
    float roadArea;

    inline float totalArea_FLAECHE() {
        return mainArea + roadArea;
    }

    // Fraction of total area that is not allocated to roads
    // Verhaeltnis Bebauungsflaeche zu Gesamtflaeche
    // old: fbant (ant = Anteil)
    inline float areaFractionMain() {
        return fractionOfTotalArea(mainArea);
    }

    // Fraction of total area that is allocated to roads
    // Verhaeltnis Strassenflaeche zu Gesamtflaeche
    // old: fsant (ant = Anteil)
    inline float areaFractionRoad() {
        return fractionOfTotalArea(roadArea);
    }

    inline float fractionOfTotalArea(float area) {
        float total = totalArea_FLAECHE();
        assert(total > 0);
        assert(area <= total);
        return area / total;
    }

    // percentage of total sealed area
    // share of roof area [%] 'PROBAU' +
    // share of other (unbuilt) sealed areas (e.g. Hofflaechen)
    // Versiegelungsgrad bebauter Flaechen [%] ID_VER 002 N, old: VER
    inline float mainPercentageSealed() {
        return static_cast<float>(helpers::roundToInteger(
            mainFractionBuiltSealed * 100 +
            mainFractionUnbuiltSealed * 100
        ));
    }

    inline float yearlyHeightToVolumeFlow(float height) {
        return height * 3.171F * totalArea_FLAECHE() / 100000.0F;
    }

};

#endif // ABIMOINPUTRECORD_H
