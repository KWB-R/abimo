/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef DBASEWRITER_H
#define DBASEWRITER_H

#include <QByteArray>
#include <QDate>
#include <QString>
#include <QVector>

#include "dbaseField.h"
#include "dbaseFile.h"

class DbaseWriter : public DbaseFile
{
public:
    DbaseWriter(QString& filePath);
    bool write();
    void addRecord();
    void setRecordField(int i, QString& value);
    void setRecordField(int i, float value);

    void setRecordField(const char* name, QString value);
    void setRecordField(const char* name, int value);
    void setRecordField(const char* name, float value);

private:
    int writeFileHeader(QByteArray& data);
    int writeFileHeaderBase(QByteArray& data, int index, DbaseFileHeader& header);
    int writeFileHeaderField(QByteArray& data, int index, DbaseField field);

    void writeFileData(QByteArray& data);
    int writeBytes(QByteArray& data, int index, int value, int n_values);
    int writeThreeByteDate(QByteArray& data, int index, QDate date);
    int writeFourByteInteger(QByteArray& data, int index, int value);
    int writeTwoByteInteger(QByteArray& data, int index, int value);
private:
    // Vector of data rows
    QVector<QVector<QString>> m_record;
};

#endif
