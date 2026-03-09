#include "databasemanager.h"

#include <QCryptographicHash>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

namespace {
const char *kConnectionName = "env_db";
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

bool DatabaseManager::open()
{
    const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.isEmpty()) {
        lastError_ = QStringLiteral("无法定位应用数据目录");
        return false;
    }

    QDir dir(baseDir);
    if (!dir.exists() && !dir.mkpath(".")) {
        lastError_ = QStringLiteral("无法创建应用数据目录");
        return false;
    }

    dbPath_ = dir.filePath(QStringLiteral("environment.db"));
    if (QSqlDatabase::contains(kConnectionName)) {
        db_ = QSqlDatabase::database(kConnectionName);
    } else {
        db_ = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), kConnectionName);
    }
    db_.setDatabaseName(dbPath_);

    if (!db_.open()) {
        lastError_ = db_.lastError().text();
        return false;
    }

    if (!initSchema()) {
        return false;
    }

    if (!ensureDefaultAdmin()) {
        return false;
    }

    return true;
}

void DatabaseManager::close()
{
    if (db_.isOpen()) {
        db_.close();
    }
}

bool DatabaseManager::reopen()
{
    close();
    return open();
}

QString DatabaseManager::lastError() const
{
    return lastError_;
}

QString DatabaseManager::databasePath() const
{
    return dbPath_;
}

bool DatabaseManager::validateUser(const QString &username, const QString &password, UserInfo *outUser)
{
    if (!db_.isOpen()) {
        lastError_ = QStringLiteral("数据库未打开");
        return false;
    }

    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT id, username, role, created_at FROM users "
                                 "WHERE username = :username AND password_hash = :hash"));
    query.bindValue(QStringLiteral(":username"), username.trimmed());
    query.bindValue(QStringLiteral(":hash"), hashPassword(password));
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }

    if (query.next()) {
        if (outUser) {
            outUser->id = query.value(0).toInt();
            outUser->username = query.value(1).toString();
            outUser->role = query.value(2).toString();
            outUser->createdAt = QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong());
        }
        return true;
    }

    return false;
}

bool DatabaseManager::addUser(const QString &username, const QString &password, const QString &role)
{
    if (username.trimmed().isEmpty() || password.isEmpty()) {
        lastError_ = QStringLiteral("用户名或密码不能为空");
        return false;
    }

    QSqlQuery query(db_);
    query.prepare(QStringLiteral("INSERT INTO users (username, password_hash, role, created_at) "
                                 "VALUES (:username, :hash, :role, :created_at)"));
    query.bindValue(QStringLiteral(":username"), username.trimmed());
    query.bindValue(QStringLiteral(":hash"), hashPassword(password));
    query.bindValue(QStringLiteral(":role"), role);
    query.bindValue(QStringLiteral(":created_at"), QDateTime::currentDateTime().toMSecsSinceEpoch());
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::resetPassword(const QString &username, const QString &newPassword)
{
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("UPDATE users SET password_hash = :hash WHERE username = :username"));
    query.bindValue(QStringLiteral(":hash"), hashPassword(newPassword));
    query.bindValue(QStringLiteral(":username"), username.trimmed());
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<UserInfo> DatabaseManager::listUsers()
{
    QList<UserInfo> result;
    QSqlQuery query(db_);
    if (!query.exec(QStringLiteral("SELECT id, username, role, created_at FROM users ORDER BY id ASC"))) {
        lastError_ = query.lastError().text();
        return result;
    }

    while (query.next()) {
        UserInfo info;
        info.id = query.value(0).toInt();
        info.username = query.value(1).toString();
        info.role = query.value(2).toString();
        info.createdAt = QDateTime::fromMSecsSinceEpoch(query.value(3).toLongLong());
        result.push_back(info);
    }

    return result;
}

bool DatabaseManager::updateUserRole(int id, const QString &role)
{
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("UPDATE users SET role = :role WHERE id = :id"));
    query.bindValue(QStringLiteral(":role"), role);
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool DatabaseManager::deleteUser(int id)
{
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("DELETE FROM users WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool DatabaseManager::setUserPassword(int id, const QString &newPassword)
{
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("UPDATE users SET password_hash = :hash WHERE id = :id"));
    query.bindValue(QStringLiteral(":hash"), hashPassword(newPassword));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool DatabaseManager::insertSample(const EnvSample &sample)
{
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("INSERT INTO env_data (ts, temperature, humidity, pm25, co2) "
                                 "VALUES (:ts, :temperature, :humidity, :pm25, :co2)"));
    query.bindValue(QStringLiteral(":ts"), sample.ts.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":temperature"), sample.temperature);
    query.bindValue(QStringLiteral(":humidity"), sample.humidity);
    query.bindValue(QStringLiteral(":pm25"), sample.pm25);
    query.bindValue(QStringLiteral(":co2"), sample.co2);
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }
    return true;
}

