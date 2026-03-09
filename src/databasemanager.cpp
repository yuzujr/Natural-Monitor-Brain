#include "databasemanager.h"

#include "repositories/alarmrepository.h"
#include "repositories/samplerepository.h"
#include "repositories/userrepository.h"

#include <QCryptographicHash>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

namespace {
const char *kConnectionName = "env_db";
}

DatabaseManager::DatabaseManager()
    : userRepository_(new UserRepository(this))
    , sampleRepository_(new SampleRepository(this))
    , alarmRepository_(new AlarmRepository(this))
{
}

DatabaseManager::~DatabaseManager()
{
    delete alarmRepository_;
    delete sampleRepository_;
    delete userRepository_;
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

QSqlDatabase DatabaseManager::database() const
{
    return db_;
}

void DatabaseManager::setLastError(const QString &error)
{
    lastError_ = error;
}

UserRepository *DatabaseManager::userRepository() const
{
    return userRepository_;
}

SampleRepository *DatabaseManager::sampleRepository() const
{
    return sampleRepository_;
}

AlarmRepository *DatabaseManager::alarmRepository() const
{
    return alarmRepository_;
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
    return userRepository_->addUser(username, password, QStringLiteral("admin"));
}

QString DatabaseManager::hashPassword(const QString &password) const
{
    return QString::fromLatin1(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5).toHex());
}
