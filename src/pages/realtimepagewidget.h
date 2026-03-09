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
    void setSourceText(const QString &text);
    void updateSample(const EnvSample &sample);
    void addSample(const EnvSample &sample);
    void refreshThemeStyles();

signals:
    void refreshIntervalChanged(int ms);
    void simulationToggleRequested();

private:
    void applyMetricCardStyle(class QFrame *card, class QLabel *nameLabel, class QLabel *valueLabel,
                              class QProgressBar *bar, const QString &accentColor);

    LineChartWidget *chart_ = nullptr;
    class QFrame *headerCard_ = nullptr;
    class QFrame *tempCard_ = nullptr;
    class QFrame *humCard_ = nullptr;
    class QFrame *pmCard_ = nullptr;
    class QFrame *co2Card_ = nullptr;
    QLabel *titleLabel_ = nullptr;
    QLabel *subtitleLabel_ = nullptr;
    QLabel *sourceLabel_ = nullptr;
    QLabel *tempTitleLabel_ = nullptr;
    QLabel *humTitleLabel_ = nullptr;
    QLabel *pmTitleLabel_ = nullptr;
    QLabel *co2TitleLabel_ = nullptr;
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