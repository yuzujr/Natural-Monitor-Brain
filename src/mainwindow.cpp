#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QStyle>
#include <QStringList>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>

#include "common/uistyles.h"
#include "languageutils.h"
#include "pages/alarmpagewidget.h"
#include "pages/exportpagewidget.h"
#include "pages/historypagewidget.h"
#include "pages/realtimepagewidget.h"
#include "pages/settingspagewidget.h"
#include "pages/usermanagementpagewidget.h"
#include "repositories/alarmrepository.h"
#include "repositories/samplerepository.h"
#include "repositories/userrepository.h"
#include "services/alarmservice.h"
#include "services/csvexporter.h"
#include "services/udpsensorclient.h"
#include "themeutils.h"

namespace {
constexpr int kDefaultLimit = 1000;
}

MainWindow::MainWindow(DatabaseManager *db, ThemeManager *themeManager, LanguageManager *languageManager,
                       const UserInfo &user, QWidget *parent)
    : QMainWindow(parent)
    , db_(db)
    , userRepository_(db ? db->userRepository() : nullptr)
    , sampleRepository_(db ? db->sampleRepository() : nullptr)
    , alarmRepository_(db ? db->alarmRepository() : nullptr)
    , alarmService_(new AlarmService(alarmRepository_))
    , themeManager_(themeManager)
    , languageManager_(languageManager)
    , sensorClient_(new UdpSensorClient(this))
    , currentUser_(user)
{
    setWindowTitle(tr("环境数据信息平台"));
    resize(1200, 760);
    UiStyles::applyMainWindowStyle(this);

    centralWidget_ = new QWidget(this);
    setCentralWidget(centralWidget_);

    auto *centralLayout = new QVBoxLayout(centralWidget_);
    centralLayout->setContentsMargins(8, 8, 8, 8);
    centralLayout->setSpacing(12);

    tabs_ = new QTabWidget(centralWidget_);
    realtimePage_ = new RealtimePageWidget(this);
    historyPage_ = new HistoryPageWidget(this);
    alarmPage_ = new AlarmPageWidget(this);
    exportPage_ = new ExportPageWidget(this);
    settingsPage_ = new SettingsPageWidget(this);

    tabs_->addTab(realtimePage_, tr("实时数据"));
    tabs_->addTab(historyPage_, tr("历史查询"));
    tabs_->addTab(alarmPage_, tr("异常报警"));
    tabs_->addTab(exportPage_, tr("数据导出"));
    tabs_->addTab(settingsPage_, tr("系统设置"));
    if (currentUser_.role == QStringLiteral("admin")) {
        userPage_ = new UserManagementPageWidget(this);
        tabs_->addTab(userPage_, tr("用户管理"));
    }
    UiStyles::applyTabStyle(tabs_);
    centralLayout->addWidget(tabs_);

    connect(realtimePage_, &RealtimePageWidget::simulationToggleRequested, this, &MainWindow::handleToggleSimulation);
    connect(realtimePage_, &RealtimePageWidget::refreshIntervalChanged, this, &MainWindow::handleRefreshIntervalChanged);
    connect(historyPage_, &HistoryPageWidget::queryRequested, this, &MainWindow::handleHistoryQuery);
    connect(alarmPage_, &AlarmPageWidget::saveThresholdsRequested, this, &MainWindow::handleSaveThresholds);
    connect(alarmPage_, &AlarmPageWidget::refreshRequested, this, &MainWindow::handleRefreshAlarms);
    connect(alarmPage_, &AlarmPageWidget::clearRequested, this, &MainWindow::handleClearAlarms);
    connect(exportPage_, &ExportPageWidget::exportHistoryRequested, this, &MainWindow::handleExportHistory);
    connect(exportPage_, &ExportPageWidget::exportStatsRequested, this, &MainWindow::handleExportStats);
    connect(exportPage_, &ExportPageWidget::exportAlarmsRequested, this, &MainWindow::handleExportAlarms);
    connect(settingsPage_, &SettingsPageWidget::refreshIntervalChanged, this, &MainWindow::handleRefreshIntervalChanged);
    connect(settingsPage_, &SettingsPageWidget::themeModeChanged, this, &MainWindow::handleThemeModeChanged);
    connect(settingsPage_, &SettingsPageWidget::applyRequested, this, &MainWindow::handleApplySettings);
    connect(settingsPage_, &SettingsPageWidget::backupRequested, this, &MainWindow::handleBackupDatabase);
    connect(settingsPage_, &SettingsPageWidget::restoreRequested, this, &MainWindow::handleRestoreDatabase);
    connect(settingsPage_, &SettingsPageWidget::cleanupRequested, this, &MainWindow::handleCleanupData);
    connect(settingsPage_, &SettingsPageWidget::languageToggleRequested, this, &MainWindow::handleLanguageToggle);
    connect(settingsPage_, &SettingsPageWidget::reloadThemeRequested, this, [this]() {
        if (themeManager_) {
            themeManager_->reload();
        }
    });
    connect(sensorClient_, &UdpSensorClient::sampleReceived, this, &MainWindow::handleSensorSampleReceived);
    connect(sensorClient_, &UdpSensorClient::requestFailed, this, &MainWindow::handleSensorRequestFailed);

    if (userPage_) {
        connect(userPage_, &UserManagementPageWidget::addUserRequested, this, &MainWindow::handleUserAdd);
        connect(userPage_, &UserManagementPageWidget::deleteUserRequested, this, &MainWindow::handleUserDelete);
        connect(userPage_, &UserManagementPageWidget::toggleRoleRequested, this, &MainWindow::handleUserToggleRole);
        connect(userPage_, &UserManagementPageWidget::resetPasswordRequested, this, &MainWindow::handleUserResetPassword);
    }

    auto *fileMenu = menuBar()->addMenu(tr("文件"));
    fileMenu->addAction(tr("注销"), this, &MainWindow::handleLogout);
    fileMenu->addAction(tr("退出"), this, &QWidget::close);

    auto *helpMenu = menuBar()->addMenu(tr("帮助"));
    helpMenu->addAction(tr("关于"), this, [this]() {
        QMessageBox::about(this, tr("关于"), tr("环境数据信息平台\n基于Qt Widgets与SQLite实现"));
    });

    statusBar()->showMessage(tr("已登录: %1 (%2)").arg(currentUser_.username, currentUser_.role));

    dataTimer_ = new QTimer(this);
    connect(dataTimer_, &QTimer::timeout, this, &MainWindow::handleDataTick);

    loadSettings();
    setRefreshInterval(refreshIntervalMs_);
    if (themeManager_) {
        connect(themeManager_, &ThemeManager::themeChanged, this, &MainWindow::handleThemeUpdated);
    }
    handleThemeUpdated();

    if (userPage_) {
        updateUserTable();
    }

    handleHistoryQuery(historyPage_->startDateTime(), historyPage_->endDateTime());
    alarmPage_->resetTimeRangeToRecentWindow();
    handleRefreshAlarms(alarmPage_->startDateTime(), alarmPage_->endDateTime());

    dataTimer_->start(refreshIntervalMs_);
}

