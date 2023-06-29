#include "abimoWriter.h"
#include "dbaseWriter.h"

#include <QString>

#include "dbaseField.h"
#include "initValues.h"

AbimoWriter::AbimoWriter(QString& filePath, InitValues& initValues) :
    DbaseWriter(filePath)
{
    // Felder mit Namen, Typ, Nachkommastellen
    defineField("CODE", "C", 0);
    defineField("R", "N", initValues.getDigitsTotalRunoff_R());
    defineField("ROW", "N", initValues.getDigitsSurfaceRunoff_ROW());
    defineField("RI", "N", initValues.getDigitsInfiltration_RI());
    defineField("RVOL", "N", initValues.getDigitsTotalRunoffFlow_RVOL());
    defineField("ROWVOL", "N", initValues.getDigitsSurfaceRunoffFlow_ROWVOL());
    defineField("RIVOL", "N", initValues.getDigitsInfiltrationFlow_RIVOL());
    defineField("FLAECHE", "N", initValues.getDigitsTotalArea_FLAECHE());
    defineField("VERDUNSTUN", "N", initValues.getDigitsEvaporation_VERDUNSTUN());

    assert(getNumberOfFields() == 9);
}