QList<EnvSample> DatabaseManager::querySamples(const QDateTime &start, const QDateTime &end, int limit)
{
    QList<EnvSample> result;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT ts, temperature, humidity, pm25, co2 "
                                 "FROM env_data WHERE ts BETWEEN :start AND :end "
                                 "ORDER BY ts ASC LIMIT :limit"));
    query.bindValue(QStringLiteral(":start"), start.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":end"), end.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":limit"), limit);
    if (!query.exec()) {
        lastError_ = query.lastError().text();
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

EnvStats DatabaseManager::queryStats(const QDateTime &start, const QDateTime &end)
{
    EnvStats stats;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT "
                                 "MIN(temperature), MAX(temperature), AVG(temperature), "
                                 "MIN(humidity), MAX(humidity), AVG(humidity), "
                                 "MIN(pm25), MAX(pm25), AVG(pm25), "
                                 "MIN(co2), MAX(co2), AVG(co2) "
                                 "FROM env_data WHERE ts BETWEEN :start AND :end"));
    query.bindValue(QStringLiteral(":start"), start.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":end"), end.toMSecsSinceEpoch());
    if (!query.exec()) {
        lastError_ = query.lastError().text();
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

bool DatabaseManager::insertAlarm(const QString &param, double value, double threshold)
{
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("INSERT INTO alarms (ts, param, value, threshold) "
                                 "VALUES (:ts, :param, :value, :threshold)"));
    query.bindValue(QStringLiteral(":ts"), QDateTime::currentDateTime().toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":param"), param);
    query.bindValue(QStringLiteral(":value"), value);
    query.bindValue(QStringLiteral(":threshold"), threshold);
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }
    return true;
}

QList<AlarmRecord> DatabaseManager::queryAlarms(const QDateTime &start, const QDateTime &end, int limit)
{
    QList<AlarmRecord> result;
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("SELECT ts, param, value, threshold "
                                 "FROM alarms WHERE ts BETWEEN :start AND :end "
                                 "ORDER BY ts DESC LIMIT :limit"));
    query.bindValue(QStringLiteral(":start"), start.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":end"), end.toMSecsSinceEpoch());
    query.bindValue(QStringLiteral(":limit"), limit);
    if (!query.exec()) {
        lastError_ = query.lastError().text();
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

bool DatabaseManager::clearAlarms()
{
    QSqlQuery query(db_);
    if (!query.exec(QStringLiteral("DELETE FROM alarms"))) {
        lastError_ = query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteSamplesBefore(const QDateTime &cutoff)
{
    QSqlQuery query(db_);
    query.prepare(QStringLiteral("DELETE FROM env_data WHERE ts < :cutoff"));
    query.bindValue(QStringLiteral(":cutoff"), cutoff.toMSecsSinceEpoch());
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::initSchema()
{
    QSqlQuery query(db_);
    if (!query.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "username TEXT UNIQUE NOT NULL, "
            "password_hash TEXT NOT NULL, "
            "role TEXT NOT NULL, "
            "created_at INTEGER NOT NULL)"))) {
        lastError_ = query.lastError().text();
        return false;
    }

    if (!query.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS env_data ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "ts INTEGER NOT NULL, "
            "temperature REAL NOT NULL, "
            "humidity REAL NOT NULL, "
            "pm25 REAL NOT NULL, "
            "co2 REAL NOT NULL)"))) {
        lastError_ = query.lastError().text();
        return false;
    }

    if (!query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_env_data_ts ON env_data(ts)"))) {
        lastError_ = query.lastError().text();
        return false;
    }

    if (!query.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS alarms ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "ts INTEGER NOT NULL, "
            "param TEXT NOT NULL, "
            "value REAL NOT NULL, "
            "threshold REAL NOT NULL)"))) {
        lastError_ = query.lastError().text();
        return false;
    }

    if (!query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_alarms_ts ON alarms(ts)"))) {
        lastError_ = query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::ensureDefaultAdmin()
{
    QSqlQuery query(db_);
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM users WHERE role = 'admin'"))) {
        lastError_ = query.lastError().text();
        return false;
    }

    if (query.next() && query.value(0).toInt() > 0) {
        return true;
    }

    const QString username = QStringLiteral("admin");
    const QString password = QStringLiteral("admin123");
    return addUser(username, password, QStringLiteral("admin"));
}

QString DatabaseManager::hashPassword(const QString &password) const
{
    return QString::fromLatin1(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5).toHex());
}
