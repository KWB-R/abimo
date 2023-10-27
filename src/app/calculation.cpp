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
#include <QVector>

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

void Calculation::runCalculationUsingData(
    QString inputFile,
    QString outputFile
)
{
    // Input data structure: vector of objects of class AbimoInputRecord
    QVector<AbimoInputRecord> inputData;

    // Output data structure: vector of objects of class AbimoOutputRecord
    QVector<AbimoOutputRecord> outputData;

    // Read dbf file into input data structure
    inputData = readAbimoInputData(inputFile);

    // Do the calculations for the input data creating output data
    calculateData(inputData, outputData);

    InitValues initValues;

    // Write the model results to the output dbf file
    writeAbimoOutputData(outputData, initValues, outputFile);
}

QVector<AbimoInputRecord> Calculation::readAbimoInputData(QString inputFile)
{
    // Open the input file
    AbimoReader dbReader(inputFile);

    // Try to read the raw (text) values into the dbReader object
    if (!dbReader.checkAndRead()) {
        abort();
    }

    QVector<AbimoInputRecord> inputData;
    AbimoInputRecord inputRecord;

    // Create input data: fill vector of AbimoInputRecord
    for (int i = 0; i < dbReader.getHeader().numberOfRecords; i++) {
        dbReader.fillRecord(i, inputRecord, false);
        inputData.append(inputRecord);
    }

    return inputData;
}

void Calculation::writeAbimoOutputData(
    QVector<AbimoOutputRecord>& outputData,
    InitValues& initValues,
    QString outputFile
)
{
    // Provide an AbimoWriter object
    AbimoWriter writer(outputFile, initValues);

    AbimoOutputRecord outputRecord;

    for (int i = 0; i < outputData.size(); i++) {
        outputRecord = outputData.at(i);
        writeResultRecord(outputRecord, writer);
    }

    // Write output data to dbf file
    writer.write();
}

