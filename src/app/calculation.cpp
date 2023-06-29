// ***************************************************************************
// * For copyright information please see COPYRIGHT in the base directory
// * of this repository (https://github.com/KWB-R/abimo).
// ***************************************************************************

#include <cfenv> // fegetround()

#include <vector>

#include <math.h>
#include <QDebug>
#include <QString>
#include <QTextStream>

#include "abimoReader.h"
#include "abimoWriter.h"
#include "abimoInputRecord.h"
#include "abimoOutputRecord.h"
#include "bagrov.h"
#include "calculation.h"
#include "usageConfiguration.h"
#include "constants.h"
#include "counters.h"
#include "effectivenessUnsealed.h"
#include "helpers.h"
#include "initValues.h"
#include "intermediateResults.h"
#include "soilAndVegetation.h"

Calculation::Calculation(
    AbimoReader& dbaseReader,
    InitValues& initValues,
    QTextStream& protocolStream
) :
    m_initValues(initValues),
    m_protocolStream(protocolStream),
    m_dbReader(dbaseReader),
    m_counters(),
    m_continueProcessing(true)
{
}

void Calculation::runCalculation(
        QString inputFile,
        QString configFile,
        QString outputFile,
        bool debug
)
{
    // Open the input file
    AbimoReader dbReader(inputFile);

    // Try to read the raw (text) values into the dbReader object
    if (!dbReader.checkAndRead()) {
        abort();
    }

    // Update default initial values with values given in configFile
    InitValues initValues;

    if (configFile.isEmpty()) {
        qDebug() << "No config file given -> Using default values";
    }
    else {
        qDebug() << "Using configuration file:" << configFile;
        QString error = InitValues::updateFromConfig(initValues, configFile);
        if (!error.isEmpty()) {
            qDebug() << "Error in updateFromConfig: " << error;
            abort();
        }
    }

    QFile logHandle(helpers::defaultLogFileName(outputFile));

    helpers::openFileOrAbort(logHandle, QFile::WriteOnly);

    QTextStream logStream(&logHandle);

    Calculation calculator(dbReader, initValues, logStream);

    bool success = calculator.calculate(outputFile, debug);

    if (!success) {
        qDebug() << "Error in calculate(): " << calculator.getError();
        abort();
    }

    logHandle.close();
}

// =============================================================================
// Diese Funktion importiert die Datensaetze aus der DBASE-Datei FileName in das
// DA Feld ein (GWD-Daten).
// Parameter: outputFile: Name der Ausgabedatei
//            debug: whether or not to show debug messages
// Rueckgabewert: BOOL. TRUE, wenn das Einlesen der Datei erfolgreich war.
// =============================================================================
bool Calculation::calculate(QString& outputFile, bool debug)
{
    // https://en.cppreference.com/w/cpp/numeric/fenv/feround

    int oldMode = std::fegetround();

    //int newMode = FE_DOWNWARD;
    //int newMode = FE_UPWARD;
    int newMode = FE_TONEAREST;
    //int newMode = FE_TOWARDZERO;

    int errCode = std::fesetround(newMode);

    m_protocolStream << QString("std::fegetround() returned: %1").arg(oldMode) << endl;
    m_protocolStream << QString("std::fesetround(%1) returned: %2").arg(newMode).arg(errCode) << errCode << endl;

    // Current Abimo record (represents one row of the input dbf file)
    AbimoInputRecord inputRecord;
    AbimoOutputRecord outputRecord;

    // Number of processed records
    int numProcessed = 0;

    // Initialise counters
    m_counters.initialise();

    // Provide an AbimoWriter object
    AbimoWriter writer(outputFile, m_initValues);

    assert(writer.getNumberOfFields() == 9);

    // Get the number of rows in the input data
    int recordCount = m_dbReader.getHeader().numberOfRecords;

    // loop over all block partial areas (= records/rows of input data)
    for (int i = 0; i < recordCount; i++) {

        // Break out of the loop if the user pressed "Cancel"
        if (!m_continueProcessing) {
            break;
        }

        // Fill record with data from the current row i
        m_dbReader.fillRecord(i, inputRecord, debug);

        // usage = integer representing the type of area usage for the current
        // block partial area
        if (inputRecord.usage == 0) {

            // Hier koennten falls gewuenscht die Flaechen dokumentiert
            // werden, deren NUTZUNG = NULL
            m_counters.incrementNoUsageGiven();
        }
        else {

            IntermediateResults results;

            // Calculate and set result record fields to calculated values
            doCalculationsFor(inputRecord, results);

            // Write all results to the log file
            logResults(inputRecord, results);

            fillResultRecord(inputRecord, results, outputRecord);

            // Set the corresponding row in the result data structure
            writeResultRecord(outputRecord, writer);

            // Increment the number of processed data rows
            numProcessed++;
        }

        // Send a signal to the progress bar dialog
        emit processSignal(progressNumber(i, recordCount, 50.0), "Berechne");
    }

    // Set counters
    m_counters.setRecordsRead(recordCount);
    m_counters.setRecordsWritten(numProcessed);

    if (!m_continueProcessing) {
        m_protocolStream << "Berechnungen abgebrochen.\r\n";
        return true;
    }

    emit processSignal(50, "Schreibe Ergebnisse.");

    if (!writer.write()) {
        QString errorText = writer.getError().textShort;
        m_protocolStream << "Error: " + errorText +"\r\n";
        m_error = "Fehler beim Schreiben der Ergebnisse.\n" + errorText;
        return false;
    }

    return true;
}

