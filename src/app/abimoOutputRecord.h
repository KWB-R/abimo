#ifndef ABIMOOUTPUTRECORD_H
#define ABIMOOUTPUTRECORD_H

#include <QString>

extern "C" class Q_DECL_EXPORT AbimoOutputRecord
{
public:
    AbimoOutputRecord();
    QString code_CODE;
    float totalRunoff_R;
    float surfaceRunoff_ROW;
    float infiltration_RI;
    float totalRunoffFlow_RVOL;
    float surfaceRunoffFlow_ROWVOL;
    float infiltrationFlow_RIVOL;
    float totalArea_FLAECHE;
    float evaporation_VERDUNSTUN;
};

#endif // ABIMOOUTPUTRECORD_H
