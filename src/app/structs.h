#ifndef STRUCTS_H
#define STRUCTS_H

#include <QString>

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

#endif // STRUCTS_H