void Calculation::doCalculationsFor(
    AbimoInputRecord& input,
    IntermediateResults& results
)
{
    // Based on the given input row, try to provide usage-specific information
    UsageTuple usageTuple = provideUsageInformation(input);

    // Provide variables relevant to calculate evaporation (?)
    // all values are 0.0 in case of water bodies -> TODO: may be not a good
    // name. It is more about soil properties
    EvaporationRelevantVariables evaporationVars = setEvaporationVars(
        usageTuple, input
    );

    // Provide information on the precipitation
    Precipitation precipitation = getPrecipitation(
        input.precipitationYear, input.precipitationSummer, m_initValues
    );

    // Provide information on the potential evaporation
    PotentialEvaporation potentialEvaporation = getPotentialEvaporation(
        usageTuple.usage, m_initValues, input.district, input.code
    );

    //
    // Do the Bagrov-calculation for sealed surfaces...
    //

    // Berechnung der Abfluesse RDV und R1V bis R4V fuer versiegelte
    // Teilflaechen und unterschiedliche Bagrovwerte ND und N1 bis N4
    // - RDV / RxV: Gesamtabfluss versiegelte Flaeche

    setBagrovValues(precipitation, potentialEvaporation, results.bagrovValues);

    // Set default area if total area is zero
    handleTotalAreaOfZero(input);

    // Runoff of impervious (sealed) surfaces
    Runoff runoff;

    // Infiltration into impervious surfaces (?)
    Infiltration infiltration;

    // Calculate runoff...
    //====================

    // ... from roofs and sealed surfaces
    calculateRunoffSealed(input, results.bagrovValues, runoff);

    // ... from unsealed surfaces
    runoff.unsealedSurface_RUV =
        precipitation.perYearCorrectedFloat -
        actualEvaporation(
            usageTuple,
            potentialEvaporation,
            evaporationVars,
            precipitation
        );

    // Calculate infiltration...
    // =========================

    // ... from sealed surfaces
    calculateInfiltrationSealed(input, results.bagrovValues, runoff, infiltration);

    // ... from unsealed road surfaces
    infiltration.unsealedRoads =
        (1 - input.roadFractionSealed) *
        input.areaFractionRoad() *
        results.bagrovValues.surface.last();

    // ... from unsealed non-road surfaces
    // old: riuv
    infiltration.unsealedSurfaces = (
        100.0F - input.mainPercentageSealed()
    ) / 100.0F * runoff.unsealedSurface_RUV;

    // Set infiltration-related fields in output record
    //=================================================

    // calculate infiltration rate 'ri' for entire block partial area
    // (mm/a)
    results.infiltration_RI = (
        infiltration.roof +
        helpers::vectorSum(infiltration.surface) +
        infiltration.unsealedRoads +
        infiltration.unsealedSurfaces
    );

    // Set runoff-related fields in output record
    //===========================================

    // calculate runoff 'ROW' for entire block patial area (FLGES +
    // STR_FLGES) (mm/a)
    results.surfaceRunoff_ROW = (
        runoff.roof +
        helpers::vectorSum(runoff.sealedSurface) +
        runoff.unsealedRoads
    );

    // calculate total system losses 'r' due to runoff and infiltration
    // for entire block partial area
    results.totalRunoff_R =
        results.surfaceRunoff_ROW +
        results.infiltration_RI;

    // Convert yearly heights to flows
    //================================

    // calculate volume 'rowvol' from runoff (qcm/s)
    results.surfaceRunoffFlow_ROWVOL = input.yearlyHeightToVolumeFlow(
        results.surfaceRunoff_ROW
    );

    // calculate volume 'rivol' from infiltration rate (qcm/s)
    results.infiltrationFlow_RIVOL = input.yearlyHeightToVolumeFlow(
        results.infiltration_RI
    );

    // calculate volume of system losses 'rvol' due to runoff and
    // infiltration
    results.totalRunoffFlow_RVOL =
        results.surfaceRunoffFlow_ROWVOL +
        results.infiltrationFlow_RIVOL;

    // Set evaporation in output record
    //=================================

    // calculate evaporation 'VERDUNST' by subtracting 'R', the sum of
    // runoff and infiltration from precipitation of entire year,
    // multiplied by precipitation correction factor
    results.evaporation_VERDUNSTUN =
        precipitation.perYearCorrectedFloat -
        results.totalRunoff_R;
}

