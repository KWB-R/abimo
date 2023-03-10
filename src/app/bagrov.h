/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef BAGROV_H
#define BAGROV_H

class Bagrov
{
public:
    static float nbagro(float bage, float x);
    static void bagrov(float *bagf, float *x0, float *y0);
private:
    const static float aa[];
};

#endif
