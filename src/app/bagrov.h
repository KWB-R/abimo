/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef BAGROV_H
#define BAGROV_H

#include "structs.h"

class Bagrov
{
public:
    static float realEvapoTranspiration(
        float precipitation,
        float potentialEvaporation,
        float efficiency,
        BagrovIntermediates& bagrovIntermediates
    );
    static float yRatio(float bage, float x, BagrovIntermediates& bi);
    static void bagrov(float *bagf, float *x0, float *y0);

private:
    const static float aa[];
};

#endif