UsageTuple Calculation::provideUsageInformation(AbimoInputRecord& input)
{
    // declaration of yield power (ERT) and irrigation (BER) for agricultural or
    // gardening purposes
    UsageResult usageResult = m_usageMappings.getUsageResult(
        input.usage,
        input.type,
        input.code
    );

    if (usageResult.tupleIndex < 0) {
        m_protocolStream << usageResult.message;
        qDebug() << usageResult.message;
        abort();
    }

    if (!usageResult.message.isEmpty()) {
        m_protocolStream << usageResult.message;
        m_counters.incrementRecordsProtocol();
    }

    UsageTuple result = m_usageMappings.getUsageTuple(usageResult.tupleIndex);

    // Override irrigation value with zero if the corresponding option is set
    if (m_initValues.getIrrigationToZero() && result.irrigation != 0) {
        //*protokollStream << "Erzwinge BER=0 fuer Code: " << input.code <<
        //", Wert war:" << usageTuple.irrigation << " \r\n";
        m_counters.incrementIrrigationForcedToZero();
        result.irrigation = 0;
    }

    return result;
}

EvaporationRelevantVariables Calculation::setEvaporationVars(
    UsageTuple& usageTuple,
    AbimoInputRecord& input
)
{
    // Initialise variables that are relevant to calculate evaporation
    EvaporationRelevantVariables result;

    result.depthToWaterTable = input.depthToWaterTable;

    // Nothing to do for waterbodies
    if (usageTuple.usage == Usage::waterbody_G) {
        return result;
    }

    // Feldkapazitaet
    result.usableFieldCapacity = SoilAndVegetation::estimateWaterHoldingCapacity(
        input.fieldCapacity_30,
        input.fieldCapacity_150,
        usageTuple.usage == Usage::forested_W
    );

    // pot. Aufstiegshoehe TAS = FLUR - mittl. Durchwurzelungstiefe TWS
    // potentielle Aufstiegshoehe
    result.potentialCapillaryRise_TAS = result.depthToWaterTable -
        m_usageMappings.getRootingDepth(usageTuple.usage, usageTuple.yield);

    // mittlere pot. kapillare Aufstiegsrate kr (mm/d) des Sommerhalbjahres
    // Kapillarer Aufstieg pro Jahr ID_KR neu, old: KR
    result.meanPotentialCapillaryRiseRate =
        SoilAndVegetation::getMeanPotentialCapillaryRiseRate(
            result.potentialCapillaryRise_TAS,
            result.usableFieldCapacity,
            usageTuple.usage,
            usageTuple.yield
        );

    return result;
}

