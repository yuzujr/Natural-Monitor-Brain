#ifndef SAMPLEREPOSITORY_H
#define SAMPLEREPOSITORY_H

#include <QDateTime>
#include <QList>

#include "databasemanager.h"

class DatabaseManager;

class SampleRepository
{
public:
    explicit SampleRepository(DatabaseManager *db);

    bool insertSample(const EnvSample &sample);
    QList<EnvSample> querySamples(const QDateTime &start, const QDateTime &end, int limit);
    EnvStats queryStats(const QDateTime &start, const QDateTime &end);
    bool deleteSamplesBefore(const QDateTime &cutoff);

private:
    DatabaseManager *db_ = nullptr;
};

#endif // SAMPLEREPOSITORY_H