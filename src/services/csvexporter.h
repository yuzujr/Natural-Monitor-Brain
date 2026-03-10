#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

#include <QList>
#include <QString>
#include <functional>

#include "common/moduletypes.h"
#include "databasemanager.h"

namespace CsvExporter {
using ProgressCallback = std::function<void(int current, int total)>;

bool exportSamples(const QList<EnvSample> &samples, const QString &path, const SampleSelection &selection,
                    const ProgressCallback &callback = nullptr);
bool exportAlarms(const QList<AlarmRecord> &records, const QString &path,
                  const ProgressCallback &callback = nullptr);
bool exportStats(const EnvStats &stats, const QString &path);
}

#endif // CSVEXPORTER_H