Precipitation Calculation::getPrecipitation(
    int precipitationYear,
    int precipitationSummer,
    InitValues& initValues
)
{
    Precipitation result;

    // Set integer fields (originally from input dbf)
    result.perYearInteger = precipitationYear;
    result.inSummerInteger = precipitationSummer;

    // Set float fields

    // Correct the (non-summer) precipitation (at ground level)
    result.perYearCorrectedFloat = static_cast<float>(
        precipitationYear * initValues.getPrecipitationCorrectionFactor()
    );

    // No correction for summer precipitation!
    result.inSummerFloat = static_cast<float>(
        precipitationSummer
    );

    return result;
}

PotentialEvaporation Calculation::getPotentialEvaporation(
    Usage& usage, InitValues& initValues, int district, QString code
)
{
    PotentialEvaporation result;

    // Parameter for the city districts
    if (usage == Usage::waterbody_G) {

        result.perYearInteger = initValueOrReportedDefaultValue(
            district, code, initValues.hashEG, 775, "EG"
        );

        // What about potentialEvaporationSummer?
        result.inSummerInteger = -1;
    }
    else {
        result.perYearInteger = initValueOrReportedDefaultValue(
            district, code, initValues.hashETP, 660, "ETP"
        );
        result.inSummerInteger = initValueOrReportedDefaultValue(
            district, code, initValues.hashETPS, 530, "ETPS"
        );
    }

    // no more correction with 1.1
    result.perYearFloat = static_cast<float>(result.perYearInteger);

    return result;
}

float Calculation::initValueOrReportedDefaultValue(
        int district,
        QString code,
        QHash<int,int> &hash,
        int defaultValue,
        QString name
)
{
    // Take value from hash table (as read from xml file) if available
    if (hash.contains(district)) {
        return hash.value(district);
    }

    // Default
    float result = hash.contains(0) ? hash.value(0) : defaultValue;

    QString districtString;
    districtString.setNum(district);

    QString resultString;
    resultString.setNum(result);

    m_protocolStream << QString(
        "\r\n%1 unbekannt fuer %2 von Bezirk %3\r\n%4=%5 angenommen\r\n"
    ).arg(name, code, districtString, name, resultString);

    m_counters.incrementRecordsProtocol();

    return result;
}

void Calculation::setBagrovValues(
    Precipitation& precipitation,
    PotentialEvaporation& potentialEvaporation,
    BagrovValues& bagrovValues
)
{
    // ratio of precipitation to potential evaporation
    float xRatio =
        precipitation.perYearCorrectedFloat /
        potentialEvaporation.perYearFloat;

    // Berechnung des Abflusses RxV fuer versiegelte Teilflaechen mittels
    // Umrechnung potentieller Verdunstungen potentialEvaporation zu realen
    // ueber Umrechnungsfaktor yRatio und subtrahiert von Niederschlag
    // precipitation

    // index 0 = roof, indices 1 - 4 = surface classes 1 - 4
    int numSurfacClasses = static_cast<int>(bagrovValues.surface.size());

    for (int i = 0; i < numSurfacClasses + 1; i++) {

        float bagrovValue = precipitation.perYearCorrectedFloat -
            Bagrov::nbagro(m_initValues.getBagrovValue(i), xRatio) *
            potentialEvaporation.perYearFloat;

        if (i == 0) {
            bagrovValues.roof = bagrovValue;
        }
        else {
            bagrovValues.surface[i - 1] = bagrovValue;
        }
    }
}

