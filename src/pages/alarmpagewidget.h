#ifndef ALARMPAGEWIDGET_H
#define ALARMPAGEWIDGET_H

#include <QDateTime>
#include <QWidget>

#include "../common/moduletypes.h"
#include "databasemanager.h"

class QCheckBox;
class QDateTimeEdit;
class QDoubleSpinBox;
class QSpinBox;
class QTableWidget;

class AlarmPageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AlarmPageWidget(QWidget *parent = nullptr);

    AlarmSettings alarmSettings() const;
    QDateTime startDateTime() const;
    QDateTime endDateTime() const;
    void setAlarmSettings(const AlarmSettings &settings);
    void setAlarmRecords(const QList<AlarmRecord> &records);

signals:
    void saveThresholdsRequested();
    void refreshRequested(const QDateTime &start, const QDateTime &end);
    void clearRequested();

private:
    QCheckBox *enabledCheck_ = nullptr;
    QSpinBox *cooldownSpin_ = nullptr;
    QDoubleSpinBox *tempThreshold_ = nullptr;
    QDoubleSpinBox *humThreshold_ = nullptr;
    QDoubleSpinBox *pmThreshold_ = nullptr;
    QDoubleSpinBox *co2Threshold_ = nullptr;
    QDateTimeEdit *startEdit_ = nullptr;
    QDateTimeEdit *endEdit_ = nullptr;
    QTableWidget *alarmTable_ = nullptr;
};

#endif // ALARMPAGEWIDGET_H
