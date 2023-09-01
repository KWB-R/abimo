/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef DBASEFIELD_H
#define DBASEFIELD_H

#include <QByteArray>
#include <QString>

class DbaseField
{

public:
    DbaseField();
    DbaseField(QByteArray);
    DbaseField(QString name, QString type, int numDecimalPlaces);
    QString getName() const;
    QString getType() const;
    int getFieldLength() const;
    int getDecimalCount() const;
    void setFieldLength(int fieldLength);
    void set(QString name, QString type, int numDecimalPlaces);

private:
    QString m_name;
    QString m_type;
    int m_fieldLength;
    int m_numDecimalPlaces;
};

#endif
