#include "effectivenessunsealed.h"

#include "constants.h"
#include "pdr.h"

// parameter values x1, x2, x3, x4 and x5 (one column each)
// for calculating the effectiveness parameter n for unsealed surfaces
const float EffectivenessUnsealed::EFFECTIVENESS_PARAMETER_VALUES[]= {
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

const float EffectivenessUnsealed::G02_VALUES[] = {
    0.0F,   0.0F,  0.0F,  0.0F,  0.3F,  0.8F,  1.4F,  2.4F,  3.7F,  5.0F,
    6.3F,   7.7F,  9.3F, 11.0F, 12.4F, 14.7F, 17.4F, 21.0F, 26.0F, 32.0F,
    39.4F, 44.7F, 48.0F, 50.7F, 52.7F, 54.0F, 55.0F, 55.0F, 55.0F, 55.0F,
    55.0F
};

EffectivenessUnsealed::EffectivenessUnsealed()
{
}

/*
================================================================================
 g e t N U V (Pointer auf aktuellen DataRecord)
 Berechnung eines Records (abgespecktes Raster)
================================================================================
*/
float EffectivenessUnsealed::calculate(PDR &record)
{
    int usableFieldCapacityIndex = (int)(record.usableFieldCapacity + 0.5);

    float g02 = G02_VALUES[usableFieldCapacityIndex];

    if (record.usage == Usage::forested_W) {
        return bag0_forest(g02);
    }

    return bag0_default(
        g02,
        record.yieldPower,
        record.irrigation,
        record.precipitationSummer == 0 &&
            record.potentialEvaporationSummer == 0
    );
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

float EffectivenessUnsealed::bag0_default(
    float g02, int yield, int irrigation, bool notSummer
)
{
    int k = (int) ((yield > 49) ? (yield / 10 + 5) : (yield / 5));

    if (k <= 0) {
        k = 1;
    }

    if (k >= 4) {
        k--;
    }

    k = 5 * MIN(k, 13) - 2;

    float result =
        EFFECTIVENESS_PARAMETER_VALUES[k - 1] +
        EFFECTIVENESS_PARAMETER_VALUES[k] * g02 +
        EFFECTIVENESS_PARAMETER_VALUES[k + 1] * g02 * g02;

    if ((result >= 2.0 && yield < 60) || (g02 >= 20.0 && yield >= 60)) {
        result =
            EFFECTIVENESS_PARAMETER_VALUES[k - 3] * g02 +
            EFFECTIVENESS_PARAMETER_VALUES[k - 2];
    }

    // Modifikation, wenn keine Sommerwerte
    if (irrigation > 0 && notSummer) {
        result *= nonSummerCorrectionFactor(irrigation);
    }

    return result;
}

float EffectivenessUnsealed::nonSummerCorrectionFactor(int irrigation)
{
    return
        0.9985F +
        0.00284F * irrigation -
        0.00000379762F * irrigation * irrigation;
}