MainWindow::~MainWindow()
{
    delete alarmService_;
}

void MainWindow::handleDataTick()
{
    if (!simulationRunning_) {
        return;
    }

    sensorClient_->requestSample();
}

void MainWindow::handleSensorSampleReceived(const EnvSample &sample)
{
    if (!lastSensorError_.isEmpty()) {
        lastSensorError_.clear();
    }
    realtimePage_->setSourceText(tr("数据来源: 本地 UDP 服务 (127.0.0.1:8888)"));

    if (sampleRepository_) {
        sampleRepository_->insertSample(sample);
    }

    realtimePage_->updateSample(sample);
    realtimePage_->addSample(sample);

    const QList<AlarmEvent> events = alarmService_->evaluateSample(sample, alarmPage_->alarmSettings());
    showAlarmEvents(events);
    if (!events.isEmpty()) {
        alarmPage_->followEndTimeToNow();
        handleRefreshAlarms(alarmPage_->startDateTime(), alarmPage_->endDateTime());
    }
    lastSample_ = sample;
    statusBar()->showMessage(tr("已接收本地 UDP 数据: %1").arg(sample.ts.toString("HH:mm:ss")), 3000);
}

void MainWindow::handleSensorRequestFailed(const QString &message)
{
    if (message == lastSensorError_) {
        statusBar()->showMessage(message, 3000);
        return;
    }

    lastSensorError_ = message;
    realtimePage_->setSourceText(tr("数据来源: 本地 UDP 服务 (127.0.0.1:8888) [连接异常]"));
    statusBar()->showMessage(message, 5000);
}

