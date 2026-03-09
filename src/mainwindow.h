#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QMainWindow>
#include <QPointer>

#include "common/moduletypes.h"
#include "databasemanager.h"

class QWidget;

class AlarmPageWidget;
class ExportPageWidget;
class HistoryPageWidget;
class AlarmRepository;
class AlarmService;
class QMessageBox;
class QTabWidget;
class QTimer;
class RealtimePageWidget;
class SampleRepository;
class SettingsPageWidget;
class ThemeManager;
class UdpSensorClient;
class UserRepository;
class UserManagementPageWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(DatabaseManager *db, ThemeManager *themeManager, const UserInfo &user, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void logoutRequested();

private slots:
    void handleDataTick();
    void handleToggleSimulation();
    void handleHistoryQuery(const QDateTime &start, const QDateTime &end);
    void handleRefreshAlarms(const QDateTime &start, const QDateTime &end);
    void handleClearAlarms();
    void handleSaveThresholds();
    void handleExportHistory(const QDateTime &start, const QDateTime &end, const SampleSelection &selection);
    void handleExportStats(const QDateTime &start, const QDateTime &end);
    void handleExportAlarms(const QDateTime &start, const QDateTime &end);
    void handleApplySettings();
    void handleBackupDatabase();
    void handleRestoreDatabase();
    void handleLogout();
    void handleRefreshIntervalChanged(int ms);
    void handleSensorSampleReceived(const EnvSample &sample);
    void handleSensorRequestFailed(const QString &message);
    void handleThemeModeChanged(const QString &modeKey);
    void handleThemeUpdated();
    void handleUserAdd(const QString &username, const QString &password, const QString &role);
    void handleUserDelete(int id);
    void handleUserToggleRole(int id, const QString &role);
    void handleUserResetPassword(int id, const QString &password);
    void handleCleanupData(int days);

private:
    void updateUserTable();
    void updateThemeStatus();
    void applyMenuPalette();
    void loadSettings();
    void saveSettings();
    void setRefreshInterval(int ms);
    void showAlarmEvents(const QList<AlarmEvent> &events);

    QString lastSensorError_;

    DatabaseManager *db_ = nullptr;
    UserRepository *userRepository_ = nullptr;
    SampleRepository *sampleRepository_ = nullptr;
    AlarmRepository *alarmRepository_ = nullptr;
    AlarmService *alarmService_ = nullptr;
    ThemeManager *themeManager_ = nullptr;
    UdpSensorClient *sensorClient_ = nullptr;
    UserInfo currentUser_;

    QWidget *centralWidget_ = nullptr;
    QTabWidget *tabs_ = nullptr;
    RealtimePageWidget *realtimePage_ = nullptr;
    HistoryPageWidget *historyPage_ = nullptr;
    AlarmPageWidget *alarmPage_ = nullptr;
    ExportPageWidget *exportPage_ = nullptr;
    SettingsPageWidget *settingsPage_ = nullptr;
    UserManagementPageWidget *userPage_ = nullptr;

    QTimer *dataTimer_ = nullptr;
    QPointer<QMessageBox> currentAlarmBox_;
    bool simulationRunning_ = true;
    EnvSample lastSample_;
    int refreshIntervalMs_ = 1000;
};

#endif // MAINWINDOW_H