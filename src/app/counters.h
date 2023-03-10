#ifndef COUNTERS_H
#define COUNTERS_H

class Counters
{
public:
    Counters();

    void initialise();

    void setRecordsRead(int n);
    void setRecordsWritten(int n);
    void incrementRecordsProtocol();
    void incrementIrrigationForcedToZero();
    void incrementNoAreaGiven();
    void incrementNoUsageGiven();

    int getRecordsRead();
    int getRecordsWritten();
    int getRecordsProtocol();
    int getIrrigationForcedToZero();
    int getNoAreaGiven();
    int getNoUsageGiven();

private:

    // Total records read
    int recordsRead;

    // Total records written
    int recordsWritten;

    // Number of records written to the (error) protocol
    int recordsProtocol;

    // Number of records for which irrigation (BER) was set to zero
    int irrigationForcedToZero;

    // Number of cases in which no calculation was performed
    int noAreaGiven;
    int noUsageGiven;
};

#endif // COUNTERS_H
