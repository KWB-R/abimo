#ifndef ABIMOREADER_H
#define ABIMOREADER_H

#include <QString>
#include <QStringList>

#include "abimoInputRecord.h"
#include "dbaseReader.h"

class AbimoReader : public DbaseReader
{
public:
    AbimoReader(const QString& file);
    ~AbimoReader();

    static QStringList requiredFields();
    bool isAbimoFile();
    void fillRecord(int k, AbimoInputRecord& record, bool debug = false);
    bool checkAndRead() override;

private:
    // current row index (0 = first row)
    int m_rowIndex;
    // whether to show debug messages when reading/interpreting values
    bool m_debug;

    QString getAsString(const char* fieldName);
    int getAsInteger(const char* fieldName);
    float getAsFloat(const char* fieldName);
    float getAsFloatFraction(const char* fieldName);
    QString rowFieldIndicator(QString& fieldName);
};

#endif // ABIMOREADER_H
