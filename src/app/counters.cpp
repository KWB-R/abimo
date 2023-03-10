#include "counters.h"

Counters::Counters()
{
    initialise();
}

void Counters::initialise() {
    recordsRead = 0;
    recordsWritten = 0;
    recordsProtocol = 0;
    irrigationForcedToZero = 0;
    noAreaGiven = 0;
    noUsageGiven = 0;
}

void Counters::setRecordsRead(int n) {
    recordsRead = n;
}

void Counters::setRecordsWritten(int n) {
    recordsWritten = n;
}

void Counters::incrementRecordsProtocol() {
    recordsProtocol++;
}

void Counters::incrementIrrigationForcedToZero() {
    irrigationForcedToZero++;
}

void Counters::incrementNoAreaGiven() {
    noAreaGiven++;
}

void Counters::incrementNoUsageGiven() {
    noUsageGiven++;
}

int Counters::getRecordsRead() {
    return recordsRead;
}

int Counters::getRecordsWritten() {
    return recordsWritten;
}

int Counters::getRecordsProtocol() {
    return recordsProtocol;
}

int Counters::getIrrigationForcedToZero() {
    return irrigationForcedToZero;
}

int Counters::getNoAreaGiven() {
    return noAreaGiven;
}

int Counters::getNoUsageGiven() {
    return noUsageGiven;
}
