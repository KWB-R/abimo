/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <QDebug>

#include "saxHandler.h"

SaxHandler::SaxHandler(InitValues &initValues):
    state(ParameterGroup::None),
    initValues(initValues)
{}

bool SaxHandler::startElement(
    const QString &,
    const QString &,
    const QString &qName,
    const QXmlAttributes &attribs
)
{
    if (qName == "section") {
        state = nameToState(attribs.value("name"));
        return (state != ParameterGroup::None && state != ParameterGroup::Invalid);
    }

    if (qName == "item") {

        QString key = attribs.value("key");
        QString value = attribs.value("value");

        switch (state) {

        case ParameterGroup::Infiltrationsfaktoren:
            setInfiltrationFactor(key, value.toFloat());
            break;

        case ParameterGroup::Bagrovwerte :
            setBagrovValue(key, value.toFloat());
            break;

        case ParameterGroup::Nachkomma :
            setDigits(key, value.toInt());
            break;

        case ParameterGroup::Diverse :
            setDiverse(key, value);
            break;

        case ParameterGroup::GewVerd :
            waterEvaporationEntry(attribs);
            break;

        case ParameterGroup::PotVerd :
            potentialEvaporationEntry(attribs);
            break;

        case ParameterGroup::None:
            qDebug() << "state is still 'None'";
            return false;

        case ParameterGroup::Invalid:
            qDebug() << "state is 'Invalid'";
            return false;
        }
    }

    return true;
}

ParameterGroup SaxHandler::nameToState(QString name)
{
    if (name == "Infiltrationsfaktoren") {
        return ParameterGroup::Infiltrationsfaktoren;
    }

    if (name == "Bagrovwerte") {
        return ParameterGroup::Bagrovwerte;
    }

    if (name == "ErgebnisNachkommaStellen") {
        return ParameterGroup::Nachkomma;
    }

    if (name == "Diverse") {
        return ParameterGroup::Diverse;
    }

    if (name == "Gewaesserverdunstung") {
        return ParameterGroup::GewVerd;
    }

    if (name == "PotentielleVerdunstung") {
        return ParameterGroup::PotVerd;
    }

    if (name.isEmpty()) {
        return ParameterGroup::None;
    }

    return ParameterGroup::Invalid;
}

void SaxHandler::setInfiltrationFactor(QString key, float value)
{
    if (key == "Dachflaechen")
        initValues.setInfiltrationFactor(0, value);
    else if (key == "Belaglsklasse1")
        initValues.setInfiltrationFactor(1, value);
    else if (key == "Belaglsklasse2")
        initValues.setInfiltrationFactor(2, value);
    else if (key == "Belaglsklasse3")
        initValues.setInfiltrationFactor(3, value);
    else if (key == "Belaglsklasse4")
        initValues.setInfiltrationFactor(4, value);
}

void SaxHandler::setBagrovValue(QString key, float value)
{
    if (key == "Dachflaechen")
        initValues.setBagrovValue(0, value); // 0 = roof
    else if (key == "Belaglsklasse1")
        initValues.setBagrovValue(1, value);
    else if (key == "Belaglsklasse2")
        initValues.setBagrovValue(2, value);
    else if (key == "Belaglsklasse3")
        initValues.setBagrovValue(3, value);
    else if (key == "Belaglsklasse4")
        initValues.setBagrovValue(4, value);
}

void SaxHandler::setDigits(QString key, int value)
{
    if (key == "R")
        initValues.setDigitsTotalRunoff(value);
    else if (key == "ROW")
        initValues.setDigitsRunoff(value);
    else if (key == "RI")
        initValues.setDigitsInfiltrationRate(value);
    else if (key == "RVOL")
        initValues.setDigitsTotalRunoffFlow(value);
    else if (key == "ROWVOL")
        initValues.setDigitsRainwaterRunoff(value);
    else if (key == "RIVOL")
        initValues.setDigitsTotalSubsurfaceFlow(value);
    else if (key == "FLAECHE")
        initValues.setDigitsTotalArea(value);
    else if (key == "VERDUNSTUNG")
        initValues.setDigitsEvaporation(value);
}

void SaxHandler::setDiverse(QString key, QString value)
{
    if (key == "BERtoZero")
        initValues.setIrrigationToZero(value == "true");
    else if (key == "NIEDKORRF")
        initValues.setPrecipitationCorrectionFactor(value.toFloat());
}

void SaxHandler::waterEvaporationEntry(const QXmlAttributes &attribs)
{
    QString bezirke = attribs.value("bezirke");
    QString eg = attribs.value("eg");

    initValues.putToHashOfType(bezirke, eg.toInt(), 13);
}

void SaxHandler::potentialEvaporationEntry(const QXmlAttributes &attribs)
{
    QString etp = attribs.value("etp");
    QString etps = attribs.value("etps");

    if (etp.length() > 0 || etps.length() > 0) {

        QString bezirke = attribs.value("bezirke");

        if (etp.length() > 0) {
            initValues.putToHashOfType(bezirke, etp.toInt(), 11);
        }

        if (etps.length() > 0) {
            initValues.putToHashOfType(bezirke, etps.toInt(), 12);
        }
    }
}

bool SaxHandler::fatalError(const QXmlParseException & /*exception*/)
{
    return false;
}
