/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef CALCULATION_H
#define CALCULATION_H

#include <QObject>
#include <QString>
#include <QTextStream>

#include "dbaseReader.h"
#include "initvalues.h"
#include "config.h"

struct Counters {

    // Total records written
    int recordsWritten;

    // Total records read
    int recordsRead;

    // Number of records for which irrigation (BER) was set to zero
    int irrigationForcedToZero;

    // Number of cases in which no calculation was performed
    long noAreaGiven;
    long noUsageGiven;

    // Number of records written to the (error) protocol
    long recordsProtocol;
};

class Calculation: public QObject
{
    Q_OBJECT

public:

    // Constructor
    Calculation(
            DbaseReader &dbaseReader,
            InitValues &initValues,
            QTextStream & protocolStream
    );

    // Static function to perform a "batch run"
    static void runCalculation(
            QString inputFile,
            QString configFile,
            QString outputFile,
            bool debug = false
    );

    // Member function to perform the calculation on an instance of the
    // Calculation class
    bool calculate(QString outputFile, bool debug = false);

    // Accessor functions to specific counters (never used!)
    long getNumberOfProtocolRecords();
    long getNumberOfNoAreaGiven();
    long getNumberOfNoUsageGiven();

    Counters getCounters();

    QString getError();

    void stopProcessing();

signals:
    void processSignal(int, QString);

private:
    Config *config;

    const static float POTENTIAL_RATES_OF_ASCENT[];
    const static float USABLE_FIELD_CAPACITIES[];
    const static float MEAN_POTENTIAL_CAPILLARY_RISE_RATES_SUMMER[];

    InitValues &initValues;
    QTextStream &protocolStream;
    DbaseReader &dbReader;
    PDR resultRecord; // old: ptrDA
    QString error;

    // *** vorlaeufig aus Teilblock 0 wird fuer die Folgeblocks genommen
    float precipitationYear; // old: regenja
    float precipitationSummer; // old: regenso

    // Abfluesse nach Bagrov fuer N1 bis N4
    float bagrovValueRoof; // old: RDV
    float bagrovValueSurface1; // old: R1V
    float bagrovValueSurface2; // old: R2V
    float bagrovValueSurface3; // old: R3V
    float bagrovValueSurface4; // old: R4V

    // runoff for unsealed partial surfaces
    float unsealedSurfaceRunoff; // old: RUV

    // Regenwasserabfluss in Qubikzentimeter pro Sekunde
    float surfaceRunoffFlow; // old: ROWVOL

    // unterirdischer Gesamtabfluss in qcm/s
    float infiltrationFlow; // old: RIVOL

    // Gesamtabfluss in qcm/s
    float totalRunoffFlow; // old: RVOL

    // potentielle Aufstiegshoehe
    float potentialCapillaryRise; // old: TAS

    // Length of array POTENTIAL_RATES_OF_ASCENT
    int n_POTENTIAL_RATES_OF_ASCENT; // old: lenTAS

    // Length of array USABLE_FIELD_CAPACITIES
    int n_USABLE_FIELD_CAPACITIES; // old: lenS

    Counters counters;

    // Variable to control whether to stop processing
    bool continueProcessing;

    // Methods

    //float getNUV(PDR &B); // now EffectivenessUnsealed::calculate()

    float getSummerModificationFactor(float wa);

    //float getG02 (int nFK); // now EffectivenessUnsealed::getG02

    // old: getNUTZ()
    void getUsage(int usageID, int type, int f30, int f150, QString code);

    void setUsageYieldIrrigation(int usageID, int type, QString code);

    void logNotDefined(QString code, int type);

    void getClimaticConditions(int bez, QString codestr);

    float initValueOrReportedDefaultValue(
        int bez, QString code, QHash<int, int> &hash, int defaultValue,
        QString name
    );
};

#endif
