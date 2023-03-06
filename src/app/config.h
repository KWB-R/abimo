#ifndef CONFIG_H
#define CONFIG_H

#include <QHash>
#include <QString>

#include "pdr.h" // for MainUsage, UsageResult, UsageTuple

class Config
{
public:

    Config();
    float getRootingDepth(Usage usage, int yield);
    UsageResult getUsageResult(int usageID, int type, QString code);
    UsageTuple getUsageTuple(int tupleID);

private:

    UsageTuple usageTuples[16];

    // assignment of usage identifiers to "type -> tuple index" hashes
    QHash<int,QHash<int,int>> usageHash;

    void initUsageYieldIrrigationTuples();
    void initUsageAndTypeToTupleHash();

    UsageResult lookup(QHash<int,int>hash, int type, QString code);
};

#endif // CONFIG_H
