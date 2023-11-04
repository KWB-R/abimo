#ifndef EFFECTIVENESSUNSEALED_H
#define EFFECTIVENESSUNSEALED_H

#include "soilAndVegetation.h"

class EffectivenessUnsealed
{
public:
    static BagrovUnsealed getEffectivityParameter(
        UsageTuple& usageTuple,
        float usableFieldCapacity,
        float precipitationSummer,
        int potentialEvaporationSummer,
        float meanPotentialCapillaryRiseRate
    );

private:
    static float tableLookup_G02(float usableFieldCapacity);
    static float bag0_forest(float g02);
    static float tableLookup_parameter(float g02, int yield);
    static float dryCorrectionFactor(int irrigation);
    static float wetCorrectionFactor(float wa);
};

#endif // EFFECTIVENESSUNSEALED_H
