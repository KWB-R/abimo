/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#ifndef SAXHANDLER_H
#define SAXHANDLER_H

#include <QXmlDefaultHandler>
#include <QXmlAttributes>
#include <QXmlParseException>

#include "initValues.h"

enum struct ParameterGroup {
    None = 0,
    Infiltrationsfaktoren = 1,
    Bagrovwerte = 2,
    Nachkomma = 3,
    Diverse = 4,
    GewVerd = 5,
    PotVerd = 6,
    Invalid = 99
};

// SAX = Simple API for XML

class SaxHandler : public QXmlDefaultHandler
{

public:
    SaxHandler(InitValues &initValues);
    bool startElement(
        const QString &namespaceURI,
        const QString &localName,
        const QString &qName,
        const QXmlAttributes &attribs
    );
    bool fatalError(const QXmlParseException &exception);

private:
    ParameterGroup m_parameterGroup;
    InitValues &initValues;
    void potentialEvaporationEntry(
        const QXmlAttributes &attribs, QString name, int index
    );
    static ParameterGroup nameToParameterGroup(QString name);
    static OutputColumn nameToOutputColumn(QString name);

    int surfaceNameToIndex(QString name);
};

#endif
