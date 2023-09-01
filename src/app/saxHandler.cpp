/***************************************************************************
 * For copyright information please see COPYRIGHT in the base directory
 * of this repository (https://github.com/KWB-R/abimo).
 ***************************************************************************/

#include <QDebug>

#include "saxHandler.h"

SaxHandler::SaxHandler(InitValues &initValues):
    m_parameterGroup(ParameterGroup::None),
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

        m_parameterGroup = nameToParameterGroup(attribs.value("name"));

        return (
            m_parameterGroup != ParameterGroup::None &&
            m_parameterGroup != ParameterGroup::Invalid
        );
    }

    if (qName == "item") {

        QString key = attribs.value("key");
        QString value = attribs.value("value");

        switch (m_parameterGroup) {

        case ParameterGroup::Infiltrationsfaktoren:

            initValues.setInfiltrationFactor(
                surfaceNameToIndex(key),
                value.toFloat()
            );
            break;

        case ParameterGroup::Bagrovwerte :

            initValues.setBagrovValue(
                surfaceNameToIndex(key),
                value.toFloat()
            );
            break;

        case ParameterGroup::Nachkomma :

            initValues.setResultDigits(
                nameToOutputColumn(key),
                value.toInt()
            );
            break;

        case ParameterGroup::Diverse :

            if (key == "BERtoZero") {
                initValues.setIrrigationToZero(value == "true");
            }
            else if (key == "NIEDKORRF") {
                initValues.setPrecipitationCorrectionFactor(value.toFloat());
            }
            break;

        case ParameterGroup::GewVerd :

            potentialEvaporationEntry(attribs, "eg", 13);
            break;

        case ParameterGroup::PotVerd :

            potentialEvaporationEntry(attribs, "etp", 11);
            potentialEvaporationEntry(attribs, "etps", 12);
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

ParameterGroup SaxHandler::nameToParameterGroup(QString name)
{
    QHash <QString, ParameterGroup> hash({
        {"Infiltrationsfaktoren", ParameterGroup::Infiltrationsfaktoren},
        {"Bagrovwerte", ParameterGroup::Bagrovwerte},
        {"ErgebnisNachkommaStellen", ParameterGroup::Nachkomma},
        {"Diverse", ParameterGroup::Diverse},
        {"Gewaesserverdunstung" , ParameterGroup::GewVerd},
        {"PotentielleVerdunstung", ParameterGroup::PotVerd}
    });

    return name.isEmpty() ?
        ParameterGroup::None :
        hash.contains(name) ? hash[name] : ParameterGroup::Invalid;
}

OutputColumn SaxHandler::nameToOutputColumn(QString name)
{
    QHash <QString, OutputColumn> hash({
        {"R", OutputColumn::TotalRunoff_R},
        {"ROW", OutputColumn::SurfaceRunoff_ROW},
        {"RI", OutputColumn::Infiltration_RI},
        {"RVOL", OutputColumn::TotalRunoffFlow_RVOL},
        {"ROWVOL", OutputColumn::SurfaceRunoffFlow_ROWVOL},
        {"RIVOL", OutputColumn::InfiltrationFlow_RIVOL},
        {"FLAECHE", OutputColumn::TotalArea_FLAECHE},
        {"VERDUNSTUNG", OutputColumn::Evaporation_VERDUNSTUN}
    });

    return name.isEmpty() ?
        OutputColumn::None :
        hash.contains(name) ? hash[name] : OutputColumn::Invalid;
}

int SaxHandler::surfaceNameToIndex(QString name)
{
    QHash <QString, int> hash({
        {"Dachflaechen", 0},
        {"Belaglsklasse1", 1},
        {"Belaglsklasse2", 2},
        {"Belaglsklasse3", 3},
        {"Belaglsklasse4", 4},
    });

    return hash[name];
}

void SaxHandler::potentialEvaporationEntry(
    const QXmlAttributes &attribs, QString name, int index
)
{
    QString element = attribs.value(name);

    if (element.length() > 0) {
        QString bezirke = attribs.value("bezirke");
        initValues.putToHashOfType(bezirke, element.toInt(), index);
    }
}

bool SaxHandler::fatalError(const QXmlParseException & /*exception*/)
{
    return false;
}
