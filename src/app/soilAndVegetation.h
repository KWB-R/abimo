/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef SOILANDVEGETATION_H
#define SOILANDVEGETATION_H

#include <vector>
#include "structs.h"

class SoilAndVegetation
{
private:
    const static std::vector<float> POTENTIAL_RATES_OF_ASCENT;
    const static std::vector<float> USABLE_FIELD_CAPACITIES;
    const static std::vector<float> MEAN_POTENTIAL_CAPILLARY_RISE_RATES_SUMMER;

public:
    static int getMeanPotentialCapillaryRiseRate(
        float potentialCapillaryRise,
        float usableFieldCapacity,
        Usage usage,
        int yieldPower
    );

    static float estimateWaterHoldingCapacity(int f30, int f150, bool isForest);
    static int estimateDaysOfGrowth(Usage usage, int yield);
};

#endif