int calculateData(
    QVector<AbimoInputRecord>& inputData,
    QVector<AbimoOutputRecord>& outputData
)
{
    // Current Abimo input record
    AbimoInputRecord inputRecord;

    // Current Abimo output record
    AbimoOutputRecord outputRecord;

    // Structure holding intermediate results
    IntermediateResults results;

    InitValues initValues;
    UsageConfiguration usageConfiguration;
    QTextStream protocolStream(stdout);
    Counters counters;

    // loop over all block partial areas (= records/rows of input data)
    for (int i = 0; i < inputData.size(); i++) {

        inputRecord = inputData.at(i);

        // usage = integer representing the type of area usage for the current
        // block partial area
        if (inputRecord.usage == 0) {
            continue;
        }

        // Calculate and set result record fields to calculated values
        Calculation::doCalculationsFor(
            inputRecord,
            results,
            initValues,
            usageConfiguration,
            counters,
            protocolStream
        );

        Calculation::fillResultRecord(inputRecord, results, outputRecord);

        // Set the corresponding row in the result data structure
        outputData.append(outputRecord);
    }

    // Return an error code (currently not set)
    return 0;
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
            doCalculationsFor(
                inputRecord,
                results,
                m_initValues,
                m_usageMappings,
                m_counters,
                m_protocolStream
            );

            // Write all results to the log file
            logResults(
                inputRecord,
                results,
                // Should Bagrov intermediates be logged?
                inputRecord.code == "1000536281000000"
            );

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
    IntermediateResults& intermediates,
    InitValues& initValues,
    UsageConfiguration& usageConfiguration,
    Counters& counters,
    QTextStream& protocolStream
)
{
    // Provide information on the precipitation
    Precipitation precipitation = getPrecipitation(
        input.precipitationYear,
        input.precipitationSummer,
        initValues.getPrecipitationCorrectionFactor()
    );

    // Based on the given input row, try to provide usage-specific information
    UsageTuple usageTuple = getUsageTuple(
        input.usage,
        input.type,
        usageConfiguration,
        initValues.getIrrigationToZero(),
        counters,
        protocolStream,
        input.code
    );

    // Provide information on the potential evaporation
    PotentialEvaporation potentialEvaporation = getPotentialEvaporation(
        initValues,
        usageTuple.usage == Usage::waterbody_G,
        input.district,
        input.code,
        counters,
        protocolStream
    );

    // Set default area if total area is zero
    handleTotalAreaOfZero(input, counters);

    intermediates.potentialEvaporation = potentialEvaporation;
    intermediates.usageTuple = usageTuple;
    intermediates.precipitation = precipitation;
    intermediates.mainArea = input.mainArea;

    //
    // Do the Bagrov-calculation for sealed surfaces...
    //

    Runoff runoff;

    // Berechnung der Abfluesse RDV und R1V bis R4V fuer versiegelte
    // Teilflaechen und unterschiedliche Bagrovwerte ND und N1 bis N4.
    // Die reale Verdunstung wird vom Niederschlag subtrahiert.
    // - RDV / RxV: Gesamtabfluss versiegelte Flaeche

    // index 0 = roof

    // Theoretical total runoff assuming that the whole area is a roof
    intermediates.runoffSealed.roof = precipitation.perYearCorrectedFloat -
        Bagrov::realEvapoTranspiration(
            precipitation.perYearCorrectedFloat,
            potentialEvaporation.perYearFloat,
            initValues.getBagrovValue(0),
            intermediates.bagrovIntermediates
        );

    // Actual runoff from roof surfaces (Abfluss der Dachflaechen), old: rowd
    runoff.roof =
        initValues.getRunoffFactor(0) * // 0 = roof!
        input.mainFractionBuiltSealed *
        input.builtSealedFractionConnected *
        input.areaFractionMain() *
        intermediates.runoffSealed.roof;

    // runoff from sealed surfaces

    // indices 1 - 4 = surface classes 1 - 4
    for (int i = 0; i < static_cast<int>(intermediates.runoffSealed.surface.size()); i++) {

        // Theoretical total runoff assuming that the whole area is sealed and
        // connected
        intermediates.runoffSealed.surface[i] = precipitation.perYearCorrectedFloat -
            Bagrov::realEvapoTranspiration(
                precipitation.perYearCorrectedFloat,
                potentialEvaporation.perYearFloat,
                initValues.getBagrovValue(i + 1),
                intermediates.bagrovIntermediates
            );

        // Runoff from the actual partial areas that are sealed and connected
        // (road and non-road) areas
        // Abfluss Belagsflaeche i + 1, old: row<i>
        runoff.sealedSurface[i] =
            initValues.getRunoffFactor(i + 1) *
            (
                input.unbuiltSealedFractionSurface.at(i + 1) *
                input.unbuiltSealedFractionConnected *
                input.mainFractionUnbuiltSealed *
                input.areaFractionMain() +
                input.roadSealedFractionSurface.at(i + 1) *
                input.roadSealedFractionConnected *
                input.roadFractionSealed *
                input.areaFractionRoad()
            ) * intermediates.runoffSealed.surface[i];
    }

    // runoff from unsealed surfaces

    // Provide soil properties. They are required to calculate tha actual
    // evapotranspiration. In the case of water bodies, all values are 0.0.
    intermediates.soilProperties = getSoilProperties(
        usageTuple.usage,
        usageTuple.yield,
        input.depthToWaterTable,
        input.fieldCapacity_30,
        input.fieldCapacity_150
    );

    runoff.unsealedSurface_RUV =
        precipitation.perYearCorrectedFloat -
        actualEvaporation(
            usageTuple,
            potentialEvaporation,
            intermediates.soilProperties,
            precipitation
        );

    // Calculate infiltration...
    // =========================

    //Infiltration infiltration;

    // infiltration from roof (?)
    intermediates.infiltration.roof =
        (1 - input.builtSealedFractionConnected) *
        input.mainFractionBuiltSealed *
        input.areaFractionMain() *
        intermediates.runoffSealed.roof;

    // infiltration from sealed surfaces
    for (int i = 0; i < static_cast<int>(intermediates.infiltration.surface.size()); i++) {

        intermediates.infiltration.surface[i] = (
            input.unbuiltSealedFractionSurface.at(i + 1) *
            input.mainFractionUnbuiltSealed *
            input.areaFractionMain() +
            input.roadSealedFractionSurface.at(i + 1) *
            input.roadFractionSealed *
            input.areaFractionRoad()
        ) * intermediates.runoffSealed.surface[i] -
        runoff.sealedSurface[i];
    }

    // infiltration from unsealed road surfaces
    intermediates.infiltration.unsealedRoads =
        (1 - input.roadFractionSealed) *
        input.areaFractionRoad() *
        intermediates.runoffSealed.surface.last();

    // infiltration from unsealed non-road surfaces
    // old: riuv
    intermediates.infiltration.unsealedSurfaces = (
        100.0F - input.mainPercentageSealed()
    ) / 100.0F * runoff.unsealedSurface_RUV;

    // Set infiltration-related fields in output record
    //=================================================

    // calculate infiltration rate 'ri' for entire block partial area
    // (mm/a)
    intermediates.infiltration_RI = (
        intermediates.infiltration.roof +
        helpers::vectorSum(intermediates.infiltration.surface) +
        intermediates.infiltration.unsealedRoads +
        intermediates.infiltration.unsealedSurfaces
    );

    // Set runoff-related fields in output record
    //===========================================

    // calculate runoff 'ROW' for entire block patial area (FLGES +
    // STR_FLGES) (mm/a)
    intermediates.surfaceRunoff_ROW = (
        runoff.roof +
        helpers::vectorSum(runoff.sealedSurface) +
        runoff.unsealedRoads
    );

    // calculate total system losses 'r' due to runoff and infiltration
    // for entire block partial area
    intermediates.totalRunoff_R =
        intermediates.surfaceRunoff_ROW +
        intermediates.infiltration_RI;

    // Convert yearly heights to flows
    //================================

    // calculate volume 'rowvol' from runoff (qcm/s)
    intermediates.surfaceRunoffFlow_ROWVOL = input.yearlyHeightToVolumeFlow(
        intermediates.surfaceRunoff_ROW
    );

    // calculate volume 'rivol' from infiltration rate (qcm/s)
    intermediates.infiltrationFlow_RIVOL = input.yearlyHeightToVolumeFlow(
        intermediates.infiltration_RI
    );

    // calculate volume of "system losses" 'rvol' due to surface runoff and
    // infiltration
    intermediates.totalRunoffFlow_RVOL =
        intermediates.surfaceRunoffFlow_ROWVOL +
        intermediates.infiltrationFlow_RIVOL;

    // Set evaporation in output record
    //=================================

    // calculate evaporation 'VERDUNST' by subtracting 'R', the sum of
    // runoff and infiltration from precipitation of entire year,
    // multiplied by precipitation correction factor
    intermediates.evaporation_VERDUNSTUN =
        precipitation.perYearCorrectedFloat -
        intermediates.totalRunoff_R;
}

