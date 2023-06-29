/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef CALCULATION_H
#define CALCULATION_H

#include <array>
#include <QObject>
#include <QString>
#include <QTextStream>

#include "abimoReader.h"
#include "abimoWriter.h"
#include "abimoInputRecord.h"
#include "abimoOutputRecord.h"
#include "usageConfiguration.h"
#include "counters.h"
#include "dbaseWriter.h"
#include "initValues.h"
#include "intermediateResults.h"
#include "soilAndVegetation.h"

class Calculation : public QObject
{
    Q_OBJECT

public:

    // Constructor
    Calculation(
        AbimoReader &dbaseReader,
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

    // Main function to perform the calculation of the whole input table
    bool calculate(QString& outputFile, bool debug = false);

    // Get-functions
    Counters getCounters() const;
    QString getError() const;

    // Function to be called to stop a running calculation
    void stopProcessing();

signals:
    void processSignal(int, QString);

private:

    //
    // All private member variables of this class will be prefixed with "m_"
    //

    // Mapping between type of area and "usage tuples" consisting of three
    // elements (main usage type, yield, irrigation)
    UsageConfiguration m_usageMappings;

    // Initial values as provided in config.xml
    InitValues& m_initValues;

    // Accessor to the log file (.log)
    QTextStream& m_protocolStream;

    // Accessor to the input file (.dbf)
    AbimoReader& m_dbReader;

    // Structure holding long and short error text
    QString m_error;

    // Structure holding different counters, required for reporting
    Counters m_counters;

    // Variable to control whether to stop processing
    bool m_continueProcessing;

    // Current prefix to be used in log file
    QString m_prefix;

    //
    // Methods
    //

    void doCalculationsFor(
        AbimoInputRecord& input,
        IntermediateResults& results
    );

    void logResults(AbimoInputRecord inputRecord, IntermediateResults results);
    void logVariable(QString name, float value);

    UsageTuple provideUsageInformation(AbimoInputRecord& input);

    EvaporationRelevantVariables setEvaporationVars(
        UsageTuple& usageTuple,
        AbimoInputRecord& input
    );

    Precipitation getPrecipitation(
        int precipitationYear,
        int precipitationSummer,
        InitValues& initValues
    );

    PotentialEvaporation getPotentialEvaporation(
        Usage& usage,
        InitValues& initValues,
        int district,
        QString code
    );

    float initValueOrReportedDefaultValue(
        int bez,
        QString code,
        QHash<int, int> &hash,
        int defaultValue,
        QString name
    );

    void setBagrovValues(
        Precipitation& precipitation,
        PotentialEvaporation& potentialEvaporation,
        BagrovValues& bagrovValues
    );

    void handleTotalAreaOfZero(AbimoInputRecord& input);

    void calculateRunoffSealed(
        AbimoInputRecord& input,
        BagrovValues& bagrovValues,
        Runoff& runoff
    );

    void calculateInfiltrationSealed(
        AbimoInputRecord& input,
        BagrovValues& bagrovValues,
        Runoff& runoff,
        Infiltration& infiltrationSealed
    );

    float actualEvaporation(
        UsageTuple& usageTuple,
        PotentialEvaporation& potentialEvaporation,
        EvaporationRelevantVariables& evaporationVars,
        Precipitation& precipitation
    );

    int fillResultRecord(
        AbimoInputRecord& input,
        IntermediateResults& results,
        AbimoOutputRecord& output
    );

    void writeResultRecord(
        AbimoOutputRecord& output,
        AbimoWriter& writer
    ) const;

    int progressNumber(int i, int n, float max);
};

#endif
