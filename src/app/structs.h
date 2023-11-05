#ifndef STRUCTS_H
#define STRUCTS_H

#include <QString>
#include <QVector>

// Descriptions from here:
// https://www.berlin.de/umweltatlas/_assets/wasser/wasserhaushalt/
//   de-abbildungen/maxsize_405ab3ac7c0e2104d3a03c317ddd93f0_a213_02a.jpg
enum struct Usage: char {
    // landwirtschaftliche Nutzflaeche (einschliesslich Graland)
    agricultural_L = 'L',
    // forstliche Nutzflaeche (Annahme gleichmaessig verteilter Bestandsaltersgruppen)
    forested_W = 'W',
    // Gewaesserflaeche
    waterbody_G = 'G',
    // gaertnerische Nutzflaeche (programmintern: BER = 75 mm/a)
    horticultural_K = 'K',
    // vegetationslose Flaeche
    vegetationless_D = 'D',
    // initial value
    unknown = '?'
};

enum struct OutputColumn {
    None = 0,
    TotalRunoff_R = 10,
    SurfaceRunoff_ROW = 11,
    Infiltration_RI = 12,
    TotalRunoffFlow_RVOL = 13,
    SurfaceRunoffFlow_ROWVOL = 14,
    InfiltrationFlow_RIVOL = 15,
    TotalArea_FLAECHE = 16,
    Evaporation_VERDUNSTUN = 17,
    Invalid = 99
};

struct IntegerRange {
    int from;
    int to;
};

struct UsageResult {
    int tupleIndex;
    QString message;
};

struct UsageTuple {
    // Hauptnutzungsform [L,W,G,B,K,D] ID_NUT 001 C
    Usage usage; // old: NUT

    // Ertragsklasse landw. Nutzflaechen ID_ERT 002 N
    int yield; // old: ERT

    // j. Beregnungshoehe landw. Nutzfl. [mm/a] ID_BER 003 N
    int irrigation; // old: BER
};

struct PotentialEvaporation {
    // l-j. MW der pot. Verdunstung [mm/a] ID_ETP 003 N
    int perYearInteger; // old: ETP

    // potentielle Verdunstung im Sommer ID_ETPS neu
    int inSummerInteger; // old: ETPS

    float perYearFloat;
};

struct Precipitation {

    // precipitation for entire year and for summer season only
    int perYearInteger;
    int inSummerInteger;

    float perYearCorrectedFloat;
    float inSummerFloat;
};

struct SoilProperties {

    // nFK-Wert (ergibt sich aus Bodenart) ID_NFK neu
    // water holding capacity (= nutzbare Feldkapazitaet)
    float usableFieldCapacity = 0.0f; // old: nFK

    // pot. Aufstiegshoehe TAS = FLUR - mittl. Durchwurzelungstiefe TWS
    // potentielle Aufstiegshoehe
    float potentialCapillaryRise_TAS = 0.0f;
    float meanPotentialCapillaryRiseRateRaw = 0.0f;
    float meanPotentialCapillaryRiseRate = 0.0f;

    // Flurabstandswert [m] ID_FLW 4.1 N
    // depth to groundwater table 'FLUR'
    float depthToWaterTable; // old: FLUR, FLW
};

struct Evaporation {
    float roof = 0.0;
    float unsealed = 0.0;
};

struct BagrovUnsealed {
    float factorWet = 1;
    float factorDry = 1;
    float effectivity = 0;
};

// Abfluesse nach Bagrov
struct RunoffSealed {

    // Abfluesse nach Bagrov fuer Daecher
    float roof = 0.0;

    // Abfluesse nach Bagrov fuer Oberflaechenklassen 1 bis 4
    QVector<float> surface = {0.0, 0.0, 0.0, 0.0};
};

// Abflussvariablen der versiegelten Flaechen
// runoff variables of sealed surfaces
struct Runoff {
    float roof = 0.0;

    // old: row1 - row4
    QVector<float> sealedSurface = {0.0, 0.0, 0.0, 0.0};

    // Calculate runoff RUV for unsealed surfaces
    // runoff for unsealed partial surfaces
    float unsealedSurface_RUV = 0.0;

    // Abfluss von unversiegelten Strassenflaechen
    // runoff from unsealed road surfaces
    // old: rowuvs
    // consider unsealed road surfaces as pavement class 4 (???)
    // old: 0.11F * (1-vgs) * fsant * R4V;
    float unsealedRoads = 0.0;
};

// Infiltrationsvariablen der versiegelten Flaechen
// infiltration variables of sealed surfaces
struct Infiltration {

    // infiltration of/for/from? roof surfaces (Infiltration der Dachflaechen)
    // old: rid
    float roof = 0.0;

    // old: ri1 - ri4
    QVector<float> surface = {0.0, 0.0, 0.0, 0.0};

    // Infiltration unversiegelter Flaechen
    // infiltration of unsealed areas
    // old: riuv
    // runoff for unsealed surfaces rowuv = 0 (???)
    float unsealedSurfaces = 0.0;

    // Infiltration von unversiegelten Strassenflaechen
    // infiltration for/from unsealed road surfaces
    // old: riuvs
    // old: 0.89F * (1-vgs) * fsant * R4V;
    float unsealedRoads = 0.0;
};

struct BagrovIntermediates {
    int i = -1;
    int j = -1;
    float a = -1.0;
    float a0 = -1.0;
    float a1 = -1.0;
    float a2 = -1.0;
    float b = -1.0;
    float bag = -1.0;
    float bag_plus_one = -1.0;
    float c = -1.0;
    float epa = -1.0;
    float eyn = -1.0;
    float h = -1.0;
    float h13 = -1.0;
    float h23 = -1.0;
    float reciprocal_bag_plus_one = -1.0;
    float sum_1 = -1.0;
    float sum_2 = -1.0;
    float w = -1.0;
    float x = -1.0;
    float y0 = -1.0;
};

#endif // STRUCTS_H
