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
#include <QVector>

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

// Main function using data structures, not files
extern "C" Q_DECL_EXPORT int calculateData(
    QVector<AbimoInputRecord>& inputData,
    QVector<AbimoOutputRecord>& outputData
);

class Calculation : public QObject
{
    Q_OBJECT

public:

    // Constructor
    Calculation(
        AbimoReader& dbaseReader,
        InitValues& initValues,
        QTextStream& protocolStream
    );

    // Static function to perform a "batch run"
    static void runCalculation(
        QString inputFile,
        QString configFile,
        QString outputFile,
        bool debug = false
    );

    static void runCalculationUsingData(
        QString inputFile,
        QString outputFile
    );

    static QVector<AbimoInputRecord> readAbimoInputData(QString inputFile);

    static void writeAbimoOutputData(
        QVector<AbimoOutputRecord>& outputData,
        InitValues& initValues,
        QString outputFile
    );

    // Main function to perform the calculation of the whole input table
    bool calculate(QString& outputFile, bool debug = false);

    // Get-functions
    Counters getCounters() const;
    QString getError() const;

    // Function to be called to stop a running calculation
    void stopProcessing();

    static void doCalculationsFor(
        AbimoInputRecord& input,
        IntermediateResults& results,
        InitValues& initValues,
        UsageConfiguration& usageConfiguration,
        Counters& m_counters,
        QTextStream& protocolStream
    );

    static int fillResultRecord(
        AbimoInputRecord& input,
        IntermediateResults& results,
        AbimoOutputRecord& output
    );

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

    static UsageTuple getUsageTuple(
        int usage,
        int type,
        UsageConfiguration& usageConfiguration,
        bool overrideIrrigationWithZero,
        Counters& counters,
        QTextStream& protocolStream,
        QString code // just for information in protocolStream
    );

    void logResults(
        AbimoInputRecord& inputRecord,
        IntermediateResults& results,
        bool logBagrovIntermediates = false
    );

    void logVariable(QString name, int value);
    void logVariable(QString name, float value);
    void logVariable(QString name, char value);

    static SoilProperties getSoilProperties(
        Usage usage,
        int yield,
        float depthToWaterTable,
        int fieldCapacity_30,
        int fieldCapacity_150
    );

    static Precipitation getPrecipitation(
        int precipitationYear,
        int precipitationSummer,
        float correctionFactor
    );

    static PotentialEvaporation getPotentialEvaporation(
        InitValues& initValues,
        bool isWaterbody,
        int district,
        QString code,
        Counters& counters,
        QTextStream& protocolStream
    );

    static float getInitialValueOrDefault(
        int bez,
        QString code,
        QHash<int, int> &hash,
        int defaultValue,
        QString name,
        Counters& counters,
        QTextStream& protocolStream
    );

    static void handleTotalAreaOfZero(
        AbimoInputRecord& input,
        Counters& counters
    );

    static float actualEvaporation(
        UsageTuple& usageTuple,
        PotentialEvaporation& potentialEvaporation,
        SoilProperties& evaporationVars,
        Precipitation& precipitation,
        float& bagrovEffectivity
    );

    static void writeResultRecord(
        AbimoOutputRecord& output,
        AbimoWriter& writer
    );

    int progressNumber(int i, int n, float max);
};

#endif
