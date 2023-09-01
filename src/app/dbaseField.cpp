/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <QByteArray>
#include <QChar>
#include <QDebug>
#include <QString>
#include <QStringList>

#include "dbaseField.h"
#include "helpers.h"

DbaseField::DbaseField():
    m_fieldLength(0),
    m_numDecimalPlaces(0)
{
}

DbaseField::DbaseField(QByteArray bytes)
{
    m_name = QString(bytes.left(10));
    m_type = QString((QChar) bytes.at(11));
    m_fieldLength = (quint8) bytes.at(16);
    m_numDecimalPlaces = (quint8) bytes.at(17);
}

DbaseField::DbaseField(QString name, QString type, int numDecimalPlaces)
{
    set(name, type, numDecimalPlaces);
}

void DbaseField::set(QString name, QString type, int numDecimalPlaces)
{
    m_name = name;
    m_type = type;
    m_fieldLength = 0;
    m_numDecimalPlaces = numDecimalPlaces;
}

QString DbaseField::getName() const
{
    return m_name;
}

QString DbaseField::getType() const
{
    return m_type;
}

int DbaseField::getFieldLength() const
{
    return m_fieldLength;
}

void DbaseField::setFieldLength(int fieldLength)
{
    m_fieldLength = fieldLength;
}

int DbaseField::getDecimalCount() const
{
    return m_numDecimalPlaces;
}