Precipitation Calculation::getPrecipitation(
    int precipitationYear,
    int precipitationSummer,
    float correctionFactor
)
{
    Precipitation result;

    // Set integer fields (originally from input dbf)
    result.perYearInteger = precipitationYear;
    result.inSummerInteger = precipitationSummer;

    // Set float fields

    // Correct the (non-summer) precipitation (at ground level)
    result.perYearCorrectedFloat = static_cast<float>(
        precipitationYear * correctionFactor
    );

    // No correction for summer precipitation!
    result.inSummerFloat = static_cast<float>(
        precipitationSummer
    );

    return result;
}

UsageTuple Calculation::getUsageTuple(
    int usage,
    int type,
    UsageConfiguration& usageConfiguration,
    bool overrideIrrigationWithZero,
    Counters& counters,
    QTextStream& protocolStream,
    QString code // just for information in protocolStream
)
{
    // declaration of yield power (ERT) and irrigation (BER) for agricultural or
    // gardening purposes
    UsageResult usageResult = usageConfiguration.getUsageResult(
        usage,
        type,
        code
    );

    if (usageResult.tupleIndex < 0) {
        protocolStream << usageResult.message;
        qDebug() << usageResult.message;
        abort();
    }

    if (!usageResult.message.isEmpty()) {
        protocolStream << usageResult.message;
        counters.incrementRecordsProtocol();
    }

    UsageTuple result = usageConfiguration.getUsageTuple(usageResult.tupleIndex);

    // Override irrigation value with zero if the corresponding option is set
    if (overrideIrrigationWithZero && result.irrigation != 0) {
        //*protokollStream << "Erzwinge BER=0 fuer Code: " << code <<
        //", Wert war:" << usageTuple.irrigation << " \r\n";
        counters.incrementIrrigationForcedToZero();
        result.irrigation = 0;
    }

    return result;
}

PotentialEvaporation Calculation::getPotentialEvaporation(
    InitValues& initValues,
    bool isWaterbody,
    int district,
    QString code,
    Counters& counters,
    QTextStream& protocolStream
)
{
    PotentialEvaporation result;

    // Parameter for the city districts
    if (isWaterbody) {

        result.perYearInteger = getInitialValueOrDefault(
            district,
            code,
            initValues.hashEG,
            775,
            "EG",
            counters,
            protocolStream
        );

        // What about potentialEvaporationSummer?
        result.inSummerInteger = -1;
    }
    else {
        result.perYearInteger = getInitialValueOrDefault(
            district,
            code,
            initValues.hashETP,
            660,
            "ETP",
            counters,
            protocolStream
        );

        result.inSummerInteger = getInitialValueOrDefault(
            district,
            code,
            initValues.hashETPS,
            530,
            "ETPS",
            counters,
            protocolStream
        );
    }

    // no more correction with 1.1
    result.perYearFloat = static_cast<float>(result.perYearInteger);

    return result;
}

