#ifndef CONFIG_H
#define CONFIG_H

#include <QHash>
#include <QString>

struct UsageResult{
    int tupleIndex;
    QString message;
};

struct UsageTuple{
    char usage;
    int yield;
    int irrigation;
};

class Config
{
public:
    Config();
    float getTWS(int ert, char nutz);
    UsageResult getUsageResult(int usage, int type, QString code);
    UsageTuple getUsageTuple(int tupleID);

private:
    UsageTuple usageTuples[16];
    void initUsageYieldIrrigationTuples();
    UsageResult lookup(QHash<int,int>hash, int type, QString code);
};

#endif // CONFIG_H
