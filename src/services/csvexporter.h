#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

#include <QList>
#include <QString>

#include "common/moduletypes.h"
#include "databasemanager.h"

namespace CsvExporter {
bool exportSamples(const QList<EnvSample> &samples, const QString &path, const SampleSelection &selection);
bool exportAlarms(const QList<AlarmRecord> &records, const QString &path);
bool exportStats(const EnvStats &stats, const QString &path);
}

#endif // CSVEXPORTER_H