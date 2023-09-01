#ifndef USAGECONFIGURATION_H
#define USAGECONFIGURATION_H

#include <QHash>
#include <QString>

#include "structs.h" // for MainUsage, UsageResult, UsageTuple

class UsageConfiguration
{
public:

    UsageConfiguration();
    UsageResult getUsageResult(int usageID, int type, QString code);
    UsageTuple getUsageTuple(int tupleID);

private:

    UsageTuple usageTuples[16];

    // assignment of usage identifiers to "type -> tuple index" hashes
    QHash<int,QHash<int,int>> usageHash;

    void initUsageYieldIrrigationTuples();
    void initUsageAndTypeToTupleHash();

    UsageResult lookup(QHash<int,int>hash, int type, QString code);
    QString notDefinedMessage(QString& code, int type);
};

#endif // USAGECONFIGURATION_H