float Calculation::getInitialValueOrDefault(
        int district,
        QString code,
        QHash<int,int> &hash,
        int defaultValue,
        QString name,
        Counters& counters,
        QTextStream& protocolStream
)
{
    // Take value from hash table (as read from xml file) if available
    if (hash.contains(district)) {
        return hash.value(district);
    }

    // Default
    float result = hash.contains(0) ? hash.value(0) : defaultValue;

    protocolStream << name << " unknown for " << code <<
        " (district: " << district << ") -> " <<
        name << "=" << result << " assumed." << endl;

    counters.incrementRecordsProtocol();

    return result;
}

void Calculation::handleTotalAreaOfZero(
    AbimoInputRecord& input,
    Counters& counters
)
{
    // if sum of total building development area and road area is
    // inconsiderably small it is assumed, that the area is unknown and
    // 100 % building development area will be given by default
    if (input.totalArea_FLAECHE() < 0.0001) {
        // *protokollStream << "\r\nDie Flaeche des Elements " +
        // record.CODE + " ist 0 \r\nund wird automatisch auf 100 gesetzt\r\n";
        counters.incrementRecordsProtocol();
        counters.incrementNoAreaGiven();
        input.mainArea = 100.0F;
    }
}

SoilProperties Calculation::getSoilProperties(
    Usage usage,
    int yield,
    float depthToWaterTable,
    int fieldCapacity_30,
    int fieldCapacity_150
)
{
    // Initialise variables that are relevant to calculate evaporation
    SoilProperties result;

    result.depthToWaterTable = depthToWaterTable;

    // Nothing to do for waterbodies
    if (usage == Usage::waterbody_G) {
        return result;
    }

    // Feldkapazitaet
    result.usableFieldCapacity = SoilAndVegetation::estimateWaterHoldingCapacity(
        fieldCapacity_30,
        fieldCapacity_150,
        usage == Usage::forested_W
    );

    // pot. Aufstiegshoehe TAS = FLUR - mittl. Durchwurzelungstiefe TWS
    // potentielle Aufstiegshoehe
    result.potentialCapillaryRise_TAS = result.depthToWaterTable -
        SoilAndVegetation::getRootingDepth(
            usage,
            yield
        );

    // mittlere pot. kapillare Aufstiegsrate kr (mm/d) des Sommerhalbjahres
    // Kapillarer Aufstieg pro Jahr ID_KR neu, old: KR
    result.meanPotentialCapillaryRiseRate =
        SoilAndVegetation::getMeanPotentialCapillaryRiseRate(
            result.potentialCapillaryRise_TAS,
            result.usableFieldCapacity,
            usage,
            yield
        );

    return result;
}

float Calculation::actualEvaporation(
    UsageTuple& usageTuple,
    PotentialEvaporation& potentialEvaporation,
    SoilProperties& evaporationVars,
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
    float effectivity = EffectivenessUnsealed::getEffectivityParameter(
        usageTuple,
        evaporationVars.usableFieldCapacity,
        precipitation.inSummerFloat,
        potentialEvaporation.inSummerInteger,
        evaporationVars.meanPotentialCapillaryRiseRate
    );

    // Use the Bagrov relation with xRatio = (P + KR + BER)/ETP to calculate
    // the real evapotranspiration    
    BagrovIntermediates throwAway;

    float result = Bagrov::realEvapoTranspiration(
        precipitation.perYearCorrectedFloat +
            evaporationVars.meanPotentialCapillaryRiseRate +
            usageTuple.irrigation,
        potentialEvaporation.perYearFloat,
        effectivity,
        throwAway
    );

    float tas = evaporationVars.potentialCapillaryRise_TAS;

    if (tas < 0) {
        float factor = exp(evaporationVars.depthToWaterTable / tas);
        result += (potentialEvaporation.perYearFloat - result) * factor;
    }

    return result;
}

