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

#include "abimoRecord.h"
#include "dbaseFile.h"

class DbaseReader : public DbaseFile
{

public:
    DbaseReader(const QString& file);

    // may be overridden by sub-classes
    virtual bool checkAndRead();

    // Function to read the dbf file
    bool read();

    // Function to get one record (row)
    QString getRecord(int num, const QString& name);
    QString getRecord(int num, int field);

// members to which classes that inherit from DbaseReader have access
protected:

    // 1 byte unsigned give the version
    QString byteToVersion(quint8 byte, bool debug = true);

    // 1 byte unsigned give the Language Driver (code page)
    QString byteToLanguageDriver(quint8 byte, bool debug = true);

    // 3 byte unsigned char give the date of last edit
    QDate bytesToDate(quint8 byteYear, quint8 byteMonth, quint8 byteDay);

    // 16 bit unsigned char to int
    int bytesToInteger(quint8 byte1, quint8 byte2);

    // 32 bit unsigned char to int
    int bytesToInteger(quint8 byte1, quint8 byte2, quint8 byte3, quint8 byte4);
};

#endif
