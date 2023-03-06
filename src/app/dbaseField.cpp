/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <QByteArray>
#include <QChar>
#include <QString>

#include "dbaseField.h"

DbaseField::DbaseField():
    fieldLength(0),
    decimalCount(0)
{
}

DbaseField::DbaseField(QByteArray array)
{
    this->name = QString(array.left(10));
    this->type = QString((QChar) array[11]);
    this->fieldLength = (quint8) array[16];
    this->decimalCount = (quint8) array[17];
}

DbaseField::DbaseField(QString name, QString type, int decimalCount)
{
    set(name, type, decimalCount);
}

DbaseField::~DbaseField()
{
}

void DbaseField::set(QString name, QString type, int decimalCount)
{
    this->name = QString(name);
    this->type = QString(type);
    this->fieldLength = 0;
    this->decimalCount = decimalCount;
}

QString DbaseField::getName()
{
    return name;
}

QString DbaseField::getType()
{
    return type;
}

int DbaseField::getFieldLength()
{
    return fieldLength;
}

void DbaseField::setFieldLength(int length)
{
    fieldLength = length;
}

int DbaseField::getDecimalCount()
{
    return decimalCount;
}
