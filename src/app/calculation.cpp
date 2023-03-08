// ***************************************************************************
// * For copyright information please see COPYRIGHT in the base directory
// * of this repository (https://github.com/KWB-R/abimo).
// ***************************************************************************

#include <vector>

#include <math.h>
#include <QDebug>
#include <QString>
#include <QTextStream>

#include "bagrov.h"
#include "calculation.h"
#include "config.h"
#include "constants.h"
#include "counters.h"
#include "dbaseReader.h"
#include "dbaseWriter.h"
#include "effectivenessunsealed.h"
#include "helpers.h"
#include "initvalues.h"
#include "pdr.h"

Calculation::Calculation(
        DbaseReader &dbaseReader,
        InitValues &initValues,
        QTextStream &protocolStream
):
    m_initValues(initValues),
    m_protocolStream(protocolStream),
    m_dbReader(dbaseReader),
    m_precipitationYear(0), // old: regenja
    m_precipitationSummer(0), // old: regenso
    m_bagrovValues({0, 0, 0, 0, 0}),
    m_unsealedSurfaceRunoff(0), // old: RUV
    m_surfaceRunoffFlow(0), // old: ROWVOL
    m_infiltrationFlow(0), // old: RIVOL
    m_totalRunoffFlow(0), // old: RVOL
    m_potentialCapillaryRise(0), // old: TAS
    m_counters(),
    m_continueProcessing(true) // old: weiter
{
}

void Calculation::stopProcessing()
{
    m_continueProcessing = false;
}

Counters Calculation::getCounters()
{
    return m_counters;
}

QString Calculation::getError()
{
    return m_error;
}

// =============================================================================
// Diese Funktion importiert die Datensaetze aus der DBASE-Datei FileName in das
// DA Feld ein (GWD-Daten).
// Parameter: outputFile: Name der Ausgabedatei
//            debug: whether or not to show debug messages
// Rueckgabewert: BOOL. TRUE, wenn das Einlesen der Datei erfolgreich war.
// =============================================================================
bool Calculation::calculate(QString outputFile, bool debug)
{
    // Current Abimo record (represents one row of the input dbf file)
    abimoRecord record;

    // Number of processed records
    int index = 0;

    // count protocol entries
    m_counters.initialise();

    // first entry into protocol
    DbaseWriter writer(outputFile, m_initValues);

    // get the number of rows in the input data
    int recordCount = m_dbReader.getNumberOfRecords();

    // loop over all block partial areas (= records/rows of input data)
    for (int k = 0; k < recordCount; k++) {

        if (!m_continueProcessing) {
            break;
        }

        m_resultRecord.wIndex = index;

        // Fill record with data from row k
        m_dbReader.fillRecord(k, record, debug);

        // NUTZUNG = integer representing the type of area usage for each block
        // partial area
        if (record.usage != 0) {

            // calculate and set result record fields to calculated values
            calculateResultRecord(record);
            writeResultRecord(record, writer);

            index++;
        }
        else {
            // Hier koennten falls gewuenscht die Flaechen dokumentiert
            // werden, deren NUTZUNG=NULL
            m_counters.incrementNoUsageGiven();
        }

        emit processSignal(progressNumber(k, recordCount, 50), "Berechne");
    }

    // set counters
    m_counters.setRecordsRead(recordCount);
    m_counters.setRecordsWritten(index);

    if (!m_continueProcessing) {
        m_protocolStream << "Berechnungen abgebrochen.\r\n";
        return true;
    }

    emit processSignal(50, "Schreibe Ergebnisse.");

    if (!writer.write()) {
        m_protocolStream << "Error: " + writer.getError() +"\r\n";
        m_error = "Fehler beim Schreiben der Ergebnisse.\n" + writer.getError();
        return false;
    }

    return true;
}

int Calculation::progressNumber(int i, int n, int max)
{
    return (int) (static_cast<float>(i) / static_cast<float>(n * max));
}

