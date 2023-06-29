/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <math.h> // for abs()
#include <vector>

#include "usageConfiguration.h"
#include "helpers.h"
#include "soilAndVegetation.h"

// Potential rate of ascent (column labels for matrix
// meanPotentialCapillaryRiseRateSummer)
// old: iTAS
const std::vector<float> SoilAndVegetation::POTENTIAL_RATES_OF_ASCENT = {
    0.1F, 0.2F, 0.3F, 0.4F, 0.5F, 0.6F, 0.7F, 0.8F,
    0.9F, 1.0F, 1.2F, 1.4F, 1.7F, 2.0F, 2.3F
};

// soil type unknown - default soil type used in the following: sand

// Usable field capacity (row labels for matrix
// meanPotentialCapillaryRiseRateSummer)
// old: inFK_S
const std::vector<float> SoilAndVegetation::USABLE_FIELD_CAPACITIES = {
    8.0F, 9.0F, 14.0F, 14.5F, 15.5F, 17.0F, 20.5F
};

// Mean potential capillary rise rate kr [mm/d] of a summer season depending on:
// - Potential rate of ascent (one column each) and
// - Usable field capacity (one row each)
// old: ijkr_S
const std::vector<float> SoilAndVegetation::MEAN_POTENTIAL_CAPILLARY_RISE_RATES_SUMMER = {
    7.0F, 6.0F, 5.0F, 1.5F, 0.5F, 0.2F, 0.1F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 5.0F, 3.0F, 1.2F, 0.5F, 0.2F, 0.1F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 3.0F, 1.5F, 0.7F, 0.3F, 0.15F, 0.1F, 0.0F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 3.0F, 2.0F, 1.0F, 0.7F, 0.4F, 0.15F, 0.1F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 4.5F, 2.5F, 1.5F, 0.7F, 0.4F, 0.15F, 0.1F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 5.0F, 3.5F, 2.0F, 1.5F, 0.8F, 0.3F, 0.1F, 0.05F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 6.0F, 5.0F, 5.0F, 5.0F, 3.0F, 2.0F, 1.0F, 0.5F, 0.15F, 0.0F, 0.0F
};

// mittlere pot. kapillare Aufstiegsrate kr (mm/d) des Sommerhalbjahres
//
// switch (bod) {
//   case S: case U: case L: case T: case LO: case HN:
// }
//
// wird eingefuegt, wenn die Bodenart in das Zahlenmaterial aufgenommen
// wird. Vorlaeufig wird Sande angenommen.
int SoilAndVegetation::getMeanPotentialCapillaryRiseRate(
        float potentialCapillaryRise,
        float usableFieldCapacity,
        Usage usage,
        int yieldPower
)
{
    float kr = (potentialCapillaryRise <= 0.0) ?
        7.0F :
        MEAN_POTENTIAL_CAPILLARY_RISE_RATES_SUMMER[
            helpers::index(potentialCapillaryRise, POTENTIAL_RATES_OF_ASCENT) +
            helpers::index(usableFieldCapacity, USABLE_FIELD_CAPACITIES) *
            POTENTIAL_RATES_OF_ASCENT.size()
        ];

    return (int)(estimateDaysOfGrowth(usage, yieldPower) * kr);
}

// mittlere Zahl der Wachstumstage
int SoilAndVegetation::estimateDaysOfGrowth(Usage usage, int yield)
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

float SoilAndVegetation::estimateWaterHoldingCapacity(int f30, int f150, bool isForest)
{
    if (helpers::min(f30, f150) < 1) {
        return 13.0F;
    }

    if (abs(f30 - f150) < helpers::min(f30, f150)) { // unwesentliche Abweichung
        return static_cast<float>(isForest ? f150 : f30);
    }

    return
        0.75F * static_cast<float>(isForest ? f150 : f30) +
        0.25F * static_cast<float>(isForest ? f30 : f150);
}
