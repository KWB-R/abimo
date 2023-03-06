// ***************************************************************************
// * For copyright information please see COPYRIGHT in the base directory
// * of this repository (https://github.com/KWB-R/abimo).
// ***************************************************************************

#include <math.h>
#include <QDebug>
#include <QString>
#include <QTextStream>

#include "bagrov.h"
#include "calculation.h"
#include "config.h"
#include "constants.h"
#include "dbaseReader.h"
#include "dbaseWriter.h"
#include "effectivenessunsealed.h"
#include "helpers.h"
#include "initvalues.h"
#include "pdr.h"

// Macro to calculate the number of elements in an array
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

// Potential rate of ascent (column labels for matrix
// meanPotentialCapillaryRiseRateSummer)
// old: iTAS
const float Calculation::POTENTIAL_RATES_OF_ASCENT[] = {
    0.1F, 0.2F, 0.3F, 0.4F, 0.5F, 0.6F, 0.7F, 0.8F,
    0.9F, 1.0F, 1.2F, 1.4F, 1.7F, 2.0F, 2.3F
};

// soil type unknown - default soil type used in the following: sand

// Usable field capacity (row labels for matrix
// meanPotentialCapillaryRiseRateSummer)
// old: inFK_S
const float Calculation::USABLE_FIELD_CAPACITIES[] = {
    8.0F, 9.0F, 14.0F, 14.5F, 15.5F, 17.0F, 20.5F
};

// Mean potential capillary rise rate kr [mm/d] of a summer season depending on:
// - Potential rate of ascent (one column each) and
// - Usable field capacity (one row each)
// old: ijkr_S
const float Calculation::MEAN_POTENTIAL_CAPILLARY_RISE_RATES_SUMMER[] = {
    7.0F, 6.0F, 5.0F, 1.5F, 0.5F, 0.2F, 0.1F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 5.0F, 3.0F, 1.2F, 0.5F, 0.2F, 0.1F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 3.0F, 1.5F, 0.7F, 0.3F, 0.15F, 0.1F, 0.0F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 3.0F, 2.0F, 1.0F, 0.7F, 0.4F, 0.15F, 0.1F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 4.5F, 2.5F, 1.5F, 0.7F, 0.4F, 0.15F, 0.1F, 0.0F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 5.0F, 5.0F, 3.5F, 2.0F, 1.5F, 0.8F, 0.3F, 0.1F, 0.05F, 0.0F, 0.0F,
    7.0F, 7.0F, 6.0F, 6.0F, 6.0F, 5.0F, 5.0F, 5.0F, 3.0F, 2.0F, 1.0F, 0.5F, 0.15F, 0.0F, 0.0F
};

Calculation::Calculation(
        DbaseReader &dbaseReader,
        InitValues &initValues,
        QTextStream &protocolStream
):
    initValues(initValues),
    protocolStream(protocolStream),
    dbReader(dbaseReader),
    precipitationYear(0), // old: regenja
    precipitationSummer(0), // old: regenso
    bagrovValueRoof(0), // old: RDV
    bagrovValueSurface1(0), // old: R1V
    bagrovValueSurface2(0), // old: R2V
    bagrovValueSurface3(0), // old: R3V
    bagrovValueSurface4(0), // old: R4V
    unsealedSurfaceRunoff(0), // old: RUV
    surfaceRunoffFlow(0), // old: ROWVOL
    infiltrationFlow(0), // old: RIVOL
    totalRunoffFlow(0), // old: RVOL
    potentialCapillaryRise(0), // old: TAS
    n_POTENTIAL_RATES_OF_ASCENT(ARRAY_SIZE(POTENTIAL_RATES_OF_ASCENT)), // old: lenTAS
    n_USABLE_FIELD_CAPACITIES(ARRAY_SIZE(USABLE_FIELD_CAPACITIES)), // old: lenS
    counters({0, 0, 0, 0L, 0L, 0L}),
    continueProcessing(true) // old: weiter
{
    config = new Config();
}

void Calculation::stopProcessing()
{
    continueProcessing = false;
}

Counters Calculation::getCounters()
{
    return counters;
}

