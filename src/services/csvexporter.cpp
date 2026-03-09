#include "csvexporter.h"

#include <QFile>
#include <QTextStream>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace {
void setUtf8(QTextStream &out)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
}
}

namespace CsvExporter {

bool exportSamples(const QList<EnvSample> &samples, const QString &path, const SampleSelection &selection)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    setUtf8(out);

    QStringList headers;
    headers << QObject::tr("时间");
    if (selection.temperature) {
        headers << QObject::tr("温度");
    }
    if (selection.humidity) {
        headers << QObject::tr("湿度");
    }
    if (selection.pm25) {
        headers << QObject::tr("PM2.5");
    }
    if (selection.co2) {
        headers << QObject::tr("CO2");
    }
    out << headers.join(',') << "\n";

    for (const EnvSample &sample : samples) {
        QStringList row;
        row << sample.ts.toString("yyyy-MM-dd HH:mm:ss");
        if (selection.temperature) {
            row << QString::number(sample.temperature, 'f', 1);
        }
        if (selection.humidity) {
            row << QString::number(sample.humidity, 'f', 1);
        }
        if (selection.pm25) {
            row << QString::number(sample.pm25, 'f', 1);
        }
        if (selection.co2) {
            row << QString::number(sample.co2, 'f', 0);
        }
        out << row.join(',') << "\n";
    }

    return true;
}

bool exportAlarms(const QList<AlarmRecord> &records, const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    setUtf8(out);
    out << QObject::tr("时间,参数,值,阈值") << "\n";

    for (const AlarmRecord &record : records) {
        out << record.ts.toString("yyyy-MM-dd HH:mm:ss") << ","
            << record.param << ","
            << QString::number(record.value, 'f', 1) << ","
            << QString::number(record.threshold, 'f', 1)
            << "\n";
    }

    return true;
}

bool exportStats(const EnvStats &stats, const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    setUtf8(out);
    out << QObject::tr("参数,最小值,最大值,平均值") << "\n";

    if (!stats.valid) {
        return true;
    }

    auto writeRow = [&](const QString &name, double minValue, double maxValue, double avgValue) {
        out << name << ","
            << QString::number(minValue, 'f', 1) << ","
            << QString::number(maxValue, 'f', 1) << ","
            << QString::number(avgValue, 'f', 1)
            << "\n";
    };

    writeRow(QObject::tr("温度"), stats.minTemp, stats.maxTemp, stats.avgTemp);
    writeRow(QObject::tr("湿度"), stats.minHum, stats.maxHum, stats.avgHum);
    writeRow(QObject::tr("PM2.5"), stats.minPm, stats.maxPm, stats.avgPm);
    writeRow(QObject::tr("CO2"), stats.minCo2, stats.maxCo2, stats.avgCo2);

    return true;
}

} // namespace CsvExporter