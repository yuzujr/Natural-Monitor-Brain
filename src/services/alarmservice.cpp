#include "alarmservice.h"

#include "repositories/alarmrepository.h"

AlarmService::AlarmService(AlarmRepository *alarmRepository)
    : alarmRepository_(alarmRepository)
{
}

QList<AlarmEvent> AlarmService::evaluateSample(const EnvSample &sample, const AlarmSettings &settings)
{
    QList<AlarmEvent> events;
    if (!settings.enabled) {
        return events;
    }

    const QDateTime now = sample.ts.isValid() ? sample.ts : QDateTime::currentDateTime();
    evaluateMetric(QObject::tr("温度"), sample.temperature, settings.temperatureThreshold, now,
                   settings.cooldownSeconds, lastAlarmTemp_, events);
    evaluateMetric(QObject::tr("湿度"), sample.humidity, settings.humidityThreshold, now,
                   settings.cooldownSeconds, lastAlarmHum_, events);
    evaluateMetric(QObject::tr("PM2.5"), sample.pm25, settings.pm25Threshold, now,
                   settings.cooldownSeconds, lastAlarmPm_, events);
    evaluateMetric(QObject::tr("CO2"), sample.co2, settings.co2Threshold, now,
                   settings.cooldownSeconds, lastAlarmCo2_, events);
    return events;
}

void AlarmService::evaluateMetric(const QString &param, double value, double threshold, const QDateTime &now,
                                  int cooldownSeconds, QDateTime &lastTriggeredAt, QList<AlarmEvent> &events)
{
    if (value <= threshold) {
        return;
    }
    if (lastTriggeredAt.isValid() && lastTriggeredAt.secsTo(now) < cooldownSeconds) {
        return;
    }

    lastTriggeredAt = now;
    if (alarmRepository_) {
        alarmRepository_->insertAlarm(param, value, threshold, now);
    }

    AlarmEvent event;
    event.param = param;
    event.value = value;
    event.threshold = threshold;
    events.push_back(event);
}