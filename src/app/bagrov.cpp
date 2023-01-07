/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <QDebug>
#include <math.h>

#include "bagrov.h"

#define ALMOST_ONE 0.99999F
#define ALMOST_ZERO 1.0e-07F

#define UPPER_LIMIT_EYN 0.7F

#define ONE_THIRD 1.0F / 3.0F
#define TWO_THIRDS 2.0F / 3.0F

// Define macros to calculate the minimum or maximum of two values
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
================================================================================
Translated by FOR_C, v2.3.2, on 10/16/94 at 18:28:43 ;
FOR_C Options SET: none ;
include <stdio.h> ;
#include <f_rt.h> ;
NBAGRO & BAGROV - Module zur Lesung der BAGROV-Gleichung
AUFRUF:
  CALL NBAGRO(BAG,y,x)
  CALL BAGROV(BAG,x,y)
PROGRAMMTYP: SUBROUTINE,
SPRACHE: FORTRAN80
EINGABEPARAMETER:
  BAG- Bagrov-Parameter mit n=BAG
  x - x=P/ETP mit
    P=PMD*niedKorrFaktor [mm/a]
    ETP::Mittlere potentielle Verdunstung [mm/a]
AUSGABEPARAMETER:
  y - ETR/ETP mit
    ETR::Mittlere reale Verdunstung [mm/a]
================================================================================
*/

Bagrov::Bagrov()
{
}

const float Bagrov::aa[]= {
    0.9946811499F, //  0
    1.213648255F,  //  1
    -1.350801214F, //  2
    11.80883489F,  //  3
    -21.53832235F, //  4
    19.3775197F,   //  5
    0.862954876F,  //  6
    9.184851852F,  //  7
    -147.2049991F, //  8
    1291.164889F,  //  9
    -6357.554955F, // 10
    19022.42165F,  // 11
    -35235.40521F, // 12
    39509.02815F,  // 13
    -24573.23867F, // 14
    6515.556685F   // 15
};

float Bagrov::nbagro(float bage, float x)
{
    int i, ia, ie, j;
    float bag, bag_plus_one, reciprocal_bag_plus_one;
    float a, a0, a1, a2, b, c, epa, eyn, h13, h23, sum_1, sum_2, w, y0;

    // General helper variable of type float
    float h;

    // If input value x is already below a threshold, return 0.0
    if (x < 0.0005F) {
        return 0.0F;
    }

    // Set input value x to 15.0 at maximum
    x = MIN(x, 15.0F);

    // Set local variable bag to value of parameter bage (20.0 at maximum)
    bag = MIN(bage, 20.0);

    // Calculate expressions that are based on bag
    bag_plus_one = bag + 1.0F;
    reciprocal_bag_plus_one = (float) (1.0 / bag_plus_one);

    h13 = (float) exp(-bag_plus_one * 1.09861);
    h23 = (float) exp(-bag_plus_one * 0.405465);

    // KOEFFIZIENTEN DER BEDINGUNGSGLEICHUNG
    a2 = -13.5F * reciprocal_bag_plus_one * (1.0F + 3.0F * (h13 - h23));
    a1 = 9.0F * reciprocal_bag_plus_one * (h13 + h13 - h23) - TWO_THIRDS * a2;
    a0 = 1.0F / (1.0F - reciprocal_bag_plus_one - 0.5F * a1 - ONE_THIRD * a2);

    // Multiply each of a1, a2 with a0
    a1 *= a0;
    a2 *= a0;

    // KOEFFIZIENTEN DES LOESUNSANSATZES
    b = (bag >= 0.49999F) ?
        (- (float) sqrt(0.25 * a1 * a1 - a2) + 0.5F * a1) :
        (- (float) sqrt(0.5F * a1 * a1 - a2));

    c = a1 - b;
    a = a0 / (b - c);

    epa = (float) exp(x / a);

    // NULLTE NAEHERUNGSLOESUNG (1. Naeherungsloesung)
    // Limit y0 to its maximum allowed value
    y0 = MIN((epa - 1.0F) / (b - c * epa), ALMOST_ONE);

    // If bag is between a certain range return y0
    if (bag >= 0.7F && bag <= 3.8F) {
        return y0;
    }

    // NUMERISCHE INTEGRATION FUER BAG > 3.8 (3. Naeherungsloesung)
    if (bag >= 3.8F) {
        h = 1.0F;
        i = 0;
        while(fabs(h) > 0.001 && i < 15) {
            y0 = MIN(y0, 0.999F);
            epa = (float) exp(bag * log(y0));
            h = MIN(MAX(1.0F - epa, ALMOST_ZERO), ALMOST_ONE);
            h *= (y0 + epa * y0 / (float) (h - bag * epa / (float) log(h)) - x);
            y0 -= h;
            i++;
        }

        // Return y0 (1.0 at maximum)
        return MIN(y0, 1.0);
    }

    // NUMERISCHE INTEGRATION FUER BAG<0.7 (2.Naeherungsloesung)
    //j = 1;

    while (true/*j <= 30*/)
    {
        eyn = (float) exp(bag * log(y0));

        // If eyn, bag are in a certain range, return y0 (1.0 at maximum)
        if ((eyn > 0.9F) || (eyn >= UPPER_LIMIT_EYN && bag > 4.0F)) {
            //*y = MIN(y0, 1.0);
            return MIN(y0, 1.0);
        }

        // Set start and end index (?), depending on the value of eyn
        if (eyn > UPPER_LIMIT_EYN) {
            ia = 8;
            ie = 16;
        }
        else {
            ia = 2;
            ie = 6;
        }

        sum_1 = 0.0F;
        sum_2 = 0.0F;
        h = 1.0F;

        // Let i loop between start index ia and end index ie
        for (i = ia; i <= ie; i++)
        {
            h *= eyn;
            w = aa[i - 1] * h;
            j = i - ia + 1; /* cls J=I-IA+1 */
            sum_2 += w / (j * (float) bag + 1.0F);
            sum_1 += w;
        }

        h = aa[ia - 2];
        h = (x - y0 * sum_2 - y0 * h) / (h + sum_1);

        y0 += h;

        // Break out of this loop if a condition is met
        if (fabs(h) / y0 < 0.007F) {
            break;
        }

        //j++;
    }

    if (y0 > 0.9) {
        bagrov(&bag, &x, &y0);
    }
    else {
      //qDebug() << "y0 <= 0.9 -> not calling bagrov()";
    }

    // Return y0 (1.0 at maximum)
    return MIN(y0, 1.0);
}

