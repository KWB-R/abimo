#ifndef DBASEFFILE_H
#define DBASEFFILE_H

#include <QByteArray>
#include <QDate>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QVector>

#include "dbaseField.h"

struct DbaseFileError
{
    // Error string (short)
    QString textShort;

    // Error string (long)
    QString textLong;
};

struct DbaseFileHeader
{
    // Version of dbf format, see byteToVersion() for a list
    QString version;

    // Date when dbf file was created
    QDate date;

    // Number of records (= data rows)
    int numberOfRecords;

    // Length of file header in bytes
    int headerLength;

    // Length of one record (= data row) in bytes
    int recordLength;

    // Language (code page) used in dbf file, see byteToLanguageDriver()
    QString languageDriver;
};

class DbaseFile
{
public:
    DbaseFile(const QString& filePath);

    // Accessor functions
    DbaseFileHeader getHeader() const;

    const QVector<DbaseField>& getFieldDefinitions() const;
    int getNumberOfFields() const;

    // Length of one data row in bytes
    int calculateRecordLength() const;

    bool hasAllOfTheseFields(QStringList& fieldNames) const;
    int getFieldPosition(const QString& fieldName) const;

    DbaseFileError getError() const;
    //QString getFullError() const;

    static int calculateFileSize(
        int headerLength, int numberOfRecords, int recordLength
    );

    // compute the number of fields in one record (row)
    static int calculateNumberOfFields(int headerLength);
    static int calculateHeaderLength(int numberOfFields);

protected: // i.e. can be accessed by child classes

    // Path to dbf file
    QFile m_file;

    // File header (without field definitions)
    DbaseFileHeader m_header;

    // Vector of field definitions (name, type)
    QVector<DbaseField> m_fields;

    // Hash assigning the 0-based field indices to the field names
    QHash<QString,int> m_fieldPositionMap;

    // Error structure with short and long error text
    DbaseFileError m_error;

    void defineField(DbaseField& field);
    void defineField(QString& name, QString& type, int numDecimalPlaces);
    void defineField(const char* name, const char* type, int numDecimalPlaces);

    void addFieldNameToFieldPositionMap();

private:
    // each field is described by 32 bytes in the file header
    static const int m_bytesPerField = 32;

    // 32 bytes file information
    static const int m_bytesFileInfo = 32;
};

#endif // DBASEFFILE_H