void MainWindow::handleToggleSimulation()
{
    simulationRunning_ = !simulationRunning_;
    realtimePage_->setSimulationRunning(simulationRunning_);
}

void MainWindow::handleHistoryQuery(const QDateTime &start, const QDateTime &end)
{
    if (!sampleRepository_) {
        return;
    }

    if (start > end) {
        QMessageBox::warning(this, tr("提示"), tr("开始时间不能晚于结束时间"));
        return;
    }

    const QList<EnvSample> samples = sampleRepository_->querySamples(start, end, kDefaultLimit);
    const EnvStats stats = sampleRepository_->queryStats(start, end);
    historyPage_->setQueryResult(samples, stats, kDefaultLimit);
}

void MainWindow::handleRefreshAlarms(const QDateTime &start, const QDateTime &end)
{
    if (!alarmRepository_) {
        return;
    }

    if (start > end) {
        QMessageBox::warning(this, tr("提示"), tr("开始时间不能晚于结束时间"));
        return;
    }

    const QList<AlarmRecord> records = alarmRepository_->queryAlarms(start, end, kDefaultLimit);
    alarmPage_->setAlarmRecords(records);
}

void MainWindow::handleClearAlarms()
{
    if (!alarmRepository_) {
        return;
    }
    if (QMessageBox::question(this, tr("确认"), tr("确定要清空报警记录吗？")) != QMessageBox::Yes) {
        return;
    }
    if (!alarmRepository_->clearAlarms()) {
        QMessageBox::warning(this, tr("失败"), db_->lastError());
        return;
    }
    handleRefreshAlarms(alarmPage_->startDateTime(), alarmPage_->endDateTime());
}

void MainWindow::handleSaveThresholds()
{
    saveSettings();
    QMessageBox::information(this, tr("已保存"), tr("报警阈值已保存"));
}

void MainWindow::handleExportHistory(const QDateTime &start, const QDateTime &end, const SampleSelection &selection)
{
    if (!sampleRepository_) {
        return;
    }

    if (start > end) {
        QMessageBox::warning(this, tr("提示"), tr("开始时间不能晚于结束时间"));
        return;
    }

    const QList<EnvSample> samples = sampleRepository_->querySamples(start, end, kDefaultLimit);
    const QString path = QFileDialog::getSaveFileName(this, tr("导出历史数据"), QString(), tr("CSV 文件 (*.csv)"));
    if (path.isEmpty()) {
        return;
    }

    if (!CsvExporter::exportSamples(samples, path, selection)) {
        QMessageBox::warning(this, tr("导出失败"), tr("无法写入CSV文件"));
        return;
    }

    QMessageBox::information(this, tr("导出成功"), tr("历史数据已导出"));
}

void MainWindow::handleExportStats(const QDateTime &start, const QDateTime &end)
{
    if (!sampleRepository_) {
        return;
    }
    if (start > end) {
        QMessageBox::warning(this, tr("提示"), tr("开始时间不能晚于结束时间"));
        return;
    }

    const EnvStats stats = sampleRepository_->queryStats(start, end);
    const QString path = QFileDialog::getSaveFileName(this, tr("导出统计结果"), QString(), tr("CSV 文件 (*.csv)"));
    if (path.isEmpty()) {
        return;
    }

    if (!CsvExporter::exportStats(stats, path)) {
        QMessageBox::warning(this, tr("导出失败"), tr("无法写入CSV文件"));
        return;
    }

    QMessageBox::information(this, tr("导出成功"), tr("统计结果已导出"));
}

void MainWindow::handleExportAlarms(const QDateTime &start, const QDateTime &end)
{
    if (!alarmRepository_) {
        return;
    }
    if (start > end) {
        QMessageBox::warning(this, tr("提示"), tr("开始时间不能晚于结束时间"));
        return;
    }

    const QList<AlarmRecord> records = alarmRepository_->queryAlarms(start, end, kDefaultLimit);
    const QString path = QFileDialog::getSaveFileName(this, tr("导出报警记录"), QString(), tr("CSV 文件 (*.csv)"));
    if (path.isEmpty()) {
        return;
    }

    if (!CsvExporter::exportAlarms(records, path)) {
        QMessageBox::warning(this, tr("导出失败"), tr("无法写入CSV文件"));
        return;
    }

    QMessageBox::information(this, tr("导出成功"), tr("报警记录已导出"));
}

