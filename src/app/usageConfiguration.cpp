#include <QHash>

#include "usageConfiguration.h"
#include "soilAndVegetation.h"

UsageConfiguration::UsageConfiguration()
{    
    initUsageYieldIrrigationTuples();
    initUsageAndTypeToTupleHash();
}

void UsageConfiguration::initUsageYieldIrrigationTuples()
{
    // Define all different value combinations
    // (usage - NUT, yield power - ERT, irrigation - BER)
    // NUT = 'K': gardening
    // NUT = 'L': agricultural land use

    usageTuples[ 0] = {Usage::vegetationless_D,  1,   0};
    usageTuples[ 1] = {Usage::waterbody_G,       0,   0};
    usageTuples[ 2] = {Usage::horticultural_K,  40,  75};
    usageTuples[ 3] = {Usage::agricultural_L,   10,   0};
    usageTuples[ 4] = {Usage::agricultural_L,   25,   0};
    usageTuples[ 5] = {Usage::agricultural_L,   30,   0};
    usageTuples[ 6] = {Usage::agricultural_L,   35,   0};
    usageTuples[ 7] = {Usage::agricultural_L,   40,   0};
    usageTuples[ 8] = {Usage::agricultural_L,   45,   0};
    usageTuples[ 9] = {Usage::agricultural_L,   45,  50};
    usageTuples[10] = {Usage::agricultural_L,   50,   0};
    usageTuples[11] = {Usage::agricultural_L,   50, 100};
    usageTuples[12] = {Usage::agricultural_L,   50,  50};
    usageTuples[13] = {Usage::agricultural_L,   55,  75};
    usageTuples[14] = {Usage::agricultural_L,   60,   0};
    usageTuples[15] = {Usage::forested_W,        0,   0};
}

//==============================================================================
// Define the mapping between usage and type identifiers and (usage, yield,
// irrigation)-tuples
//==============================================================================