QString Calculation::getError()
{
    return error;
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
    
    // Verhaeltnis Bebauungsflaeche / Strassenflaeche zu Gesamtflaeche
    // (ant = Anteil)
    // share of building development area / road area to total area
    float areaFractionMain; // old: fbant
    float areaFractionRoad; // old: fsant
    
    // Abflussvariablen der versiegelten Flaechen
    // runoff variables of sealed surfaces
    float runoffSealedSurface1; // old: row1
    float runoffSealedSurface2; // old: row2
    float runoffSealedSurface3; // old: row3
    float runoffSealedSurface4; // old: row4
    
    // Infiltrationsvariablen der versiegelten Flaechen
    // infiltration variables of sealed surfaces
    float infiltrationSealedSurface1; // old: ri1
    float infiltrationSealedSurface2; // old: ri2
    float infiltrationSealedSurface3; // old: ri3
    float infiltrationSealedSurface4; // old: ri4
    
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
    
    // intermediate float values
    float totalRunoff;
    float infiltration;
    float surfaceRunoff;

    // count protocol entries
    counters.recordsProtocol = 0L;
    counters.noAreaGiven = 0L;
    counters.noUsageGiven = 0L;

    // first entry into protocol
    DbaseWriter writer(outputFile, initValues);

    // get the number of rows in the input data
    counters.recordsRead = dbReader.getNumberOfRecords();

    // loop over all block partial areas (= records/rows of input data)
    for (int k = 0; k < counters.recordsRead; k++) {

        if (!continueProcessing) {
            protocolStream << "Berechnungen abgebrochen.\r\n";
            return true;
        }

        resultRecord.wIndex = index;

        // Fill record with data from row k
        dbReader.fillRecord(k, record, debug);

        // NUTZUNG = integer representing the type of area usage for each block
        // partial area
        if (record.usage != 0) {

            // CODE: unique identifier for each block partial area

            // precipitation for entire year and for summer season only
            precipitationYear = record.precipitationYear;
            precipitationSummer = record.precipitationSummer;

            // depth to groundwater table 'FLUR'
            resultRecord.depthToWaterTable = record.depthToWaterTable;

            getUsage(
                record.usage,
                // structure type
                record.type,
                // field capacity [%] for 0-30cm below ground level
                record.fieldCapacity_30,
                // field capacity [%] for 0-150cm below ground level
                record.fieldCapacity_150,
                record.code
            );

            // cls_6a: an dieser Stelle muss garantiert werden, dass f30 und
            // f150 als Parameter von getNUTZ einen definierten Wert erhalten
            // und zwar 0.
            // FIXED: alle Werte sind definiert... wenn keine 0, sondern nichts
            // bzw. Leerzeichen angegeben wurden, wird nun eine 0 eingesetzt
            // aber eigentlich war das auch schon so ... ???

            // Bagrov-calculation for sealed surfaces
            getClimaticConditions(record.district, record.code);

            // percentage of total sealed area
            resultRecord.mainPercentageSealed = INT_ROUND(
                // share of roof area [%] 'PROBAU'
                record.mainFractionBuiltSealed * 100 +
                // share of other (unbuilt) sealed areas (e.g. Hofflaechen)
                record.mainFractionUnbuiltSealed * 100
            );

            // if sum of total building development area and road area is
            // inconsiderably small it is assumed, that the area is unknown and
            // 100 % building development area will be given by default
            if (record.mainArea + record.roadArea < 0.0001) {
                // *protokollStream << "\r\nDie Flaeche des Elements " +
                // record.CODE + " ist 0 \r\nund wird automatisch auf 100 gesetzt\r\n";
                counters.recordsProtocol++;
                counters.noAreaGiven++;
                record.mainArea = 100.0F;
            }

            float totalArea = record.mainArea + record.roadArea;

            // Verhaeltnis Bebauungsflaeche zu Gesamtflaeche
            // ratio of building development area to total area
            areaFractionMain = record.mainArea / totalArea;
            
            // Verhaeltnis Strassenflaeche zu Gesamtflaeche
            // ratio of roads area to total area
            areaFractionRoad = record.roadArea / totalArea;

            // Runoff for sealed surfaces
            // cls_1: Fehler a:
            //   rowd = (1.0F - initValues.getInfdach()) * vgd * kb * fbant * RDV;
            //   richtige Zeile folgt (kb ----> kd)
            
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
                (1.0F - initValues.getInfiltrationFactorRoof()) *
                record.mainFractionBuiltSealed *
                record.builtSealedFractionConnected *
                areaFractionMain *
                bagrovValueRoof;

            runoffSealedSurface1 =
                (1.0F - initValues.getInfiltrationFactorSurface1()) *
                (
                    record.unbuiltSealedFractionSurface1 *
                    record.unbuiltSealedFractionConnected *
                    record.mainFractionUnbuiltSealed *
                    areaFractionMain +
                    record.roadSealedFractionSurface1 *
                    record.roadSealedFractionConnected *
                    record.roadFractionSealed *
                    areaFractionRoad
                ) * bagrovValueSurface1;

            runoffSealedSurface2 =
                (1.0F - initValues.getInfiltrationFactorSurface2()) *
                (
                    record.unbuiltSealedFractionSurface2 *
                    record.unbuiltSealedFractionConnected *
                    record.mainFractionUnbuiltSealed *
                    areaFractionMain +
                    record.roadSealedFractionSurface2 *
                    record.roadSealedFractionConnected *
                    record.roadFractionSealed *
                    areaFractionRoad
                ) * bagrovValueSurface2;

            runoffSealedSurface3 =
                (1.0F - initValues.getInfiltrationFactorSurface3()) *
                (
                    record.unbuiltSealedFractionSurface3 *
                    record.unbuiltSealedFractionConnected *
                    record.mainFractionUnbuiltSealed *
                    areaFractionMain +
                    record.roadSealedFractionSurface3 *
                    record.roadSealedFractionConnected *
                    record.roadFractionSealed *
                    areaFractionRoad
                ) * bagrovValueSurface3;

            runoffSealedSurface4 =
                (1.0F - initValues.getInfiltrationFactorSurface4()) *
                (
                    record.unbuiltSealedFractionSurface4 *
                    record.unbuiltSealedFractionConnected *
                    record.mainFractionUnbuiltSealed *
                    areaFractionMain +
                    record.roadSealedFractionSurface4 *
                    record.roadSealedFractionConnected *
                    record.roadFractionSealed *
                    areaFractionRoad
                ) * bagrovValueSurface4;

            // Infiltration for sealed surfaces
            infiltrationRoofs =
                (1 - record.builtSealedFractionConnected) *
                record.mainFractionBuiltSealed *
                areaFractionMain *
                bagrovValueRoof;

            infiltrationSealedSurface1 = (
                record.unbuiltSealedFractionSurface1 *
                record.mainFractionUnbuiltSealed *
                areaFractionMain +
                record.roadSealedFractionSurface1 *
                record.roadFractionSealed *
                areaFractionRoad
            ) * bagrovValueSurface1 - runoffSealedSurface1;

            infiltrationSealedSurface2 = (
                record.unbuiltSealedFractionSurface2 *
                record.mainFractionUnbuiltSealed *
                areaFractionMain +
                record.roadSealedFractionSurface2 *
                record.roadFractionSealed *
                areaFractionRoad
            ) * bagrovValueSurface2 - runoffSealedSurface2;

            infiltrationSealedSurface3 = (
                record.unbuiltSealedFractionSurface3 *
                record.mainFractionUnbuiltSealed *
                areaFractionMain +
                record.roadSealedFractionSurface3 *
                record.roadFractionSealed *
                areaFractionRoad
            ) * bagrovValueSurface3 - runoffSealedSurface3;

            infiltrationSealedSurface4 = (
                record.unbuiltSealedFractionSurface4 *
                record.mainFractionUnbuiltSealed *
                areaFractionMain +
                record.roadSealedFractionSurface4 *
                record.roadFractionSealed *
                areaFractionRoad
            ) * bagrovValueSurface4 - runoffSealedSurface4;
            
            // consider unsealed road surfaces as pavement class 4
            // old: 0.11F * (1-vgs) * fsant * R4V;
            runoffPerviousRoads = 0.0F;

            // old: 0.89F * (1-vgs) * fsant * R4V;
            infiltrationPerviousRoads =
                (1 - record.roadFractionSealed) *
                areaFractionRoad *
                bagrovValueSurface4;

            // runoff for unsealed surfaces rowuv = 0
            infiltrationPerviousSurfaces = (
                100.0F - (float) resultRecord.mainPercentageSealed
            ) / 100.0F * unsealedSurfaceRunoff;

            // calculate runoff 'row' for entire block patial area (FLGES +
            // STR_FLGES) (mm/a)
            surfaceRunoff = (
                runoffSealedSurface1 +
                runoffSealedSurface2 +
                runoffSealedSurface3 +
                runoffSealedSurface4 +
                runoffRoofs +
                runoffPerviousRoads
            );

            resultRecord.runoff = INT_ROUND(surfaceRunoff);
            
            // calculate volume 'rowvol' from runoff (qcm/s)
            surfaceRunoffFlow = surfaceRunoff * 3.171F * (
                record.mainArea +
                record.roadArea
            ) / 100000.0F;
            
            // calculate infiltration rate 'ri' for entire block partial area
            // (mm/a)
            infiltration = (
                infiltrationSealedSurface1 +
                infiltrationSealedSurface2 +
                infiltrationSealedSurface3 +
                infiltrationSealedSurface4 +
                infiltrationRoofs +
                infiltrationPerviousRoads +
                infiltrationPerviousSurfaces
            );

            resultRecord.infiltrationRate = INT_ROUND(infiltration);
            
            // calculate volume 'rivol' from infiltration rate (qcm/s)
            infiltrationFlow = infiltration * 3.171F * (
                record.mainArea +
                record.roadArea
            ) / 100000.0F;

            // calculate total system losses 'r' due to runoff and infiltration
            // for entire block partial area
            totalRunoff = surfaceRunoff + infiltration;

            // set totalRunoff in the result record
            resultRecord.totalRunoff = INT_ROUND(totalRunoff);
            
            // calculate volume of system losses 'rvol' due to runoff and
            // infiltration
            totalRunoffFlow = surfaceRunoffFlow + infiltrationFlow;

// cls_5b:
            // calculate evaporation 'verdunst' by subtracting 'r', the sum of
            // runoff and infiltration from precipitation of entire year,
            // multiplied by precipitation correction factor
            float evaporation = (
                precipitationYear *
                initValues.getPrecipitationCorrectionFactor()
            ) - totalRunoff;

            // write the calculated variables into respective fields
            writer.addRecord();
            writer.setRecordField("CODE", record.code);
            writer.setRecordField("R", totalRunoff);
            writer.setRecordField("ROW", surfaceRunoff);
            writer.setRecordField("RI", infiltration);
            writer.setRecordField("RVOL", totalRunoffFlow);
            writer.setRecordField("ROWVOL", surfaceRunoffFlow);
            writer.setRecordField("RIVOL", infiltrationFlow);
            writer.setRecordField("FLAECHE", totalArea);
// cls_5c:
            writer.setRecordField("VERDUNSTUN", evaporation);

            index++;
        }
        else {
            counters.noUsageGiven++;
        }

        // cls_2: Hier koennten falls gewuenscht die Flaechen dokumentiert
        // werden, deren NUTZUNG=NULL (siehe auch cls_3)

        emit processSignal(
            (int)((float) k / (float) counters.recordsRead * 50.0),
            "Berechne"
        );
    }

    counters.recordsWritten = index;

    emit processSignal(50, "Schreibe Ergebnisse.");

    if (!writer.write()) {
        protocolStream << "Error: "+ writer.getError() +"\r\n";
        error = "Fehler beim Schreiben der Ergebnisse.\n" + writer.getError();
        return false;
    }

    return true;
}

