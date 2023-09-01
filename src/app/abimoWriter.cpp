#include "abimoWriter.h"
#include "dbaseWriter.h"

#include <QString>

#include "dbaseField.h"
#include "initValues.h"
#include "structs.h"

AbimoWriter::AbimoWriter(QString& filePath, InitValues& initValues) :
    DbaseWriter(filePath)
{
    // Felder mit Namen, Typ, Nachkommastellen
    defineField("CODE", "C", 0);
    defineField("R", "N", initValues.getResultDigits(OutputColumn::TotalRunoff_R));
    defineField("ROW", "N", initValues.getResultDigits(OutputColumn::SurfaceRunoff_ROW));
    defineField("RI", "N", initValues.getResultDigits(OutputColumn::Infiltration_RI));
    defineField("RVOL", "N", initValues.getResultDigits(OutputColumn::TotalRunoffFlow_RVOL));
    defineField("ROWVOL", "N", initValues.getResultDigits(OutputColumn::SurfaceRunoffFlow_ROWVOL));
    defineField("RIVOL", "N", initValues.getResultDigits(OutputColumn::InfiltrationFlow_RIVOL));
    defineField("FLAECHE", "N", initValues.getResultDigits(OutputColumn::TotalArea_FLAECHE));
    defineField("VERDUNSTUN", "N", initValues.getResultDigits(OutputColumn::Evaporation_VERDUNSTUN));

    assert(getNumberOfFields() == 9);
}
