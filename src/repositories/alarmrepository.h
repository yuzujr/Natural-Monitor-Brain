#ifndef ALARMREPOSITORY_H
#define ALARMREPOSITORY_H

#include <QDateTime>
#include <QList>
#include <QString>

#include "databasemanager.h"

class DatabaseManager;

class AlarmRepository
{
public:
    explicit AlarmRepository(DatabaseManager *db);

    bool insertAlarm(const QString &param, double value, double threshold, const QDateTime &timestamp = QDateTime::currentDateTime());
    QList<AlarmRecord> queryAlarms(const QDateTime &start, const QDateTime &end, int limit);
    bool clearAlarms();

private:
    DatabaseManager *db_ = nullptr;
};

#endif // ALARMREPOSITORY_H