// =============================================================================
// FIXME:
// =============================================================================
void Calculation::getUsage(
        int usageID,
        int type,
        int fieldCapacity_30,
        int fieldCapacity_150,
        QString code
)
{
    // Feldlaengen von iTAS und inFK_S, L, T, U
    // extern int lenTAS, lenS, lenL, lenT, lenU;

    // declaration of yield power (ERT) and irrigation (BER) for agricultural or
    // gardening purposes
    setUsageYieldIrrigation(usageID, type, code);

    if (resultRecord.usage != Usage::waterbody_G)
    {
        // Feldkapazitaet
        // cls_6b: der Fall der mit NULL belegten FELD_30 und FELD_150 Werte
        // wird hier im ersten Fall behandelt - ich erwarte dann den Wert 0
        resultRecord.usableFieldCapacity = PDR::estimateWaterHoldingCapacity(
                    fieldCapacity_30,
                    fieldCapacity_150,
                    resultRecord.usage == Usage::forested_W
        );

        // mittl. Durchwurzelungstiefe TWS
        float rootingDepth = config->getRootingDepth(
                    resultRecord.usage,
                    resultRecord.yieldPower
        );

        // pot. Aufstiegshoehe TAS = FLUR - mittl. Durchwurzelungstiefe TWS
        potentialCapillaryRise = resultRecord.depthToWaterTable - rootingDepth;

        // mittlere pot. kapillare Aufstiegsrate kr (mm/d) des Sommerhalbjahres
        //
        // switch (bod) {
        //   case S: case U: case L: case T: case LO: case HN:
        // }
        //
        // wird eingefuegt, wenn die Bodenart in das Zahlenmaterial aufgenommen
        // wird. Vorlaeufig wird Sande angenommen.

        float kr = (potentialCapillaryRise <= 0.0) ?
            7.0F :
            MEAN_POTENTIAL_CAPILLARY_RISE_RATES_SUMMER[
                Helpers::index(
                    potentialCapillaryRise,
                    POTENTIAL_RATES_OF_ASCENT,
                    n_POTENTIAL_RATES_OF_ASCENT
                ) +
                Helpers::index(
                    resultRecord.usableFieldCapacity,
                    USABLE_FIELD_CAPACITIES,
                    n_USABLE_FIELD_CAPACITIES
                ) * n_POTENTIAL_RATES_OF_ASCENT
            ];

        int daysOfGrowth = PDR::estimateDaysOfGrowth(
                    resultRecord.usage,
                    resultRecord.yieldPower
        );

        // mittlere pot. kapillare Aufstiegsrate kr (mm/d) des Sommerhalbjahres
        resultRecord.meanPotentialCapillaryRiseRate = (int)(daysOfGrowth * kr);
    }

    if (initValues.getIrrigationToZero() && resultRecord.irrigation != 0) {
        //*protokollStream << "Erzwinge BER=0 fuer Code: " << code << ", Wert war:" << ptrDA.BER << " \r\n";
        counters.irrigationForcedToZero++;
        resultRecord.irrigation = 0;
    }
}

