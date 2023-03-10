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
#include "abimoRecord.h"
#include "counters.h"
#include "dbaseWriter.h"
#include "initValues.h"
#include "config.h"

class Calculation: public QObject
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

    // Member function to perform the calculation on an instance of the
    // Calculation class
    bool calculate(QString outputFile, bool debug = false);

    Counters getCounters();

    QString getError();

    void stopProcessing();

signals:
    void processSignal(int, QString);

private:

    // All private member variables of this class will be prefixed with "m_"
    Config m_config;

    InitValues &m_initValues;
    QTextStream &m_protocolStream;
    AbimoReader &m_dbReader;
    PDR m_resultRecord; // old: ptrDA
    QString m_error;

    // Abfluesse nach Bagrov fuer Daecher (index 0) und Oberflaechenklassen
    // 1 bis 4 (index 1 bis 4)
    std::array<float,5> m_bagrovValues;

    // runoff for unsealed partial surfaces
    float m_unsealedSurfaceRunoff; // old: RUV

    // Regenwasserabfluss in Qubikzentimeter pro Sekunde
    float m_surfaceRunoffFlow; // old: ROWVOL

    // unterirdischer Gesamtabfluss in qcm/s
    float m_infiltrationFlow; // old: RIVOL

    // Gesamtabfluss in qcm/s
    float m_totalRunoffFlow; // old: RVOL

    // potentielle Aufstiegshoehe
    float m_potentialCapillaryRise; // old: TAS

    // Additional member variables (m_ indicates member)

    // Langjaehriger MW des Gesamtabflusses [mm/a] 004 N
    float m_totalRunoff; // old: R
    float m_surfaceRunoff;

    // Langjaehriger MW des unterird. Abflusses [mm/a] 004 N
    // old: RI
    float m_infiltration;
    float m_evaporation;

    Counters m_counters;

    // Variable to control whether to stop processing
    bool m_continueProcessing;

    // Methods

    float getSummerModificationFactor(float wa);

    void logNotDefined(QString code, int type);

    void getClimaticConditions(int bez, QString codestr);

    float realEvapotranspiration(
        float potentialEvaporation, float precipiation
    );

    float initValueOrReportedDefaultValue(
        int bez, QString code, QHash<int, int> &hash, int defaultValue,
        QString name
    );

    int progressNumber(int i, int n, float max);
    void calculateResultRecord(AbimoRecord &record);
    void writeResultRecord(AbimoRecord &record, DbaseWriter& writer);
};

#endif