void MainWindow::handleApplySettings()
{
    saveSettings();
    updateThemeStatus();
    QMessageBox::information(this, tr("提示"), tr("设置已应用"));
}

void MainWindow::handleBackupDatabase()
{
    if (!userRepository_) {
        return;
    }
    const QString path = QFileDialog::getSaveFileName(this, tr("备份数据库"), QString(), tr("数据库文件 (*.db)"));
    if (path.isEmpty()) {
        return;
    }

    if (QFile::exists(path)) {
        QFile::remove(path);
    }

    if (!QFile::copy(db_->databasePath(), path)) {
        QMessageBox::warning(this, tr("失败"), tr("数据库备份失败"));
        return;
    }

    QMessageBox::information(this, tr("成功"), tr("数据库已备份"));
}

void MainWindow::handleRestoreDatabase()
{
    if (!db_) {
        return;
    }

    const bool wasRunning = dataTimer_ && dataTimer_->isActive();
    if (dataTimer_) {
        dataTimer_->stop();
    }

    const QString path = QFileDialog::getOpenFileName(this, tr("恢复数据库"), QString(), tr("数据库文件 (*.db)"));
    if (path.isEmpty()) {
        if (dataTimer_ && wasRunning) {
            dataTimer_->start(refreshIntervalMs_);
        }
        return;
    }

    if (QMessageBox::question(this, tr("确认"), tr("恢复数据库会覆盖现有数据，是否继续？")) != QMessageBox::Yes) {
        if (dataTimer_ && wasRunning) {
            dataTimer_->start(refreshIntervalMs_);
        }
        return;
    }

    db_->close();

    if (!QFile::remove(db_->databasePath())) {
        db_->reopen();
        if (dataTimer_ && wasRunning) {
            dataTimer_->start(refreshIntervalMs_);
        }
        QMessageBox::warning(this, tr("失败"), tr("无法删除原数据库"));
        return;
    }

    if (!QFile::copy(path, db_->databasePath())) {
        db_->reopen();
        if (dataTimer_ && wasRunning) {
            dataTimer_->start(refreshIntervalMs_);
        }
        QMessageBox::warning(this, tr("失败"), tr("数据库恢复失败"));
        return;
    }

    if (!db_->reopen()) {
        if (dataTimer_ && wasRunning) {
            dataTimer_->start(refreshIntervalMs_);
        }
        QMessageBox::warning(this, tr("失败"), tr("数据库恢复后重新连接失败"));
        return;
    }

    settingsPage_->setDatabasePath(db_->databasePath());
    if (dataTimer_ && wasRunning) {
        dataTimer_->start(refreshIntervalMs_);
    }
    QMessageBox::information(this, tr("成功"), tr("数据库已恢复"));
}

void MainWindow::handleLogout()
{
    if (QMessageBox::question(this, tr("确认注销"), tr("确定要注销当前账号并返回登录界面吗？")) != QMessageBox::Yes) {
        return;
    }

    if (dataTimer_) {
        dataTimer_->stop();
    }

    emit logoutRequested();
    close();
}

void MainWindow::handleRefreshIntervalChanged(int ms)
{
    setRefreshInterval(ms);
    saveSettings();
}

void MainWindow::handleLanguageToggle()
{
    if (!languageManager_) {
        return;
    }

    if (dataTimer_) {
        dataTimer_->stop();
    }

    languageManager_->toggleLanguage();
    emit relaunchRequested();
    close();
}

void MainWindow::handleThemeModeChanged(const QString &modeKey)
{
    if (!themeManager_) {
        return;
    }

    themeManager_->setThemeMode(ThemeManager::themeModeFromKey(modeKey));
    saveSettings();
}

void MainWindow::handleThemeUpdated()
{
    UiStyles::applyMainWindowStyle(this);
    UiStyles::applyTabStyle(tabs_);
    realtimePage_->refreshThemeStyles();
    UiStyles::applyPageStyle(historyPage_);
    UiStyles::applyPageStyle(alarmPage_);
    UiStyles::applyPageStyle(exportPage_);
    UiStyles::applyPageStyle(settingsPage_);
    if (userPage_) {
        UiStyles::applyPageStyle(userPage_);
    }
    applyMenuPalette();
    realtimePage_->update();
    updateThemeStatus();
}