void Calculation::setUsageYieldIrrigation(int usageID, int type, QString code)
{
    UsageResult result;

    result = config->getUsageResult(usageID, type, code);

    if (result.tupleIndex < 0) {
        protocolStream << result.message;
        qDebug() << result.message;
        abort();
    }

    if (!result.message.isEmpty()) {
        protocolStream << result.message;
        counters.recordsProtocol++;
    }

    resultRecord.setUsageYieldIrrigation(config->getUsageTuple(result.tupleIndex));
}

// =============================================================================
// FIXME:
// =============================================================================
void Calculation::getClimaticConditions(int district, QString code)
{
    // Effectivity parameter
    float effectivityParameter;

    // Potential evaporation
    float potentialEvaporation;

    // Prepcipitation at ground level
    float precipitation;

    // ratio of precipitation to potential evaporation
    float xRatio;

    // ratio of real evaporation to potential evaporation
    float yRatio;

    // real evapotranspiration
    float realEvapotranspiration;

    // Later on two additional parameters, (now and?) here:
    // * ptrDA.P1 = p1;
    // * ptrDA.PS = ps;
    resultRecord.precipitationYear = precipitationYear;
    resultRecord.precipitationSummer = precipitationSummer;

    // Parameter for the city districts
    if (resultRecord.usage == Usage::waterbody_G)
    {
        resultRecord.longtimeMeanPotentialEvaporation = initValueOrReportedDefaultValue(
            district, code, initValues.hashEG, 775, "EG"
        );
    }
    else
    {
        resultRecord.longtimeMeanPotentialEvaporation = initValueOrReportedDefaultValue(
            district, code, initValues.hashETP, 660, "ETP"
        );

        resultRecord.potentialEvaporationSummer = initValueOrReportedDefaultValue(
            district, code, initValues.hashETPS, 530, "ETPS"
        );
    }

    // Declaration of potential evaporation and precipitation
    potentialEvaporation = (float) resultRecord.longtimeMeanPotentialEvaporation; // no more correction with 1.1
    precipitation = (float) resultRecord.precipitationYear * initValues.getPrecipitationCorrectionFactor();
      // ptrDA.KF

    // Berechnung der Abfluesse RDV und R1V bis R4V fuer versiegelte
    // Teilflaechen und unterschiedliche Bagrovwerte ND und N1 bis N4

    // ratio precipitation to potential evaporation
    xRatio = precipitation / potentialEvaporation;

    Bagrov bagrov;

    // Berechnung des Abflusses RxV fuer versiegelte Teilflaechen mittels
    // Umrechnung potentieller Verdunstungen potentialEvaporation zu realen
    // ueber Umrechnungsfaktor yRatio und subtrahiert von Niederschlag
    // precipitation

    bagrovValueRoof = precipitation - bagrov.nbagro(initValues.getBagrovValueRoof(), xRatio) * potentialEvaporation;

    bagrovValueSurface1 = precipitation - bagrov.nbagro(initValues.getBagrovValueSuface1(), xRatio) * potentialEvaporation;
    bagrovValueSurface2 = precipitation - bagrov.nbagro(initValues.getBagrovValueSuface2(), xRatio) * potentialEvaporation;
    bagrovValueSurface3 = precipitation - bagrov.nbagro(initValues.getBagrovValueSuface3(), xRatio) * potentialEvaporation;
    bagrovValueSurface4 = precipitation - bagrov.nbagro(initValues.getBagrovValueSuface4(), xRatio) * potentialEvaporation;

    // Calculate runoff RUV for unsealed partial surfaces
    if (resultRecord.usage == Usage::waterbody_G)
    {
        unsealedSurfaceRunoff = precipitation - potentialEvaporation;
    }
    else
    {
        // Determine effectiveness parameter bag for unsealed surfaces
        // Modul Raster abgespeckt
        effectivityParameter = EffectivenessUnsealed::calculate(resultRecord);

        if (resultRecord.precipitationSummer > 0 &&
                resultRecord.potentialEvaporationSummer > 0) {
            effectivityParameter *= getSummerModificationFactor(
                (float) (
                    resultRecord.precipitationSummer +
                    resultRecord.irrigation +
                    resultRecord.meanPotentialCapillaryRiseRate
                ) / resultRecord.potentialEvaporationSummer
            );
        }

        // Calculate the x-factor of bagrov relation: x = (P + KR + BER)/ETP
        // Then get the y-factor: y = fbag(n, x)
        yRatio = bagrov.nbagro(
            effectivityParameter,
            (
                precipitation +
                resultRecord.meanPotentialCapillaryRiseRate +
                resultRecord.irrigation
            ) / potentialEvaporation
        );

        // Get the real evapotransporation using estimated y-factor
        realEvapotranspiration = yRatio * potentialEvaporation;

        if (potentialCapillaryRise < 0) {
            realEvapotranspiration += (
                potentialEvaporation - yRatio * potentialEvaporation
            ) * (float) exp(
                resultRecord.depthToWaterTable / potentialCapillaryRise
            );
        }

        unsealedSurfaceRunoff = precipitation - realEvapotranspiration;
    }
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

    protocolStream << "\r\n" + name + " unbekannt fuer " + code +
        " von Bezirk " + districtString + "\r\n" + name +
        "=" + string + " angenommen\r\n";
    counters.recordsProtocol++;

    return result;
}

// =============================================================================
// Get factor to be applied for "summer"
// =============================================================================
float Calculation::getSummerModificationFactor(float wa)
{
    const float VALUES_WA[] =
    {
        0.45F, 0.50F, 0.55F, 0.60F, 0.65F, 0.70F, 0.75F, // 0 ..  6
        0.80F, 0.85F, 0.90F, 0.95F, 1.00F, 1.05F, 1.10F  // 7 .. 13
    };

    const float VALUES_F[] =
    {
        0.65F, 0.75F, 0.82F, 0.90F, 1.00F, 1.06F, 1.15F, // 0 ..  6
        1.22F, 1.30F, 1.38F, 1.47F, 1.55F, 1.63F, 1.70F  // 7 .. 13
    };

    return Helpers::interpolate(wa, VALUES_WA, VALUES_F, 14);
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

    QFile logHandle(Helpers::defaultLogFileName(outputFile));

    Helpers::openFileOrAbort(logHandle, QFile::WriteOnly);

    QTextStream logStream(&logHandle);

    Calculation calculator(dbReader, initValues, logStream);

    bool success = calculator.calculate(outputFile, debug);

    if (!success) {
        qDebug() << "Error in calculate(): " << calculator.getError();
        abort();
    }

    logHandle.close();
}