void Calculation::logResults(
        AbimoInputRecord& inputRecord,
        IntermediateResults& results,
        bool logBagrovIntermediates
)
{
    m_protocolStream << endl << "*** Code: " << inputRecord.code << endl;

    //m_prefix = inputRecord.code;

    if (logBagrovIntermediates) {
        logVariable("bagrov_i", results.bagrovIntermediates.i);
        logVariable("bagrov_j", results.bagrovIntermediates.j);
        logVariable("bagrov_a", results.bagrovIntermediates.a);
        logVariable("bagrov_a0", results.bagrovIntermediates.a0);
        logVariable("bagrov_a1", results.bagrovIntermediates.a1);
        logVariable("bagrov_a2", results.bagrovIntermediates.a2);
        logVariable("bagrov_b", results.bagrovIntermediates.b);
        logVariable("bagrov_bag", results.bagrovIntermediates.bag);
        logVariable("bagrov_bag_plus_one", results.bagrovIntermediates.bag_plus_one);
        logVariable("bagrov_c", results.bagrovIntermediates.c);
        logVariable("bagrov_epa", results.bagrovIntermediates.epa);
        logVariable("bagrov_eyn", results.bagrovIntermediates.eyn);
        logVariable("bagrov_h", results.bagrovIntermediates.h);
        logVariable("bagrov_h13", results.bagrovIntermediates.h13);
        logVariable("bagrov_h23", results.bagrovIntermediates.h23);
        logVariable("bagrov_reciprocal_bag_plus_one", results.bagrovIntermediates.reciprocal_bag_plus_one);
        logVariable("bagrov_sum_1", results.bagrovIntermediates.sum_1);
        logVariable("bagrov_sum_2", results.bagrovIntermediates.sum_2);
        logVariable("bagrov_w", results.bagrovIntermediates.w);
        logVariable("bagrov_x", results.bagrovIntermediates.x);
        logVariable("bagrov_y0", results.bagrovIntermediates.y0);
    }

    Precipitation pr = results.precipitation;
    PotentialEvaporation pe = results.potentialEvaporation;
    UsageTuple ut = results.usageTuple;
    RunoffSealed rs = results.runoffSealed;
    SoilProperties sp = results.soilProperties;
    Infiltration inf = results.infiltration;

    logVariable("pr.perYearCorrectedFloat", pr.perYearCorrectedFloat);
    logVariable("pr.inSummerFloat", pr.inSummerFloat);

    logVariable("pe.perYearFloat", pe.perYearFloat);
    logVariable("pe.inSummerInteger", pe.inSummerInteger);

    logVariable("ut.usage", (char) ut.usage);
    logVariable("ut.yield", ut.yield);
    logVariable("ut.irrigation", ut.irrigation);

    logVariable("in.mainArea", results.mainArea);

    logVariable("runoffSealed.roof", rs.roof);

    for (int i = 0; i < rs.surface.size(); i++) {
        logVariable(QString("rs.surface[%1]").arg(i), rs.surface[i]);
    }

    logVariable("sp.depthToWaterTable", sp.depthToWaterTable);
    logVariable("sp.usableFieldCapacity", sp.usableFieldCapacity);
    logVariable("sp.potentialCapillaryRise_TAS", sp.potentialCapillaryRise_TAS);
    logVariable("sp.meanPotentialCapillaryRiseRate", sp.meanPotentialCapillaryRiseRate);

    logVariable("infiltration.roof", inf.roof);
    logVariable("infiltration.unsealedRoads", inf.unsealedRoads);
    logVariable("infiltration.unsealedSurfaces", inf.unsealedSurfaces);

    for (int i = 0; i < inf.surface.size(); i++) {
        logVariable(QString("infiltration.surface[%1]").arg(i), inf.surface[i]);
    }

    logVariable("surfaceRunoff_ROW", results.surfaceRunoff_ROW);
    logVariable("surfaceRunoffFlow_ROWVOL", results.surfaceRunoffFlow_ROWVOL);
    logVariable("infiltration_RI", results.infiltration_RI);
    logVariable("infiltrationFlow_RIVOL", results.infiltrationFlow_RIVOL);
    logVariable("totalRunoff_R", results.totalRunoff_R);
    logVariable("totalRunoffFlow_RVOL", results.totalRunoffFlow_RVOL);
    logVariable("evaporation_VERDUNSTUN", results.evaporation_VERDUNSTUN);
}

void Calculation::logVariable(QString name, int value)
{
    m_protocolStream << name << "=" << value << endl;
}

void Calculation::logVariable(QString name, float value)
{
    //m_protocolStream << m_prefix << ";" <<
    m_protocolStream << name << "=" << QString::number(value, 'g', 10) << endl;
}

void Calculation::logVariable(QString name, char value)
{
    m_protocolStream << name << "=" << value << endl;
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
) {
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