void Calculation::handleTotalAreaOfZero(AbimoInputRecord& input)
{
    // if sum of total building development area and road area is
    // inconsiderably small it is assumed, that the area is unknown and
    // 100 % building development area will be given by default
    if (input.totalArea_FLAECHE() < 0.0001) {
        // *protokollStream << "\r\nDie Flaeche des Elements " +
        // record.CODE + " ist 0 \r\nund wird automatisch auf 100 gesetzt\r\n";
        m_counters.incrementRecordsProtocol();
        m_counters.incrementNoAreaGiven();
        input.mainArea = 100.0F;
    }
}

void Calculation::calculateRunoffSealed(
    AbimoInputRecord& input,
    BagrovValues& bagrovValues,
    Runoff& runoff
)
{
    // runoff from roof surfaces (Abfluss der Dachflaechen), old: rowd
    runoff.roof =
        (1.0F - m_initValues.getInfiltrationFactor(0)) * // 0 = roof!
        input.mainFractionBuiltSealed *
        input.builtSealedFractionConnected *
        input.areaFractionMain() *
        bagrovValues.roof;

    for (int i = 0; i < static_cast<int>(runoff.sealedSurface.size()); i++) {

        // Abfluss Belagsflaeche i + 1, old: row<i>
        runoff.sealedSurface[i] =
            (1.0F - m_initValues.getInfiltrationFactor(i + 1)) *
            (
                input.unbuiltSealedFractionSurface.at(i + 1) *
                input.unbuiltSealedFractionConnected *
                input.mainFractionUnbuiltSealed *
                input.areaFractionMain() +
                input.roadSealedFractionSurface.at(i + 1) *
                input.roadSealedFractionConnected *
                input.roadFractionSealed *
                input.areaFractionRoad()
            ) * bagrovValues.surface[i];
    }
}

void Calculation::calculateInfiltrationSealed(
    AbimoInputRecord& input,
    BagrovValues& bagrovValues,
    Runoff& runoff,
    Infiltration& infiltrationSealed
)
{
    infiltrationSealed.roof =
        (1 - input.builtSealedFractionConnected) *
        input.mainFractionBuiltSealed *
        input.areaFractionMain() *
        bagrovValues.roof;

    for (int i = 0; i < static_cast<int>(infiltrationSealed.surface.size()); i++) {

        infiltrationSealed.surface[i] = (
            input.unbuiltSealedFractionSurface.at(i + 1) *
            input.mainFractionUnbuiltSealed *
            input.areaFractionMain() +
            input.roadSealedFractionSurface.at(i + 1) *
            input.roadFractionSealed *
            input.areaFractionRoad()
        ) * bagrovValues.surface[i] -
        runoff.sealedSurface[i];
    }
}

float Calculation::actualEvaporation(
    UsageTuple& usageTuple,
    PotentialEvaporation& potentialEvaporation,
    EvaporationRelevantVariables& evaporationVars,
    Precipitation& precipitation
)
{
    // For water bodies, return the potential evaporation
    if (usageTuple.usage == Usage::waterbody_G) {
        return potentialEvaporation.perYearFloat;
    }

    // Otherwise calculate the real evapo transpiration
    assert(potentialEvaporation.perYearFloat > 0.0);

    // Determine effectivity/effectiveness ??? parameter (old???: bag) for
    // unsealed surfaces
    // Modul Raster abgespeckt (???)
    float effectivityParameter = EffectivenessUnsealed::getEffectivityParameter(
        usageTuple,
        evaporationVars.usableFieldCapacity,
        precipitation.inSummerFloat,
        potentialEvaporation.inSummerInteger,
        evaporationVars.meanPotentialCapillaryRiseRate
    );

    // Calculate the x-factor of bagrov relation: x = (P + KR + BER)/ETP
    // Then get the y-factor: y = fbag(n, x)
    // ratio of real evaporation to potential evaporation
    float yRatio = Bagrov::nbagro(
        effectivityParameter,
        (
            precipitation.perYearCorrectedFloat +
            evaporationVars.meanPotentialCapillaryRiseRate +
            usageTuple.irrigation
        ) / potentialEvaporation.perYearFloat
    );

    // Get the real evapotransporation using estimated y-factor
    float result = yRatio * potentialEvaporation.perYearFloat;

    if (evaporationVars.potentialCapillaryRise_TAS < 0) {
        result += (potentialEvaporation.perYearFloat - result) *
            static_cast<float>(
                exp(evaporationVars.depthToWaterTable /
                evaporationVars.potentialCapillaryRise_TAS)
            );
    }

    return result;
}

