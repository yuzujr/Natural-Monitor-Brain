#ifndef REALTIMEPAGEWIDGET_H
#define REALTIMEPAGEWIDGET_H

#include <QWidget>

#include "databasemanager.h"

class QLabel;
class QComboBox;
class QProgressBar;
class QPushButton;
class LineChartWidget;

class RealtimePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RealtimePageWidget(QWidget *parent = nullptr);

    void setRefreshInterval(int ms);
    void setSimulationRunning(bool running);
    void updateSample(const EnvSample &sample);
    void addSample(const EnvSample &sample);

signals:
    void refreshIntervalChanged(int ms);
    void simulationToggleRequested();

private:
    LineChartWidget *chart_ = nullptr;
    QLabel *tempValue_ = nullptr;
    QLabel *humValue_ = nullptr;
    QLabel *pmValue_ = nullptr;
    QLabel *co2Value_ = nullptr;
    QLabel *lastUpdateLabel_ = nullptr;
    QProgressBar *tempBar_ = nullptr;
    QProgressBar *humBar_ = nullptr;
    QProgressBar *pmBar_ = nullptr;
    QProgressBar *co2Bar_ = nullptr;
    QComboBox *intervalCombo_ = nullptr;
    QPushButton *toggleSimButton_ = nullptr;
};

#endif // REALTIMEPAGEWIDGET_H