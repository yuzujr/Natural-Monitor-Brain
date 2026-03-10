#include "alarmrepository.h"

#include <QSqlError>
#include <QSqlQuery>
//初始化仓库类实例，建立与数据库管理器的联系。
AlarmRepository::AlarmRepository(DatabaseManager *db)
    : db_(db)
{
}
//当系统判定发生异常时，将报警的详细信息永久保存到数据库中。
bool AlarmRepository::insertAlarm(const QString &param, double value, double threshold, const QDateTime &timestamp)
{
    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("INSERT INTO alarms (ts, param, value, threshold) "
                                 "VALUES (:ts, :param, :value, :threshold)"));
    query.bindValue(QStringLiteral(":ts"), timestamp.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":param"), param);
    query.bindValue(QStringLiteral(":value"), value);
    query.bindValue(QStringLiteral(":threshold"), threshold);
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return false;
    }
    return true;
}
//根据用户在界面上选择的时间范围，从数据库中提取历史报警记录。
QList<AlarmRecord> AlarmRepository::queryAlarms(const QDateTime &start, const QDateTime &end, int limit)
{
    QList<AlarmRecord> result;
    QSqlQuery query(db_->database());
    query.prepare(QStringLiteral("SELECT ts, param, value, threshold "
                                 "FROM alarms WHERE ts BETWEEN :start AND :end "
                                 "ORDER BY ts DESC LIMIT :limit"));
    query.bindValue(QStringLiteral(":start"), start.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":end"), end.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":limit"), limit);
    if (!query.exec()) {
        db_->setLastError(query.lastError().text());
        return result;
    }

    while (query.next()) {
        AlarmRecord record;
        record.ts = QDateTime::fromMSecsSinceEpoch(query.value(0).toLongLong());
        record.param = query.value(1).toString();
        record.value = query.value(2).toDouble();
        record.threshold = query.value(3).toDouble();
        result.push_back(record);
    }
    return result;
}
//实现界面上的“一键清空”功能。
bool AlarmRepository::clearAlarms()
{
    QSqlQuery query(db_->database());
    if (!query.exec(QStringLiteral("DELETE FROM alarms"))) {
        db_->setLastError(query.lastError().text());
        return false;
    }
    return true;
}