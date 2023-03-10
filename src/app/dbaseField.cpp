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

QString DbaseField::getName()
{
    return m_name;
}

QString DbaseField::getType()
{
    return m_type;
}

int DbaseField::getFieldLength()
{
    return m_fieldLength;
}

void DbaseField::setFieldLength(int fieldLength)
{
    m_fieldLength = fieldLength;
}

int DbaseField::getDecimalCount()
{
    return m_numDecimalPlaces;
}

QString DbaseField::formatNumericString(QString s, int length, int decimalPlaces)
{
    // https://www.dbase.com/Knowledgebase/INT/db7_file_fmt.htm
    // F - Float: Number stored as a string, right justified, and padded with
    //     blanks to the width of the field.
    // N - Numeric: Number stored as a string, right justified, and padded with
    //     blanks to the width of the field.

    const QChar blank = QChar(0x30); // should be QChar(0x20)!

    // s = "123.456", length = 5, decimalPlaces = 1
    // left = "123", right = "456"
    // leftLength = 3 (length - 1 - decimalPlaces - ((isNegative) ? 1 : 0))
    // left.rightJustified(leftLength, blank) = "123"
    // right.leftJustified(decimalPlaces, blank) = "456" and not "4"!

    if (decimalPlaces <= 0) {
        return s.rightJustified(length, blank, true);
    }

    // The rest of the function handles numbers with decimal places

    // Provide the substrings left and right of the decimal character (".")
    QStringList parts = s.split(".");

    assert(parts.length() == 2);

    QString left = parts.at(0);
    QString right = parts.at(1);

    bool isNegative = left.contains('-');

    // this looks useless...
    //if (isNegative) {
    //    left = QString("-") + left.right(left.length() - 1);
    //}

    int leftLength = length - 1 - decimalPlaces - ((isNegative) ? 1 : 0);

    QString result = QString("%1.%2").arg(
        left.rightJustified(leftLength, blank, true),
        right.leftJustified(decimalPlaces, blank, true)
    );

    // fails!
    // assert(result.length() == length);

    //qDebug() << QString("formatNumericString('%1', %2, %3) = '%4'").arg(
    //    s, QString::number(length), QString::number(decimalPlaces), result
    //);

    return result;
}
