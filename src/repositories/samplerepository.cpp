#include "samplerepository.h"

#include <QSqlError>
#include <QSqlQuery>

SampleRepository::SampleRepository(DatabaseManager *db)
    : db_(db)
{
}

bool SampleRepository::insertSample(const EnvSample &sample)
{
    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("INSERT INTO env_data (ts, temperature, humidity, pm25, co2) "
                                 "VALUES (:ts, :temperature, :humidity, :pm25, :co2)"));
    query.bindValue(QStringLiteral(":ts"), sample.ts.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":temperature"), sample.temperature);
    query.bindValue(QStringLiteral(":humidity"), sample.humidity);
    query.bindValue(QStringLiteral(":pm25"), sample.pm25);
    query.bindValue(QStringLiteral(":co2"), sample.co2);
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return false;
    }
    return true;
}

QList<EnvSample> SampleRepository::querySamples(const QDateTime &start, const QDateTime &end, int limit)
{
    QList<EnvSample> result;
    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("SELECT ts, temperature, humidity, pm25, co2 "
                                 "FROM env_data WHERE ts BETWEEN :start AND :end "
                                 "ORDER BY ts ASC LIMIT :limit"));
    query.bindValue(QStringLiteral(":start"), start.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":end"), end.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":limit"), limit);
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return result;
    }

    while (query.next()) {
        EnvSample sample;
        sample.ts = QDateTime::fromMSecsSinceEpoch(query.value(0).toLongLong());
        sample.temperature = query.value(1).toDouble();
        sample.humidity = query.value(2).toDouble();
        sample.pm25 = query.value(3).toDouble();
        sample.co2 = query.value(4).toDouble();
        result.push_back(sample);
    }
    return result;
}

EnvStats SampleRepository::queryStats(const QDateTime &start, const QDateTime &end)
{
    EnvStats stats;
    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("SELECT "
                                 "MIN(temperature), MAX(temperature), AVG(temperature), "
                                 "MIN(humidity), MAX(humidity), AVG(humidity), "
                                 "MIN(pm25), MAX(pm25), AVG(pm25), "
                                 "MIN(co2), MAX(co2), AVG(co2) "
                                 "FROM env_data WHERE ts BETWEEN :start AND :end"));
    query.bindValue(QStringLiteral(":start"), start.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":end"), end.toMSecsSinceEpoch());
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return stats;
    }

    if (query.next() && !query.isNull(0)) {
        stats.valid = true;
        stats.minTemp = query.value(0).toDouble();
        stats.maxTemp = query.value(1).toDouble();
        stats.avgTemp = query.value(2).toDouble();
        stats.minHum = query.value(3).toDouble();
        stats.maxHum = query.value(4).toDouble();
        stats.avgHum = query.value(5).toDouble();
        stats.minPm = query.value(6).toDouble();
        stats.maxPm = query.value(7).toDouble();
        stats.avgPm = query.value(8).toDouble();
        stats.minCo2 = query.value(9).toDouble();
        stats.maxCo2 = query.value(10).toDouble();
        stats.avgCo2 = query.value(11).toDouble();
    }
    return stats;
}

bool SampleRepository::deleteSamplesBefore(const QDateTime &cutoff)
{
    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("DELETE FROM env_data WHERE ts < :cutoff"));
    query.bindValue(QStringLiteral(":cutoff"), cutoff.toMSecsSinceEpoch());
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return false;
    }
    return true;
}