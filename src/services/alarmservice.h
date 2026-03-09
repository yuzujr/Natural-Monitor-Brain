#ifndef ALARMSERVICE_H
#define ALARMSERVICE_H

#include <QDateTime>
#include <QList>

#include "common/moduletypes.h"
#include "databasemanager.h"

class AlarmRepository;

class AlarmService
{
public:
    explicit AlarmService(AlarmRepository *alarmRepository);

    QList<AlarmEvent> evaluateSample(const EnvSample &sample, const AlarmSettings &settings);

private:
    void evaluateMetric(const QString &param, double value, double threshold, const QDateTime &now,
                        int cooldownSeconds, QDateTime &lastTriggeredAt, QList<AlarmEvent> &events);

    AlarmRepository *alarmRepository_ = nullptr;
    QDateTime lastAlarmTemp_;
    QDateTime lastAlarmHum_;
    QDateTime lastAlarmPm_;
    QDateTime lastAlarmCo2_;
};

#endif // ALARMSERVICE_H