void Calculation::calculateResultRecord(abimoRecord &record)
{
    // Verhaeltnis Bebauungsflaeche / Strassenflaeche zu Gesamtflaeche
    // (ant = Anteil)
    // share of building development area / road area to total area
    float areaFractionMain; // old: fbant
    float areaFractionRoad; // old: fsant

    // Abflussvariablen der versiegelten Flaechen
    // runoff variables of sealed surfaces
    // Take care: for consistency use indices 1 to 4 only, do not use 0 (roofs)!
    // old: row1 - row4
    std::vector<float> runoffSealedSurfaces = {0, 0, 0, 0, 0};

    // Infiltrationsvariablen der versiegelten Flaechen
    // infiltration variables of sealed surfaces
    // Take care: for consistency use indices 1 to 4 only, do not use 0 (roofs)!
    // old: ri1 - ri4
    std::vector<float> infiltrationSealedSurfaces = {0, 0, 0, 0, 0};

    // Abfluss- / Infiltrationsvariablen der Dachflaechen
    // runoff- / infiltration variables of roof surfaces
    float runoffRoofs; // old: rowd
    float infiltrationRoofs; // old: rid

    // Abfluss- / Infiltrationsvariablen unversiegelter Strassenflaechen
    // runoff- / infiltration variables of unsealed road surfaces
    float runoffPerviousRoads; // old: rowuvs
    float infiltrationPerviousRoads; // old: riuvs

    // Infiltration unversiegelter Flaechen
    // infiltration of unsealed areas
    float infiltrationPerviousSurfaces; // old: riuv

    // precipitation for entire year and for summer season only
    m_precipitationYear = record.precipitationYear;
    m_precipitationSummer = record.precipitationSummer;

    // depth to groundwater table 'FLUR'
    m_resultRecord.depthToWaterTable = record.depthToWaterTable;

    // declaration of yield power (ERT) and irrigation (BER) for agricultural or
    // gardening purposes
    UsageResult usageResult = m_config.getUsageResult(
        record.usage, record.type, record.code
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

    m_resultRecord.setUsageYieldIrrigation(
        m_config.getUsageTuple(usageResult.tupleIndex)
    );

    if (m_resultRecord.usage != Usage::waterbody_G)
    {
        // Feldkapazitaet
        m_resultRecord.usableFieldCapacity = PDR::estimateWaterHoldingCapacity(
            record.fieldCapacity_30,
            record.fieldCapacity_150,
            m_resultRecord.usage == Usage::forested_W
        );

        // mittl. Durchwurzelungstiefe TWS
        float rootingDepth = m_config.getRootingDepth(
            m_resultRecord.usage,
            m_resultRecord.yieldPower
        );

        // pot. Aufstiegshoehe TAS = FLUR - mittl. Durchwurzelungstiefe TWS
        m_potentialCapillaryRise = m_resultRecord.depthToWaterTable - rootingDepth;

        // mittlere pot. kapillare Aufstiegsrate kr (mm/d) des Sommerhalbjahres
        m_resultRecord.meanPotentialCapillaryRiseRate =
            PDR::getMeanPotentialCapillaryRiseRate(
                m_potentialCapillaryRise,
                m_resultRecord.usableFieldCapacity,
                m_resultRecord.usage,
                m_resultRecord.yieldPower
            );
    }

    if (m_initValues.getIrrigationToZero() && m_resultRecord.irrigation != 0) {
        //*protokollStream << "Erzwinge BER=0 fuer Code: " << code << ", Wert war:" << ptrDA.BER << " \r\n";
        m_counters.incrementIrrigationForcedToZero();
        m_resultRecord.irrigation = 0;
    }

    // Bagrov-calculation for sealed surfaces
    getClimaticConditions(record.district, record.code);

    // percentage of total sealed area
    // share of roof area [%] 'PROBAU' +
    // share of other (unbuilt) sealed areas (e.g. Hofflaechen)
    m_resultRecord.mainPercentageSealed = helpers::roundToInteger(
        record.mainFractionBuiltSealed * 100 +
        record.mainFractionUnbuiltSealed * 100
    );

    // if sum of total building development area and road area is
    // inconsiderably small it is assumed, that the area is unknown and
    // 100 % building development area will be given by default
    if (record.mainArea + record.roadArea < 0.0001) {
        // *protokollStream << "\r\nDie Flaeche des Elements " +
        // record.CODE + " ist 0 \r\nund wird automatisch auf 100 gesetzt\r\n";
        m_counters.incrementRecordsProtocol();
        m_counters.incrementNoAreaGiven();
        record.mainArea = 100.0F;
    }

    m_totalArea = record.mainArea + record.roadArea;

    // Verhaeltnis Bebauungsflaeche zu Gesamtflaeche
    // ratio of building development area to total area
    areaFractionMain = record.mainArea / m_totalArea;

    // Verhaeltnis Strassenflaeche zu Gesamtflaeche
    // ratio of roads area to total area
    areaFractionRoad = record.roadArea / m_totalArea;

    // Runoff for sealed surfaces

    //  Legende der Abflussberechnung der 4 Belagsklassen bzw. Dachklasse:
    //    rowd / rowx: Abfluss Dachflaeche / Abfluss Belagsflaeche x
    //    infdach / infbelx: Infiltrationsparameter Dachfl. / Belagsfl. x
    //    belx: Anteil Belagsklasse x
    //    blsx: Anteil Strassenbelagsklasse x
    //    vgd / vgb: Anteil versiegelte Dachfl. / sonstige versiegelte Flaeche zu Gesamtblockteilflaeche
    //    kd / kb / ks: Grad der Kanalisierung Dach / sonst. vers. Fl. / Strassenflaechen
    //    fbant / fsant: ?
    //    RDV / RxV: Gesamtabfluss versiegelte Flaeche

    runoffRoofs =
            (1.0F - m_initValues.getInfiltrationFactor(0)) * // 0 = roof!
            record.mainFractionBuiltSealed *
            record.builtSealedFractionConnected *
            areaFractionMain *
            m_bagrovValues[0]; // 0 = roof!

    for (int i = 1; i < static_cast<int>(runoffSealedSurfaces.size()); i++) {

        runoffSealedSurfaces[i] =
            (1.0F - m_initValues.getInfiltrationFactor(i)) *
            (
                record.unbuiltSealedFractionSurface.at(i) *
                record.unbuiltSealedFractionConnected *
                record.mainFractionUnbuiltSealed *
                areaFractionMain +
                record.roadSealedFractionSurface.at(i) *
                record.roadSealedFractionConnected *
                record.roadFractionSealed *
                areaFractionRoad
            ) * m_bagrovValues[i];
    }

    // Infiltration for sealed surfaces
    infiltrationRoofs =
        (1 - record.builtSealedFractionConnected) *
        record.mainFractionBuiltSealed *
        areaFractionMain *
        m_bagrovValues[0]; // 0 = roof

    for (int i = 1; i < static_cast<int>(infiltrationSealedSurfaces.size()); i++) {

        infiltrationSealedSurfaces[i] = (
            record.unbuiltSealedFractionSurface.at(i) *
            record.mainFractionUnbuiltSealed *
            areaFractionMain +
            record.roadSealedFractionSurface.at(i) *
            record.roadFractionSealed *
            areaFractionRoad
        ) * m_bagrovValues[i] - runoffSealedSurfaces[i];
    }

    // consider unsealed road surfaces as pavement class 4
    // old: 0.11F * (1-vgs) * fsant * R4V;
    runoffPerviousRoads = 0.0F;

    // old: 0.89F * (1-vgs) * fsant * R4V;
    infiltrationPerviousRoads =
            (1 - record.roadFractionSealed) *
            areaFractionRoad *
            m_bagrovValues[4];

    // runoff for unsealed surfaces rowuv = 0
    infiltrationPerviousSurfaces = (
        100.0F - static_cast<float>(m_resultRecord.mainPercentageSealed)
    ) / 100.0F * m_unsealedSurfaceRunoff;

    // calculate runoff 'row' for entire block patial area (FLGES +
    // STR_FLGES) (mm/a)
    m_surfaceRunoff = (
        helpers::vectorSum(runoffSealedSurfaces) +
        runoffRoofs +
        runoffPerviousRoads
    );

    m_resultRecord.runoff = helpers::roundToInteger(m_surfaceRunoff);

    // calculate volume 'rowvol' from runoff (qcm/s)
    m_surfaceRunoffFlow = m_surfaceRunoff * 3.171F * (
        record.mainArea +
        record.roadArea
    ) / 100000.0F;

    // calculate infiltration rate 'ri' for entire block partial area
    // (mm/a)
    m_infiltration = (
        helpers::vectorSum(infiltrationSealedSurfaces) +
        infiltrationRoofs +
        infiltrationPerviousRoads +
        infiltrationPerviousSurfaces
    );

    m_resultRecord.infiltrationRate = helpers::roundToInteger(m_infiltration);

    // calculate volume 'rivol' from infiltration rate (qcm/s)
    m_infiltrationFlow = m_infiltration * 3.171F * (
        record.mainArea +
        record.roadArea
    ) / 100000.0F;

    // calculate total system losses 'r' due to runoff and infiltration
    // for entire block partial area
    m_totalRunoff = m_surfaceRunoff + m_infiltration;

    // set totalRunoff in the result record
    //resultRecord.totalRunoff = INT_ROUND(totalRunoff);

    // calculate volume of system losses 'rvol' due to runoff and
    // infiltration
    m_totalRunoffFlow = m_surfaceRunoffFlow + m_infiltrationFlow;

    // calculate evaporation 'verdunst' by subtracting 'r', the sum of
    // runoff and infiltration from precipitation of entire year,
    // multiplied by precipitation correction factor
    m_evaporation = (
        m_precipitationYear *
        m_initValues.getPrecipitationCorrectionFactor()
    ) - m_totalRunoff;

}

// =============================================================================
// This function uses...
// This function modifies...
// =============================================================================
void Calculation::getClimaticConditions(int district, QString code)
{
    // Potential evaporation
    float potentialEvaporation;

    // Prepcipitation at ground level
    float precipitation;

    // ratio of precipitation to potential evaporation
    float xRatio;

    // Later on two additional parameters, (now and?) here:
    // * ptrDA.P1 = p1;
    // * ptrDA.PS = ps;
    m_resultRecord.precipitationYear = m_precipitationYear;
    m_resultRecord.precipitationSummer = m_precipitationSummer;

    // Parameter for the city districts
    if (m_resultRecord.usage == Usage::waterbody_G)
    {
        m_resultRecord.longtimeMeanPotentialEvaporation = initValueOrReportedDefaultValue(
            district, code, m_initValues.hashEG, 775, "EG"
        );
    }
    else
    {
        m_resultRecord.longtimeMeanPotentialEvaporation = initValueOrReportedDefaultValue(
            district, code, m_initValues.hashETP, 660, "ETP"
        );

        m_resultRecord.potentialEvaporationSummer = initValueOrReportedDefaultValue(
            district, code, m_initValues.hashETPS, 530, "ETPS"
        );
    }

    // Declaration of potential evaporation and precipitation
    potentialEvaporation = static_cast<float>(m_resultRecord.longtimeMeanPotentialEvaporation); // no more correction with 1.1
    precipitation = static_cast<float>(m_resultRecord.precipitationYear * m_initValues.getPrecipitationCorrectionFactor());

    // Berechnung der Abfluesse RDV und R1V bis R4V fuer versiegelte
    // Teilflaechen und unterschiedliche Bagrovwerte ND und N1 bis N4

    // ratio precipitation to potential evaporation
    xRatio = precipitation / potentialEvaporation;

    // Berechnung des Abflusses RxV fuer versiegelte Teilflaechen mittels
    // Umrechnung potentieller Verdunstungen potentialEvaporation zu realen
    // ueber Umrechnungsfaktor yRatio und subtrahiert von Niederschlag
    // precipitation
    // index 0 = roof, indices 1 - 4 = surface classes 1 - 4

    for (int i = 0; i < static_cast<int>(m_bagrovValues.size()); i++) {

        m_bagrovValues[i] = precipitation -
            Bagrov::nbagro(m_initValues.getBagrovValue(i), xRatio) *
            potentialEvaporation;
    }

    // Calculate runoff RUV for unsealed surfaces
    float actualEvaporation = (m_resultRecord.usage == Usage::waterbody_G) ?
        potentialEvaporation :
        realEvapotranspiration(potentialEvaporation, precipitation);

    m_unsealedSurfaceRunoff = precipitation - actualEvaporation;
}

float Calculation::realEvapotranspiration(
    float potentialEvaporation,
    float precipitation
)
{
    assert(potentialEvaporation > 0.0);

    // Effectivity parameter
    float effectivityParameter;

    // ratio of real evaporation to potential evaporation
    float yRatio;

    // result value: real evapotranspiration
    float realEvapotranspiration;

    // Determine effectiveness parameter bag for unsealed surfaces
    // Modul Raster abgespeckt
    effectivityParameter = EffectivenessUnsealed::getEffectivityParameter(
                m_resultRecord.usableFieldCapacity,
                m_resultRecord.usage == Usage::forested_W,
                m_resultRecord.yieldPower,
                m_resultRecord.irrigation,
                m_resultRecord.precipitationSummer,
                m_resultRecord.potentialEvaporationSummer,
                m_resultRecord.meanPotentialCapillaryRiseRate
    );

    // Calculate the x-factor of bagrov relation: x = (P + KR + BER)/ETP
    // Then get the y-factor: y = fbag(n, x)
    yRatio = Bagrov::nbagro(
        effectivityParameter,
        (
            precipitation +
            m_resultRecord.meanPotentialCapillaryRiseRate +
            m_resultRecord.irrigation
        ) / potentialEvaporation
    );

    // Get the real evapotransporation using estimated y-factor
    realEvapotranspiration = yRatio * potentialEvaporation;

    if (m_potentialCapillaryRise < 0) {
        realEvapotranspiration += (
            potentialEvaporation - yRatio * potentialEvaporation
        ) * static_cast<float>(
            exp(m_resultRecord.depthToWaterTable / m_potentialCapillaryRise)
        );
    }

    return realEvapotranspiration;
}

float Calculation::initValueOrReportedDefaultValue(
        int district,
        QString code,
        QHash<int, int> &hash,
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

    QString string;
    string.setNum(result);

    m_protocolStream << "\r\n" + name + " unbekannt fuer " + code +
        " von Bezirk " + districtString + "\r\n" + name +
        "=" + string + " angenommen\r\n";

    m_counters.incrementRecordsProtocol();

    return result;
}

void Calculation::writeResultRecord(abimoRecord &record, DbaseWriter &writer)
{
    writer.addRecord();
    writer.setRecordField("CODE", record.code);
    writer.setRecordField("R", m_totalRunoff); // local
    writer.setRecordField("ROW", m_surfaceRunoff); // local
    writer.setRecordField("RI", m_infiltration); // local
    writer.setRecordField("RVOL", m_totalRunoffFlow); // member of Calculate
    writer.setRecordField("ROWVOL", m_surfaceRunoffFlow); // member of Calculate
    writer.setRecordField("RIVOL", m_infiltrationFlow); // member of Calculate
    writer.setRecordField("FLAECHE", m_totalArea); // local
    writer.setRecordField("VERDUNSTUN", m_evaporation); // local
}

void Calculation::runCalculation(
        QString inputFile,
        QString configFile,
        QString outputFile,
        bool debug
)
{
    // Open the input file
    DbaseReader dbReader(inputFile);

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
