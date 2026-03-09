#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>

#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class LineChartWidget;
class ThemeManager;
class QCheckBox;
class QComboBox;
class QDateTimeEdit;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QFileSystemWatcher;
class QSpinBox;
class QTableWidget;
class QTabWidget;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(DatabaseManager *db, ThemeManager *themeManager, const UserInfo &user, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleDataTick();
    void handleToggleSimulation();
    void handleHistoryQuery();
    void handleRefreshAlarms();
    void handleClearAlarms();
    void handleSaveThresholds();
    void handleExportHistory();
    void handleExportStats();
    void handleExportAlarms();
    void handleApplySettings();
    void handleBackupDatabase();
    void handleRestoreDatabase();
    void handleIntervalComboChanged(int index);
    void handleThemeUpdated();
    void handleUserAdd();
    void handleUserDelete();
    void handleUserToggleRole();
    void handleUserResetPassword();
    void handleCleanupData();

private:
    QWidget *createRealtimePage();
    QWidget *createHistoryPage();
    QWidget *createAlarmPage();
    QWidget *createExportPage();
    QWidget *createSettingsPage();
    QWidget *createUserPage();

    void updateRealtimeUi(const EnvSample &sample);
    void updateHistoryTable(const QList<EnvSample> &samples);
    void updateStatsTable(const EnvStats &stats);
    void updateAlarmTable(const QList<AlarmRecord> &records);
    void updateUserTable();

    void updateThemeStatus();
    void applyMenuPalette();
    void loadSettings();
    void saveSettings();
    void setRefreshInterval(int ms);

    EnvSample generateSample();
    void checkAlarms(const EnvSample &sample);
    bool exportSamplesToCsv(const QList<EnvSample> &samples, const QString &path,
                             bool useTemp, bool useHum, bool usePm, bool useCo2);
    bool exportAlarmToCsv(const QList<AlarmRecord> &records, const QString &path);
    bool exportStatsToCsv(const EnvStats &stats, const QString &path);

    Ui::MainWindow *ui = nullptr;
    DatabaseManager *db_ = nullptr;
    ThemeManager *themeManager_ = nullptr;
    UserInfo currentUser_;

    QTabWidget *tabs_ = nullptr;

    QTimer *dataTimer_ = nullptr;
    bool simulationRunning_ = true;
    EnvSample lastSample_;

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
    QComboBox *realtimeIntervalCombo_ = nullptr;
    QPushButton *toggleSimButton_ = nullptr;

    QDateTimeEdit *historyStart_ = nullptr;
    QDateTimeEdit *historyEnd_ = nullptr;
    QCheckBox *historyTempCheck_ = nullptr;
    QCheckBox *historyHumCheck_ = nullptr;
    QCheckBox *historyPmCheck_ = nullptr;
    QCheckBox *historyCo2Check_ = nullptr;
    QTableWidget *historyTable_ = nullptr;
    QTableWidget *statsTable_ = nullptr;
    QLabel *historyCountLabel_ = nullptr;

    QCheckBox *alarmEnabledCheck_ = nullptr;
    QSpinBox *alarmCooldownSpin_ = nullptr;
    QDoubleSpinBox *tempThreshold_ = nullptr;
    QDoubleSpinBox *humThreshold_ = nullptr;
    QDoubleSpinBox *pmThreshold_ = nullptr;
    QDoubleSpinBox *co2Threshold_ = nullptr;
    QDateTimeEdit *alarmStart_ = nullptr;
    QDateTimeEdit *alarmEnd_ = nullptr;
    QTableWidget *alarmTable_ = nullptr;

    QDateTime lastAlarmTemp_;
    QDateTime lastAlarmHum_;
    QDateTime lastAlarmPm_;
    QDateTime lastAlarmCo2_;

    QDateTimeEdit *exportStart_ = nullptr;
    QDateTimeEdit *exportEnd_ = nullptr;
    QCheckBox *exportTempCheck_ = nullptr;
    QCheckBox *exportHumCheck_ = nullptr;
    QCheckBox *exportPmCheck_ = nullptr;
    QCheckBox *exportCo2Check_ = nullptr;
    QDateTimeEdit *exportAlarmStart_ = nullptr;
    QDateTimeEdit *exportAlarmEnd_ = nullptr;

    QComboBox *settingsIntervalCombo_ = nullptr;
    QComboBox *soundCombo_ = nullptr;
    QLabel *themeStatusLabel_ = nullptr;
    QLabel *dbPathLabel_ = nullptr;
    QSpinBox *cleanupDaysSpin_ = nullptr;

    QTableWidget *userTable_ = nullptr;
    QLineEdit *newUserName_ = nullptr;
    QLineEdit *newUserPass_ = nullptr;
    QComboBox *newUserRole_ = nullptr;

    int refreshIntervalMs_ = 1000;
};

#endif // MAINWINDOW_H
