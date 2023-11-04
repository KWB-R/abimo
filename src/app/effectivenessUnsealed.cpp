#include "effectivenessUnsealed.h"

#include <vector>

#include "helpers.h"

BagrovUnsealed EffectivenessUnsealed::getEffectivityParameter(
    UsageTuple& usageTuple,
    float usableFieldCapacity,
    float precipitationSummer,
    int potentialEvaporationSummer,
    float meanPotentialCapillaryRiseRate
)
{
    BagrovUnsealed result;

    bool isForest = usageTuple.usage == Usage::forested_W;

    // hsonne: these expressions are not the opposites of each other!
    bool isWet = precipitationSummer > 0.0 &&
        potentialEvaporationSummer > 0;

    bool isDry = !(precipitationSummer > 0.0) &&
        potentialEvaporationSummer == 0;

    float g02 = tableLookup_G02(usableFieldCapacity);

    if (isForest) {
        result.effectivity = bag0_forest(g02);
    } else {
        result.effectivity = tableLookup_parameter(g02, usageTuple.yield);
        // multiply with modification factor if we are not in summer
        if (usageTuple.irrigation > 0 && isDry) {
            result.factorDry = dryCorrectionFactor(usageTuple.irrigation);
            result.effectivity *= result.factorDry;
        }
    }

    if (isWet) {

        float height = static_cast<float>(
            precipitationSummer +
            usageTuple.irrigation +
            meanPotentialCapillaryRiseRate
        );

        result.factorWet = wetCorrectionFactor(height / potentialEvaporationSummer);
        result.effectivity *= result.factorWet;
    }

    return result;
}

float EffectivenessUnsealed::tableLookup_G02(float usableFieldCapacity)
{
    static const std::vector<float> G02_VALUES = {
        0.0F,   0.0F,  0.0F,  0.0F,  0.3F,  0.8F,  1.4F,  2.4F,  3.7F,  5.0F,
        6.3F,   7.7F,  9.3F, 11.0F, 12.4F, 14.7F, 17.4F, 21.0F, 26.0F, 32.0F,
        39.4F, 44.7F, 48.0F, 50.7F, 52.7F, 54.0F, 55.0F, 55.0F, 55.0F, 55.0F,
        55.0F
    };

    return G02_VALUES.at(static_cast<int>(usableFieldCapacity + 0.5));
}

float EffectivenessUnsealed::bag0_forest(float g02)
{
    if (g02 <= 10.0) {
        return 3.0F;
    }

    if (g02 <= 25.0) {
        return 4.0F;
    }

    return 8.0F;
}

float EffectivenessUnsealed::tableLookup_parameter(float g02, int yield)
{
    // parameter values x1, x2, x3, x4 and x5 (one column each)
    // for calculating the effectiveness parameter n for unsealed surfaces
    static const std::vector<float> EFFECTIVENESS_PARAMETER_VALUES = {
        0.04176F, -0.647F , 0.218F  ,  0.01472F, 0.0002089F,
        0.04594F, -0.314F , 0.417F  ,  0.02463F, 0.0001143F,
        0.05177F, -0.010F , 0.596F  ,  0.02656F, 0.0002786F,
        0.05693F,  0.033F , 0.676F  ,  0.0279F , 0.00035F  ,
        0.06162F,  0.176F , 0.773F  ,  0.02809F, 0.0004695F,
        0.06962F,  0.24F  , 0.904F  ,  0.02562F, 0.0007149F,
        0.0796F ,  0.31F  , 1.039F  ,  0.0288F , 0.0008696F,
        0.07998F,  0.7603F, 1.2F    ,  0.0471F , 0.000293F ,
        0.08762F,  1.019F , 1.373F  ,  0.04099F, 0.0014141F,
        0.11833F,  1.1334F, 1.95F   ,  0.0525F , 0.00125F  ,
        0.155F  ,  1.5F   , 2.64999F,  0.0725F , 0.001249F ,
        0.20041F,  2.0918F, 3.69999F,  0.08F   , 0.001999F ,
        0.33895F,  3.721F , 6.69999F, -0.07F   , 0.013F
    };

    // hsonne: do we expect yield to be between 0 and 100?
    //assert(yield >= 0);
    //assert(yield <= 100);

    // yield <  50 -> k = INTEGER(yield / 5)
    // yield >= 50 -> k = INTEGER(yield / 10 + 5)
    int k = static_cast<int>((yield < 50) ? (yield / 5) : (yield / 10 + 5));

    // make sure that k is at least 1
    if (k <= 0) {
        k = 1;
    }

    // if k is at least 4, reduce it by one (hsonne: why?)
    if (k >= 4) {
        k--;
    }

    // make sure that k is less or equal to 13 (hsonne: what happens here?)
    k = 5 * helpers::min(k, 13) - 2;

    float result =
        EFFECTIVENESS_PARAMETER_VALUES.at(k - 1) +
        EFFECTIVENESS_PARAMETER_VALUES.at(k) * g02 +
        EFFECTIVENESS_PARAMETER_VALUES.at(k + 1) * g02 * g02;

    if ((result >= 2.0 && yield < 60) || (g02 >= 20.0 && yield >= 60)) {
        result =
            EFFECTIVENESS_PARAMETER_VALUES.at(k - 3) * g02 +
            EFFECTIVENESS_PARAMETER_VALUES.at(k - 2);
    }

    return result;
}

float EffectivenessUnsealed::dryCorrectionFactor(int irrigation)
{
    return
        0.9985F +
        0.00284F * irrigation -
        0.00000379762F * irrigation * irrigation;
}

float EffectivenessUnsealed::wetCorrectionFactor(float wa)
{
    static const std::vector<float> VALUES_WA = {
        0.45F, 0.50F, 0.55F, 0.60F, 0.65F, 0.70F, 0.75F, // 0 ..  6
        0.80F, 0.85F, 0.90F, 0.95F, 1.00F, 1.05F, 1.10F  // 7 .. 13
    };

    static const std::vector<float> VALUES_F = {
        0.65F, 0.75F, 0.82F, 0.90F, 1.00F, 1.06F, 1.15F, // 0 ..  6
        1.22F, 1.30F, 1.38F, 1.47F, 1.55F, 1.63F, 1.70F  // 7 .. 13
    };

    return helpers::interpolate(wa, VALUES_WA, VALUES_F);
}
