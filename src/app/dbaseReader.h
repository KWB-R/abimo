/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef DBASEREADER_H
#define DBASEREADER_H

#include <QDate>
#include <QFile>
#include <QHash>
#include <QString>

// _fraction indicates numbers between 0 and 1 (instead of percentages)
struct abimoRecord {
    int usage; // old: NUTZUNG
    QString code; // old: CODE
    int precipitationYear; // old: REGENJA
    int precipitationSummer; // old: REGENSO
    float depthToWaterTable; // old: FLUR
    int type; // old: TYP
    int fieldCapacity_30; // old: FELD_30
    int fieldCapacity_150; // old: FELD_150
    int district; // old: BEZIRK

    //
    // degree of sealing (Versiegelungsgrad) of ...
    //

    // ... roof surfaces (Dachflaechen), share of roof area [%] 'PROBAU'
    float mainFractionBuiltSealed; // old: PROBAU_fraction, vgd

    // ... other sealed surfaces (sonst. versiegelte Flaechen)
    float mainFractionUnbuiltSealed; // old: PROVGU_fraction, vgb

    // ... roads (Strassen)
    float roadFractionSealed; // old: VGSTRASSE_fraction, vgs

    //
    // degree of canalization (Kanalisierungsgrad) for ...
    //

    // ... roof surfaces (Dachflaechen)
    float builtSealedFractionConnected; // old: KAN_BEB_fraction, kd

    // ... other sealed surfaces (sonst. versiegelte Flaechen)
    float unbuiltSealedFractionConnected; // old: KAN_VGU_fraction, kb

    // ... sealed roads (Strassen)
    float roadSealedFractionConnected; // old: KAN_STR_fraction, ks

    //
    // share of each pavement class for surfaces except roads of block area
    // Anteil der jeweiligen Belagsklasse
    //

    float unbuiltSealedFractionSurface1; // old: BELAG1_fraction, bl1
    float unbuiltSealedFractionSurface2; // old: BELAG2_fraction, bl2
    float unbuiltSealedFractionSurface3; // old: BELAG3_fraction, bl3
    float unbuiltSealedFractionSurface4; // old: BELAG4_fraction, bl4

    //
    // share of each road pavement class for roads of block area
    // Anteil der jeweiligen Strassenbelagsklasse
    //

    float roadSealedFractionSurface1; // old: STR_BELAG1_fraction, bls1
    float roadSealedFractionSurface2; // old: STR_BELAG2_fraction, bls2
    float roadSealedFractionSurface3; // old: STR_BELAG3_fraction, bls3
    float roadSealedFractionSurface4; // old: STR_BELAG4_fraction, bls4

    // total area within city block, except roads
    float mainArea; // old: FLGES, fb;

    // area of roads within city block
    float roadArea; // old: STR_FLGES, fs;
};

class DbaseReader
{

public:
    DbaseReader(const QString&);
    ~DbaseReader();
    bool read();
    QString getVersion();
    QString getLanguageDriver();
    QDate getDate();
    int getNumberOfRecords();
    int getLengthOfHeader();
    int getLengthOfEachRecord();
    int getCountFields();
    QString getRecord(int num, int field);
    QString getRecord(int num, const QString& name);
    QString getError();
    QString getFullError();
    static QStringList requiredFields();
    bool isAbimoFile();
    bool checkAndRead();
    QString* getValues();
    void fillRecord(int k, abimoRecord& record, bool debug = false);

private:

    // VARIABLES

    QFile file;
    QString version;
    QString languageDriver;
    QDate date;
    QHash<QString, int> hash;
    QString error;
    QString fullError;
    QString* values;

    // count of records in file
    int numberOfRecords;

    // length of the header in byte
    int lengthOfHeader;

    // length of a record in byte
    int lengthOfEachRecord;

    // count of fields
    int countFields;

    // FUNCTIONS

    int expectedFileSize();

    // 1 byte unsigned give the version
    QString checkVersion(quint8, bool debug = true);

    // 1 byte unsigned give the Language Driver (code page)
    QString checkLanguageDriver(quint8 i_byte, bool debug = true);

    // 3 byte unsigned char give the date of last edit
    QDate checkDate(quint8 i_year, quint8 i_month, quint8 i_day);

    // 32 bit unsigned char to int
    int check32(quint8 i1, quint8 i2, quint8 i3, quint8 i4);

    // 16 bit unsigned char to int
    int check16(quint8 i1, quint8 i2);

    // compute the count of fields
    int computeCountFields(int);

    // convert string to float and divide by 100
    float floatFraction(QString string);
};

#endif