/*
================================================================================
FIXME:
================================================================================
*/
void Bagrov::bagrov(float *bagf, float *x0, float *y0)
{
    bool doloop; /* LOGICAL16 */
    int i, ii, j;

    qDebug() << "In bagrov()...";

    /* meiko : initialisiere i (einzige Aenderung) */
    i = 0;

    float delta, du, h, s, sg, si, su, u, x;

    if (*x0 == 0.0) {
        *y0 = 0.0F;
        return;
    }

    *y0 = 0.99F;

    doloop = false;

    /* NUMERISCHE INTEGRATION DER BAGROVBEZIEHUNG */
L_21:
    j = 1;
    du = 2.0F **y0;
    h = 1.0F + 1.0F / (1.0F - (float) exp(*bagf * log(*y0)));
    si = h * du / 4.0F;
    sg = 0.0F;
    su = 0.0F;

    do {
        s = si;
        j = j * 2;
        du = du / 2.0F;
        u = du / 2.0F;
        sg = sg + su;
        su = 0.0F;

        for (ii = 1; ii <= j; ii += 2) {
            su += 1.0F / (1.0F - (float) exp(*bagf * log(u)));
            u += du;
        }

        si = (2.0F * sg + 4.0F * su + h) * du / 6.0F;
    }
    while (fabs(s - si) > 0.001F * s);

    x = si;

    /* ENDE DER NUMERISCHEN INTEGRATION */
    if (doloop) {
        goto L_42;
    }

    if (*x0 > x) {
        *y0 = 1.0F;
        return;
    }

    *y0 = 0.5F;

    i = 1;

    /* SCHLEIFE I=1(1)10 ZUR BERECHNUNG VON DELTA */

L_41:
    doloop = true;
    goto L_21;

L_42:
    delta = (*x0 - x) * (1.0F - (float) exp(*bagf * (float) log(*y0)));
    *y0 = *y0 + delta;

    if (*y0 >= 1.0) {
        *y0 = 0.99F;
        goto L_90;
    }

    if (*y0 <= 0.0) {
        *y0 = 0.01F;
        goto L_90;
    }

    if (fabs(delta) < 0.01F) {
        return;
    }

L_90:
    if (i < 10) {
        i = i + 1;
        goto L_41;
    }

}	/* end of function */