void UsageConfiguration::initUsageAndTypeToTupleHash()
{
    // assignment of type identifiers to tuple indices
    //
    // For each type, define the "tuple index", i.e. the index in the array
    // "usageTuples" that holds the required (usage, yield, irrigation)-tuple.
    // The entry type2tuple[-1] defines the "default behaviour" in case that
    // type2tuple[type] does not exist: the value in type2tuple[-1] is then
    // not a tuple index but the "alternative type" of which to lookup the tuple
    // index instead. For example, "type2tuple[-1] = 72" means: "For any types
    // that are not found as keys in type2tuple, use type2tuple[72] (=10) as the
    // tuple index. This finally leads to the (usage, yield, irrigation)-tuple
    // usageTuples[10] = {'L', 50, 0}.

    QHash<int,int> type2tuple;

    type2tuple[ 1] =  6;
    type2tuple[ 2] =  6;
    type2tuple[ 3] =  8;
    type2tuple[ 4] =  7;
    type2tuple[ 5] =  7;
    type2tuple[ 6] =  7;
    type2tuple[ 7] =  6;
    type2tuple[ 8] =  6;
    type2tuple[ 9] =  7;
    type2tuple[10] =  7;
    type2tuple[11] =  6;
    type2tuple[21] =  8;
    type2tuple[22] =  2;
    type2tuple[23] =  2;
    type2tuple[24] = 13;
    type2tuple[25] =  2;
    type2tuple[26] =  7;
    type2tuple[29] =  5;
    type2tuple[33] =  6;
    type2tuple[38] =  6;
    type2tuple[39] =  6;
    type2tuple[71] =  8;
    type2tuple[72] = 10;
    type2tuple[73] = 10;
    type2tuple[74] = 10;
    type2tuple[-1] = 72; // lookup again for (default) type 72

    // assign the type2tuple hash to a set of usage identifiers
    usageHash[10] = type2tuple;
    usageHash[21] = type2tuple;
    usageHash[22] = type2tuple;
    usageHash[23] = type2tuple;
    usageHash[30] = type2tuple;

    // redefine the type2tuple hash and assign it to another usage identifier
    type2tuple.clear();
    type2tuple[30] = 6;
    type2tuple[31] = 5;
    type2tuple[-1] = 31;
    usageHash[40] = type2tuple;

    type2tuple.clear();
    type2tuple[12] =  8;
    type2tuple[13] = 10;
    type2tuple[14] = 10;
    type2tuple[28] =  7;
    type2tuple[41] =  7;
    type2tuple[42] =  6;
    type2tuple[43] =  6;
    type2tuple[44] =  9;
    type2tuple[45] =  7;
    type2tuple[46] = 12;
    type2tuple[47] =  8;
    type2tuple[49] =  9;
    type2tuple[50] =  9;
    type2tuple[51] =  8;
    type2tuple[60] =  8;
    type2tuple[-1] = 60;
    usageHash[50] = type2tuple;

    // For usage "60", see below

    type2tuple.clear();
    type2tuple[59] =  2;
    type2tuple[-1] = 59;
    usageHash[70] = type2tuple;

    type2tuple.clear();
    type2tuple[91] =  7;
    type2tuple[92] =  4;
    type2tuple[93] =  5;
    type2tuple[94] =  5;
    type2tuple[99] =  3;
    type2tuple[-1] = 99;
    usageHash[80] = type2tuple;

    type2tuple.clear();
    type2tuple[98] =  0;
    type2tuple[-1] = 98;
    usageHash[90] = type2tuple;

    type2tuple.clear();
    type2tuple[55] = 15;
    type2tuple[-1] = 55;
    usageHash[100] = type2tuple;

    // Usages for which parameters are independent of type. The value "-2"
    // is an indicator for: No matter what value "type" has, use the hash value
    // type2tuple[-2] as the "tuple index", i.e. find the (usage, yield,
    // irrigation)-tuple in usageTuples[type2tuple[-2]].

    type2tuple.clear(); type2tuple[-2] =  8; usageHash[ 60] = type2tuple;
    type2tuple.clear(); type2tuple[-2] = 15; usageHash[101] = type2tuple;
    type2tuple.clear(); type2tuple[-2] = 14; usageHash[102] = type2tuple;
    type2tuple.clear(); type2tuple[-2] =  1; usageHash[110] = type2tuple;
    type2tuple.clear(); type2tuple[-2] =  7; usageHash[121] = type2tuple;
    type2tuple.clear(); type2tuple[-2] =  6; usageHash[122] = type2tuple;
    type2tuple.clear(); type2tuple[-2] = 12; usageHash[130] = type2tuple;
    type2tuple.clear(); type2tuple[-2] = 10; usageHash[140] = type2tuple;
    type2tuple.clear(); type2tuple[-2] = 11; usageHash[150] = type2tuple;
    type2tuple.clear(); type2tuple[-2] =  2; usageHash[160] = type2tuple;
    type2tuple.clear(); type2tuple[-2] =  2; usageHash[161] = type2tuple;
    type2tuple.clear(); type2tuple[-2] =  2; usageHash[162] = type2tuple;
    type2tuple.clear(); type2tuple[-2] =  3; usageHash[170] = type2tuple;
    type2tuple.clear(); type2tuple[-2] =  0; usageHash[171] = type2tuple;
    type2tuple.clear(); type2tuple[-2] =  7; usageHash[172] = type2tuple;
    type2tuple.clear(); type2tuple[-2] =  8; usageHash[173] = type2tuple;
    type2tuple.clear(); type2tuple[-2] = 14; usageHash[174] = type2tuple;
    type2tuple.clear(); type2tuple[-2] = 10; usageHash[180] = type2tuple;
    type2tuple.clear(); type2tuple[-2] =  7; usageHash[190] = type2tuple;
    type2tuple.clear(); type2tuple[-2] = 12; usageHash[200] = type2tuple;
}

