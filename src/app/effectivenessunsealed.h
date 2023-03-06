#ifndef EFFECTIVENESSUNSEALED_H
#define EFFECTIVENESSUNSEALED_H

#include "pdr.h"

class EffectivenessUnsealed
{
private:
    const static float EFFECTIVENESS_PARAMETER_VALUES[];
    const static float G02_VALUES[];
    static float bag0_forest(float g02);
    static float bag0_default(float g02, int yield, int irrigation, bool notSummer);
    static float nonSummerCorrectionFactor(int irrigation);

public:
    EffectivenessUnsealed();
    static float calculate(PDR &record);
};

#endif // EFFECTIVENESSUNSEALED_H
