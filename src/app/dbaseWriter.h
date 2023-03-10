/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef DBASEWRITER_H
#define DBASEWRITER_H

#include <QByteArray>
#include <QDate>
#include <QHash>
#include <QString>
#include <QVector>

#include "dbaseField.h"
#include "dbaseFile.h"
#include "initValues.h"

class DbaseWriter : public DbaseFile
{
public:
    DbaseWriter(const QString& filePath, const InitValues& initValues);
    bool write();
    void addRecord();
    void setRecordField(int i, const QString& value);
    void setRecordField(int i, float value);
    void setRecordField(const QString& name, const QString& value);
    void setRecordField(const QString& name, float value);
private:
    QVector<QVector<QString>> record;
    QVector<DbaseField> fields;
    int writeFileHeader(QByteArray& data);
    void writeFileData(QByteArray& data);
    int writeBytes(QByteArray& data, int index, int value, int n_values);
    int writeThreeByteDate(QByteArray& data, int index, QDate date);
    int writeFourByteInteger(QByteArray& data, int index, int value);
    int writeTwoByteInteger(QByteArray& data, int index, int value);
};

#endif
