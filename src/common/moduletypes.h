#ifndef MODULETYPES_H
#define MODULETYPES_H

#include <QString>

struct SampleSelection
{
    bool temperature = true;
    bool humidity = true;
    bool pm25 = true;
    bool co2 = true;
};

struct AlarmSettings
{
    bool enabled = true;
    int cooldownSeconds = 30;
    double temperatureThreshold = 35.0;
    double humidityThreshold = 85.0;
    double pm25Threshold = 150.0;
    double co2Threshold = 1200.0;
};

struct AlarmEvent
{
    QString param;
    double value = 0.0;
    double threshold = 0.0;
};

#endif // MODULETYPES_H