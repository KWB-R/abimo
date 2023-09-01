#include <QString>
#include <QStringList>

#include "dbaseFile.h"
#include "dbaseField.h"
#include "helpers.h"

DbaseFile::DbaseFile(const QString& filePath) :
    m_file(filePath),
    m_header(),
    m_fields(),
    m_fieldPositionMap(),
    m_error()
{
    m_header.date = QDateTime::currentDateTime().date();
}

DbaseFileHeader DbaseFile::getHeader() const
{
    assert(m_header.recordLength == calculateRecordLength());
    return m_header;
}

int DbaseFile::getNumberOfFields() const
{
    return m_fields.size();
}

bool DbaseFile::hasAllOfTheseFields(QStringList& fieldNames) const
{
    return helpers::containsAll(m_fieldPositionMap, fieldNames);
}

int DbaseFile::getFieldPosition(const QString& fieldName) const
{
    return (m_fieldPositionMap.contains(fieldName)) ?
        m_fieldPositionMap[fieldName] :
        -1;
}

DbaseFileError DbaseFile::getError() const
{
    return m_error;
}

int DbaseFile::calculateFileSize(
    int headerLength, int numberOfRecords, int recordLength
)
{
    return headerLength + (numberOfRecords * recordLength) + 1;
}

int DbaseFile::calculateNumberOfFields(int headerLength)
{
    return (headerLength - m_bytesFileInfo - 1) / m_bytesPerField;
}

int DbaseFile::calculateHeaderLength(int numberOfFields)
{
    return numberOfFields * m_bytesPerField + m_bytesFileInfo + 1;
}

void DbaseFile::defineField(DbaseField& field)
{
    // Add field description to vector m_fields
    m_fields.push_back(field);

    // Add the corresponding column number to the hash
    addFieldNameToFieldPositionMap();

    // Update field 'recordLength' in the file header
    m_header.recordLength = calculateRecordLength();
}

void DbaseFile::defineField(QString& name, QString& type, int numDecimalPlaces)
{
    DbaseField field = DbaseField(name, type, numDecimalPlaces);
    defineField(field);
}

void DbaseFile::defineField(const char* name, const char* type, int numDecimalPlaces)
{
    DbaseField field = DbaseField(name, type, numDecimalPlaces);
    defineField(field);
}

void DbaseFile::addFieldNameToFieldPositionMap()
{
    // index of the last field in the vector m_fields
    int index = m_fields.size() - 1;
    m_fieldPositionMap[m_fields.at(index).getName()] = index;
}

// Calculate the length of one data row in bytes (1 byte separator?)
int DbaseFile::calculateRecordLength() const
{
    int length = 1;

    //assert(m_fields.size() > 0);

    for (int i = 0; i < m_fields.size(); i++) {
        length += m_fields[i].getFieldLength();
    }

    return length;
}

const QVector<DbaseField>& DbaseFile::getFieldDefinitions() const
{
    return m_fields;
}
