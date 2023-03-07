/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <math.h> // for abs()
#include <vector>

#include "config.h"
#include "constants.h" // for MIN() macro
#include "helpers.h"
#include "pdr.h"

// Potential rate of ascent (column labels for matrix
// meanPotentialCapillaryRiseRateSummer)
// old: iTAS
const std::vector<float> PDR::POTENTIAL_RATES_OF_ASCENT = {
    0.1F, 0.2F, 0.3F, 0.4F, 0.5F, 0.6F, 0.7F, 0.8F,
    0.9F, 1.0F, 1.2F, 1.4F, 1.7F, 2.0F, 2.3F
};

// soil type unknown - default soil type used in the following: sand

// Usable field capacity (row labels for matrix
// meanPotentialCapillaryRiseRateSummer)
// old: inFK_S
const std::vector<float> PDR::USABLE_FIELD_CAPACITIES = {
    8.0F, 9.0F, 14.0F, 14.5F, 15.5F, 17.0F, 20.5F
};

// Mean potential capillary rise rate kr [mm/d] of a summer season depending on:
// - Potential rate of ascent (one column each) and
// - Usable field capacity (one row each)
// old: ijkr_S
const std::vector<float> PDR::MEAN_POTENTIAL_CAPILLARY_RISE_RATES_SUMMER = {
    7.0F, 6.0F, 5.0F, 1.5F, 0.5F, 0.2F, 0.1F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 5.0F, 3.0F, 1.2F, 0.5F, 0.2F, 0.1F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 3.0F, 1.5F, 0.7F, 0.3F, 0.15F, 0.1F, 0.0F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 3.0F, 2.0F, 1.0F, 0.7F, 0.4F, 0.15F, 0.1F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 4.5F, 2.5F, 1.5F, 0.7F, 0.4F, 0.15F, 0.1F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 5.0F, 3.5F, 2.0F, 1.5F, 0.8F, 0.3F, 0.1F, 0.05F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 6.0F, 5.0F, 5.0F, 5.0F, 3.0F, 2.0F, 1.0F, 0.5F, 0.15F, 0.0F, 0.0F
};

PDR::PDR():
    wIndex(0),
    usableFieldCapacity(0),
    depthToWaterTable(0),
    usage(Usage::unknown),
//    totalRunoff(0),
    runoff(0),
    infiltrationRate(0),
    mainPercentageSealed(0),
    yieldPower(0),
    irrigation(0),
    precipitationYear(0.0F),
    longtimeMeanPotentialEvaporation(0),
    meanPotentialCapillaryRiseRate(0),
    precipitationSummer(0.0F),
    potentialEvaporationSummer(0)
{}

void PDR::setUsageYieldIrrigation(Usage usage, int yield, int irrigation)
{
    this->usage = usage;
    this->yieldPower = yield;
    this->irrigation = irrigation;
}

void PDR::setUsageYieldIrrigation(UsageTuple tuple)
{
    setUsageYieldIrrigation(tuple.usage, tuple.yield, tuple.irrigation);
}

// mittlere pot. kapillare Aufstiegsrate kr (mm/d) des Sommerhalbjahres
//
// switch (bod) {
//   case S: case U: case L: case T: case LO: case HN:
// }
//
// wird eingefuegt, wenn die Bodenart in das Zahlenmaterial aufgenommen
// wird. Vorlaeufig wird Sande angenommen.
int PDR::getMeanPotentialCapillaryRiseRate(
        float potentialCapillaryRise,
        float usableFieldCapacity,
        Usage usage,
        int yieldPower
)
{
    float kr = (potentialCapillaryRise <= 0.0) ?
        7.0F :
        MEAN_POTENTIAL_CAPILLARY_RISE_RATES_SUMMER[
            Helpers::index(potentialCapillaryRise, POTENTIAL_RATES_OF_ASCENT) +
            Helpers::index(usableFieldCapacity, USABLE_FIELD_CAPACITIES) *
            POTENTIAL_RATES_OF_ASCENT.size()
        ];

    return (int)(estimateDaysOfGrowth(usage, yieldPower) * kr);
}

// mittlere Zahl der Wachstumstage
int PDR::estimateDaysOfGrowth(Usage usage, int yield)
{
    switch (usage)
    {
        case Usage::agricultural_L: return (yield <= 50) ? 60 : 75;
        case Usage::horticultural_K: return 100;
        case Usage::forested_W: return 90;
        case Usage::vegetationless_D: return 50;
        default: return 50;
    }
}

float PDR::estimateWaterHoldingCapacity(int f30, int f150, bool isForest)
{
    if (MIN(f30, f150) < 1) {
        return 13.0F;
    }

    if (abs(f30 - f150) < MIN(f30, f150)) { // unwesentliche Abweichung
        return (float) (isForest ? f150 : f30);
    }

    return
        0.75F * (float) (isForest ? f150 : f30) +
        0.25F * (float) (isForest ? f30 : f150);
}
