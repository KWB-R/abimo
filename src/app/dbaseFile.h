#ifndef DBASEFFILE_H
#define DBASEFFILE_H

#include <QDate>
#include <QFile>
#include <QString>
#include <QVector>

class DbaseFile
{
public:
    DbaseFile(const QString& filePath);

    // Accessor functions
    QString getVersion() const;
    QString getLanguageDriver() const;
    QDate getDate() const;

    int getHeaderLength() const;
    int getRecordLength() const;
    int getNumberOfRecords() const;
    int getNumberOfFields() const;

    QVector<QString> getValues() const;

    QString getError() const;
    QString getFullError() const;

    int expectedFileSize() const;

    // compute the number of fields in one record (row)
    static int calculateNumberOfFields(int headerLength);
    static int calculateHeaderLength(int numberOfFields);

private:
    // each field is described by 32 bytes in the file header
    static const int m_bytesPerField = 32;

    // 32 bytes file information
    static const int m_bytesFileInfo = 32;

protected:

    // Path to dbf file
    QFile m_file;

    // Version of dbf format, see byteToVersion() for a list
    QString m_version;

    // Language (code page) used in dbf file, see byteToLanguageDriver()
    QString m_languageDriver;

    // Date when dbf file was created
    QDate m_date;

    // length of the header in bytes
    int m_headerLength;

    // length of a record in bytes
    int m_recordLength;

    // number of records (data rows) in file
    int m_numberOfRecords;

    // number of fields in a record (data row)
    int m_numberofFields;

    // Hash assigning the 0-based field indices to the field names
    QHash<QString,int> m_fieldPositionMap;

    // String values representing the data content of the dbf file
    QVector<QString> m_stringValues;

    // Error string (short)
    QString m_error;

    // Error string (long)
    QString m_fullError;
};

#endif // DBASEFFILE_H