//==============================================================================
//    Bestimmung der Durchwurzelungstiefe TWS
//==============================================================================
float UsageConfiguration::getRootingDepth(Usage usage, int yield)
{
    // Zuordnung Durchwurzelungstiefe in Abhaengigkeit der Nutzung
    switch(usage) {

        // D - Devastierung
        case Usage::vegetationless_D: return 0.2F;

        // L - landwirtschaftliche Nutzung
        case Usage::agricultural_L: return (yield <= 50) ? 0.6F : 0.7F;

        // K - Kleingaerten
        case Usage::horticultural_K: return 0.7F;

        // W - Wald
        case Usage::forested_W: return 1.0F;

        // Other
        default: return 0.2F;
    }
}

UsageResult UsageConfiguration::getUsageResult(int usageID, int type, QString code)
{
    if (!usageHash.contains(usageID)) {
        return {
            -1,
            QString("\r\nDiese  Meldung sollte nie erscheinen: \r\n") +
                "Nutzung nicht definiert fuer Element " + code + "\r\n"
        };
    }

    return lookup(usageHash[usageID], type, code);
}

UsageResult UsageConfiguration::lookup(QHash<int,int>hash, int type, QString code)
{
    if (hash.contains(type)) {
        return {hash[type], ""};
    }

    if (hash.contains(-1)) {
        int defaultType = hash[-1];
        return {hash[defaultType], notDefinedMessage(code, defaultType)};
    }

    return {hash[-2], ""};
}

QString UsageConfiguration::notDefinedMessage(QString& code, int type)
{
    QString message = "\r\nNutzungstyp nicht definiert fuer Element " +
        code + "\r\nTyp=" + QString::number(type) +
        " angenommen\r\n";

    return message;
}

UsageTuple UsageConfiguration::getUsageTuple(int tupleID)
{
    assert(tupleID >= 0);
    return usageTuples[tupleID];
}

/*
usage_groups:
usage_group,usages
g1,10|21|22|23|30

{usage_group,type,id}
{g1,1,6}
{g1,2,6}
{g1,3,8}
{g1,4,7}
{g1,5,7}
{g1,6,7}
{g1,7,6}
{g1,8,6}
{g1,9,7}
{g1,10,7}
{g1,11,6}
{g1,21,8}
{g1,22,2}
{g1,23,2}
{g1,24,13}
{g1,25,2}
{g1,26,7}
{g1,29,5}
{g1,33,6}
{g1,38,6}
{g1,39,6}
{g1,71,8}
{g1,72,10}
{g1,73,10}
{g1,74,10}
{g1,-72,10} -> notDefinedMessage(code, 72);

{40,30,6}
{40,31,5}
{40,-31,5} -> notDefinedMessage(code, 31);

{50,12,8}
{50,13,10}
{50,14,10}
{50,28,7}
{50,41,7}
{50,42,6}
{50,43,6}
{50,44,9}
{50,45,7}
{50,46,12}
{50,47,8}
{50,49,9}
{50,50,9}
{50,51,8}
{50,60,8}
{50,-60,8} -> notDefinedMessage(code, 60);

{60,*,8}

{70,59,2}
{70,-59,2} -> notDefinedMessage(code, 59);

{80,91,7}
{80,92,4}
{80,93,5}
{80,94,5}
{80,99,3}
{80,-99,3} -> notDefinedMessage(code, 99);

{90,98,0}
{90,-98,0) -> notDefinedMessage(code, 98);

{100,55,15}
{100,-55,15} -> notDefinedMessage(code, 55);

{101,*,15}
{102,*,14}
{110,*,1}
{121,*,7}
{122,*,6}
{130,*,12}
{140,*,10}
{150,*,11}
{160,*,2}
{161,*,2}
{162,*,2}
{170,*,3}
{171,*,0}
{172,*,7}
{173,*,8}
{174,*,14}
{180,*,10}
{190,*,7}
{200,*,12}
*/
