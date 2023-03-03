/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef PDR_H
#define PDR_H

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
    Usage usage;
    int yield;
    int irrigation;
};

class PDR
{
public:
    PDR();
    void setUsageYieldIrrigation(Usage usage, int yield = 0, int irrigation = 0);
    void setUsageYieldIrrigation(UsageTuple tuple);
    static float estimateWaterHoldingCapacity(int f30, int f150, bool isForest);
    static int estimateDaysOfGrowth(Usage usage, int yield);

    // Elementnummer EB_INDEX neu
    unsigned wIndex;

    // nFK-Wert (ergibt sich aus Bodenart) ID_NFK neu
    // water holding capacity (= nutzbare Feldkapazitaet)
    float usableFieldCapacity; // old: nFK

    // Flurabstandswert [m] ID_FLW 4.1 N
    float depthToWaterTable; // old: FLW

    // Hauptnutzungsform [L,W,G,B,K,D] ID_NUT 001 C
    Usage usage; // old: NUT

    // Langjaehriger MW des Gesamtabflusses [mm/a] 004 N
    int totalRunoff; // old: R

    // Langjaehriger MW des Regenwasserabflusses [mm/a] 003 N
    int runoff; // old: ROW

    // Langjaehriger MW des unterird. Abflusses [mm/a] 004 N
    int infiltrationRate; // old: RI

    // Versiegelungsgrad bebauter Flaechen [%] ID_VER 002 N
    int nonRoadPercentageSealed; // old: VER

    // Ertragsklasse landw. Nutzflaechen ID_ERT 002 N
    int yieldPower; // old: ERT

    // j. Beregnungshoehe landw. Nutzfl. [mm/a] ID_BER 003 N
    int irrigation; // old: BER

    // Niederschlag <MD-Wert> [mm/a] ID_PMD 003 N
    float precipitationYear; // old: P1

    // l-j. MW der pot. Verdunstung [mm/a] ID_ETP 003 N
    int longtimeMeanPotentialEvaporation; // old: ETP

    // Kapillarer Aufstieg pro Jahr ID_KR neu
    int meanPotentialCapillaryRiseRate; // old: KR

    // Sommer-Niederschlag ID_PS neu
    float precipitationSummer; // old: P1S

    // potentielle Verdunstung im Sommer ID_ETPS neu
    int potentialEvaporationSummer; // old: ETPS
};

#endif
