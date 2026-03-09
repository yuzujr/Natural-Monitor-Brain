#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QSqlDatabase>

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

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);

    bool open();
    void close();
    bool reopen();
    QString lastError() const;
    QString databasePath() const;

    bool validateUser(const QString &username, const QString &password, UserInfo *outUser);
    bool addUser(const QString &username, const QString &password, const QString &role);
    bool resetPassword(const QString &username, const QString &newPassword);
    QList<UserInfo> listUsers();
    bool updateUserRole(int id, const QString &role);
    bool deleteUser(int id);
    bool setUserPassword(int id, const QString &newPassword);

    bool insertSample(const EnvSample &sample);
    QList<EnvSample> querySamples(const QDateTime &start, const QDateTime &end, int limit);
    EnvStats queryStats(const QDateTime &start, const QDateTime &end);

    bool insertAlarm(const QString &param, double value, double threshold);
    QList<AlarmRecord> queryAlarms(const QDateTime &start, const QDateTime &end, int limit);
    bool clearAlarms();
    bool deleteSamplesBefore(const QDateTime &cutoff);

private:
    bool initSchema();
    bool ensureDefaultAdmin();
    QString hashPassword(const QString &password) const;

    QSqlDatabase db_;
    QString lastError_;
    QString dbPath_;
};

#endif // DATABASEMANAGER_H