void Calculation::logResults(
        AbimoInputRecord inputRecord,
        IntermediateResults results
)
{
    m_protocolStream << endl << "*** Code: " << inputRecord.code << endl;

    //m_prefix = inputRecord.code;

    logVariable("bagrov_roof", results.bagrovValues.roof);

    for (int i = 1; i < results.bagrovValues.surface.size(); i++) {
        logVariable(
            QString("bagrov_surface[%1]").arg(i),
            results.bagrovValues.surface[i]
        );
    }

    logVariable("surfaceRunoff_ROW", results.surfaceRunoff_ROW);
    logVariable("surfaceRunoffFlow_ROWVOL", results.surfaceRunoffFlow_ROWVOL);
    logVariable("infiltration_RI", results.infiltration_RI);
    logVariable("infiltrationFlow_RIVOL", results.infiltrationFlow_RIVOL);
    logVariable("totalRunoff_R", results.totalRunoff_R);
    logVariable("totalRunoffFlow_RVOL", results.totalRunoffFlow_RVOL);
    logVariable("evaporation_VERDUNSTUN", results.evaporation_VERDUNSTUN);
}

void Calculation::logVariable(QString name, float value)
{
    //m_protocolStream << m_prefix << ";" <<
    m_protocolStream << name << "=" << QString::number(value, 'g', 10) << endl;
}

int Calculation::fillResultRecord(
    AbimoInputRecord& input,
    IntermediateResults& results,
    AbimoOutputRecord& output
)
{
    output.code_CODE = input.code;
    output.totalRunoff_R = results.totalRunoff_R;
    output.surfaceRunoff_ROW = results.surfaceRunoff_ROW;
    output.infiltration_RI = results.infiltration_RI;
    output.totalRunoffFlow_RVOL = results.totalRunoffFlow_RVOL;
    output.surfaceRunoffFlow_ROWVOL = results.surfaceRunoffFlow_ROWVOL;
    output.infiltrationFlow_RIVOL = results.infiltrationFlow_RIVOL;
    output.totalArea_FLAECHE = input.totalArea_FLAECHE();
    output.evaporation_VERDUNSTUN = results.evaporation_VERDUNSTUN;

    return 0;
}

void Calculation::writeResultRecord(
    AbimoOutputRecord& record,
    AbimoWriter& writer
) const
{
    writer.addRecord();
    writer.setRecordField("CODE", record.code_CODE);
    writer.setRecordField("R", record.totalRunoff_R);
    writer.setRecordField("ROW", record.surfaceRunoff_ROW);
    writer.setRecordField("RI", record.infiltration_RI);
    writer.setRecordField("RVOL", record.totalRunoffFlow_RVOL);
    writer.setRecordField("ROWVOL", record.surfaceRunoffFlow_ROWVOL);
    writer.setRecordField("RIVOL", record.infiltrationFlow_RIVOL);
    writer.setRecordField("FLAECHE", record.totalArea_FLAECHE);
    writer.setRecordField("VERDUNSTUN", record.evaporation_VERDUNSTUN);
}

int Calculation::progressNumber(int i, int n, float max)
{
    return (int) (static_cast<float>(i) / static_cast<float>(n) * max);
}

void Calculation::stopProcessing()
{
    m_continueProcessing = false;
}

Counters Calculation::getCounters() const
{
    return m_counters;
}

QString Calculation::getError() const
{
    return m_error;
}