void MainWindow::handleUserAdd(const QString &username, const QString &password, const QString &role)
{
    if (!db_) {
        return;
    }
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请输入用户名和密码"));
        return;
    }

    if (!userRepository_->addUser(username, password, role)) {
        QMessageBox::warning(this, tr("添加失败"), db_->lastError());
        return;
    }

    updateUserTable();
}

void MainWindow::handleUserDelete(int id)
{
    if (!userRepository_ || !userPage_) {
        return;
    }

    if (id == currentUser_.id) {
        QMessageBox::warning(this, tr("提示"), tr("不能删除当前登录用户"));
        return;
    }

    if (QMessageBox::question(this, tr("确认"), tr("确定删除该用户吗？")) != QMessageBox::Yes) {
        return;
    }

    if (!userRepository_->deleteUser(id)) {
        QMessageBox::warning(this, tr("失败"), db_->lastError());
        return;
    }

    updateUserTable();
}

void MainWindow::handleUserToggleRole(int id, const QString &role)
{
    if (!userRepository_ || !userPage_) {
        return;
    }

    const QString newRole = (role == QStringLiteral("admin")) ? QStringLiteral("user") : QStringLiteral("admin");
    if (!userRepository_->updateUserRole(id, newRole)) {
        QMessageBox::warning(this, tr("失败"), db_->lastError());
        return;
    }

    updateUserTable();
}

void MainWindow::handleUserResetPassword(int id, const QString &password)
{
    if (!userRepository_ || !userPage_) {
        return;
    }

    if (!userRepository_->setUserPassword(id, password)) {
        QMessageBox::warning(this, tr("失败"), db_->lastError());
        return;
    }

    QMessageBox::information(this, tr("成功"), tr("密码已更新"));
}

void MainWindow::handleCleanupData(int days)
{
    if (!sampleRepository_) {
        return;
    }

    if (QMessageBox::question(this, tr("确认"), tr("将删除%1天之前的历史数据，是否继续？").arg(days))
        != QMessageBox::Yes) {
        return;
    }

    const QDateTime cutoff = QDateTime::currentDateTime().addDays(-days);
    if (!sampleRepository_->deleteSamplesBefore(cutoff)) {
        QMessageBox::warning(this, tr("失败"), db_->lastError());
        return;
    }

    QMessageBox::information(this, tr("完成"), tr("历史数据已清理"));
}

void MainWindow::updateUserTable()
{
    if (!userRepository_ || !userPage_) {
        return;
    }

    userPage_->setUsers(userRepository_->listUsers());
}

void MainWindow::applyMenuPalette()
{
    if (!menuBar()) {
        return;
    }
    menuBar()->setAutoFillBackground(true);
    menuBar()->setPalette(QApplication::palette());

    const QString menuStyle = QStringLiteral(
        "QMenuBar { background-color: palette(window); color: palette(windowText); }"
        "QMenuBar::item:selected { background-color: palette(highlight); color: palette(highlightedText); }"
        "QMenu { background-color: palette(window); color: palette(windowText); }"
        "QMenu::item:selected { background-color: palette(highlight); color: palette(highlightedText); }"
    );
    menuBar()->setStyleSheet(menuStyle);
}

void MainWindow::updateThemeStatus()
{
    if (!settingsPage_) {
        return;
    }
    if (!themeManager_) {
        settingsPage_->setThemeMode(QStringLiteral("system"));
        settingsPage_->setThemeStatus(tr("跟随系统默认主题"));
        return;
    }

    settingsPage_->setThemeMode(themeManager_->themeModeKey());
    const QString modeName = ThemeManager::themeModeDisplayName(themeManager_->themeMode());
    const QString schemeName = themeManager_->currentSchemeName().isEmpty()
        ? tr("未检测到")
        : themeManager_->currentSchemeName();
    const QString styleName = themeManager_->currentStyleName().isEmpty()
        ? QApplication::style()->objectName()
        : themeManager_->currentStyleName();
    const QString statusText = tr("%1，当前效果: %2，样式: %3")
        .arg(modeName, schemeName, styleName);
    settingsPage_->setThemeStatus(statusText);
    if (languageManager_) {
        settingsPage_->setLanguageInfo(languageManager_->currentLanguageDisplayName(),
                                       languageManager_->nextLanguageButtonText());
    }
}

