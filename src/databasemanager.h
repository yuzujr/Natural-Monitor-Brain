#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QDateTime>
#include <QSqlDatabase>

class AlarmRepository;
class SampleRepository;
class UserRepository;

struct UserInfo
{
    int id = -1;
    QString username;
    QString role;
    QDateTime createdAt;
};

struct EnvSample
{
    QDateTime ts;
    double temperature = 0.0;
    double humidity = 0.0;
    double pm25 = 0.0;
    double co2 = 0.0;
};

struct EnvStats
{
    bool valid = false;
    double minTemp = 0.0;
    double maxTemp = 0.0;
    double avgTemp = 0.0;
    double minHum = 0.0;
    double maxHum = 0.0;
    double avgHum = 0.0;
    double minPm = 0.0;
    double maxPm = 0.0;
    double avgPm = 0.0;
    double minCo2 = 0.0;
    double maxCo2 = 0.0;
    double avgCo2 = 0.0;
};

struct AlarmRecord
{
    QDateTime ts;
    QString param;
    double value = 0.0;
    double threshold = 0.0;
};

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool open();
    void close();
    bool reopen();
    QString lastError() const;
    QString databasePath() const;
    QSqlDatabase database() const;
    void setLastError(const QString &error);
    QString hashPassword(const QString &password) const;

    UserRepository *userRepository() const;
    SampleRepository *sampleRepository() const;
    AlarmRepository *alarmRepository() const;

private:
    bool initSchema();
    bool ensureDefaultAdmin();

    QSqlDatabase db_;
    QString lastError_;
    QString dbPath_;
    UserRepository *userRepository_ = nullptr;
    SampleRepository *sampleRepository_ = nullptr;
    AlarmRepository *alarmRepository_ = nullptr;
};

#endif // DATABASEMANAGER_H