void MainWindow::loadSettings()
{
    QSettings settings;
    refreshIntervalMs_ = settings.value(QStringLiteral("refresh_interval_ms"), 1000).toInt();
    const QString themeModeKey = settings.value(QStringLiteral("theme_mode"), QStringLiteral("system")).toString();

    AlarmSettings alarmSettings;
    alarmSettings.temperatureThreshold = settings.value(QStringLiteral("thresholds/temperature"), 35.0).toDouble();
    alarmSettings.humidityThreshold = settings.value(QStringLiteral("thresholds/humidity"), 85.0).toDouble();
    alarmSettings.pm25Threshold = settings.value(QStringLiteral("thresholds/pm25"), 150.0).toDouble();
    alarmSettings.co2Threshold = settings.value(QStringLiteral("thresholds/co2"), 1200.0).toDouble();
    alarmSettings.enabled = settings.value(QStringLiteral("alarm_enabled"), true).toBool();
    alarmSettings.cooldownSeconds = settings.value(QStringLiteral("alarm_cooldown_sec"), 30).toInt();

    alarmPage_->setAlarmSettings(alarmSettings);
    realtimePage_->setRefreshInterval(refreshIntervalMs_);
    realtimePage_->setSimulationRunning(simulationRunning_);
    settingsPage_->setRefreshInterval(refreshIntervalMs_);
    settingsPage_->setSoundMode(settings.value(QStringLiteral("alarm_sound"), QStringLiteral("beep")).toString());
    settingsPage_->setThemeMode(themeModeKey);
    if (themeManager_) {
        themeManager_->setThemeMode(ThemeManager::themeModeFromKey(themeModeKey));
    }
    if (db_) {
        settingsPage_->setDatabasePath(db_->databasePath());
    }
}

void MainWindow::saveSettings()
{
    const AlarmSettings alarmSettings = alarmPage_->alarmSettings();

    QSettings settings;
    settings.setValue(QStringLiteral("refresh_interval_ms"), refreshIntervalMs_);
    settings.setValue(QStringLiteral("alarm_sound"), settingsPage_->soundMode());
    settings.setValue(QStringLiteral("theme_mode"), settingsPage_->themeMode());
    settings.setValue(QStringLiteral("thresholds/temperature"), alarmSettings.temperatureThreshold);
    settings.setValue(QStringLiteral("thresholds/humidity"), alarmSettings.humidityThreshold);
    settings.setValue(QStringLiteral("thresholds/pm25"), alarmSettings.pm25Threshold);
    settings.setValue(QStringLiteral("thresholds/co2"), alarmSettings.co2Threshold);
    settings.setValue(QStringLiteral("alarm_enabled"), alarmSettings.enabled);
    settings.setValue(QStringLiteral("alarm_cooldown_sec"), alarmSettings.cooldownSeconds);
}

void MainWindow::setRefreshInterval(int ms)
{
    refreshIntervalMs_ = ms;
    if (dataTimer_) {
        dataTimer_->setInterval(refreshIntervalMs_);
    }

    realtimePage_->setRefreshInterval(ms);
    settingsPage_->setRefreshInterval(ms);
}

void MainWindow::showAlarmEvents(const QList<AlarmEvent> &events)
{
    if (events.isEmpty()) {
        return;
    }

    if (currentAlarmBox_) {
        currentAlarmBox_->close();
        currentAlarmBox_->deleteLater();
        currentAlarmBox_.clear();
    }

    if (settingsPage_->soundMode() == QStringLiteral("beep")) {
        QApplication::beep();
    }

    QStringList lines;
    for (const AlarmEvent &event : events) {
        lines << tr("[%1] %2 超过阈值: 当前值 %3，阈值 %4")
                     .arg(event.ts.toString("yyyy-MM-dd HH:mm:ss"))
                     .arg(event.param)
                     .arg(QString::number(event.value, 'f', 1))
                     .arg(QString::number(event.threshold, 'f', 1));
    }

    const QString title = events.size() > 1 ? tr("多指标预警报告") : tr("预警报告");
    auto *box = new QMessageBox(QMessageBox::Warning,
                                title,
                                tr("本次检测发现 %1 项异常：\n%2").arg(events.size()).arg(lines.join('\n')),
                                QMessageBox::Ok,
                                this);
    box->setAttribute(Qt::WA_DeleteOnClose);
    currentAlarmBox_ = box;
    connect(box, &QObject::destroyed, this, [this, box]() {
        if (currentAlarmBox_ == box) {
            currentAlarmBox_.clear();
        }
    });
    box->show();
}