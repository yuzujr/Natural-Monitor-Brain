#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "linechartwidget.h"
#include "themeutils.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QFont>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSettings>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QTabWidget>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QtGlobal>
#include <QStandardPaths>
#include <QDir>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace {
constexpr int kDefaultLimit = 1000;
constexpr int kDefaultMaxPoints = 60;

void setUtf8(QTextStream &out)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
}
}

MainWindow::MainWindow(DatabaseManager *db, ThemeManager *themeManager, const UserInfo &user, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , db_(db)
    , themeManager_(themeManager)
    , currentUser_(user)
{
    ui->setupUi(this);
    setWindowTitle(tr("环境数据信息平台"));
    resize(1200, 760);

    auto *centralLayout = new QVBoxLayout(ui->centralwidget);
    centralLayout->setContentsMargins(8, 8, 8, 8);

    tabs_ = new QTabWidget(ui->centralwidget);
    tabs_->addTab(createRealtimePage(), tr("实时数据"));
    tabs_->addTab(createHistoryPage(), tr("历史查询"));
    tabs_->addTab(createAlarmPage(), tr("异常报警"));
    tabs_->addTab(createExportPage(), tr("数据导出"));
    tabs_->addTab(createSettingsPage(), tr("系统设置"));
    if (currentUser_.role == QStringLiteral("admin")) {
        tabs_->addTab(createUserPage(), tr("用户管理"));
    }

    centralLayout->addWidget(tabs_);

    auto *fileMenu = menuBar()->addMenu(tr("文件"));
    fileMenu->addAction(tr("退出"), this, &QWidget::close);

    auto *helpMenu = menuBar()->addMenu(tr("帮助"));
    helpMenu->addAction(tr("关于"), this, [this]() {
        QMessageBox::about(this, tr("关于"),
                           tr("环境数据信息平台\n基于Qt Widgets与SQLite实现"));
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

    dataTimer_->start(refreshIntervalMs_);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QWidget *MainWindow::createRealtimePage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *toolbar = new QHBoxLayout();
    auto *sourceLabel = new QLabel(tr("数据来源: 模拟传感器"), page);
    sourceLabel->setStyleSheet(QStringLiteral("color: #666;"));

    toggleSimButton_ = new QPushButton(tr("暂停采集"), page);
    connect(toggleSimButton_, &QPushButton::clicked, this, &MainWindow::handleToggleSimulation);

    realtimeIntervalCombo_ = new QComboBox(page);
    realtimeIntervalCombo_->addItem(tr("1 秒"), 1000);
    realtimeIntervalCombo_->addItem(tr("5 秒"), 5000);
    realtimeIntervalCombo_->addItem(tr("10 秒"), 10000);
    connect(realtimeIntervalCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::handleIntervalComboChanged);

    toolbar->addWidget(sourceLabel);
    toolbar->addStretch();
    toolbar->addWidget(new QLabel(tr("刷新频率"), page));
    toolbar->addWidget(realtimeIntervalCombo_);
    toolbar->addWidget(toggleSimButton_);
    layout->addLayout(toolbar);

    auto *dataGroup = new QGroupBox(tr("当前环境数据"), page);
    auto *grid = new QGridLayout(dataGroup);

    auto createRow = [&](int row, const QString &name, QLabel *&valueLabel, QProgressBar *&bar, int min, int max) {
        auto *nameLabel = new QLabel(name, dataGroup);
        valueLabel = new QLabel("--", dataGroup);
        valueLabel->setStyleSheet(QStringLiteral("font-size: 20px; font-weight: 600;"));
        bar = new QProgressBar(dataGroup);
        bar->setRange(min, max);
        bar->setTextVisible(false);
        grid->addWidget(nameLabel, row, 0);
        grid->addWidget(valueLabel, row, 1);
        grid->addWidget(bar, row, 2);
    };

    createRow(0, tr("温度 (C)"), tempValue_, tempBar_, 0, 400);
    createRow(1, tr("湿度 (%)"), humValue_, humBar_, 0, 100);
    createRow(2, tr("PM2.5 (ug/m3)"), pmValue_, pmBar_, 0, 200);
    createRow(3, tr("CO2 (ppm)"), co2Value_, co2Bar_, 300, 2000);

    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(2, 3);
    layout->addWidget(dataGroup);

    chart_ = new LineChartWidget(page);
    chart_->setMaxPoints(kDefaultMaxPoints);
    auto *chartGroup = new QGroupBox(tr("趋势概览"), page);
    auto *chartLayout = new QVBoxLayout(chartGroup);
    chartLayout->addWidget(chart_);
    layout->addWidget(chartGroup);

    lastUpdateLabel_ = new QLabel(tr("最近更新时间: --"), page);
    lastUpdateLabel_->setStyleSheet(QStringLiteral("color: #666;"));
    layout->addWidget(lastUpdateLabel_);

    layout->addStretch();
    return page;
}

QWidget *MainWindow::createHistoryPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *filterGroup = new QGroupBox(tr("查询条件"), page);
    auto *filterLayout = new QGridLayout(filterGroup);

    historyStart_ = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), filterGroup);
    historyEnd_ = new QDateTimeEdit(QDateTime::currentDateTime(), filterGroup);
    historyStart_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    historyEnd_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    historyStart_->setCalendarPopup(true);
    historyEnd_->setCalendarPopup(true);

    historyTempCheck_ = new QCheckBox(tr("温度"), filterGroup);
    historyHumCheck_ = new QCheckBox(tr("湿度"), filterGroup);
    historyPmCheck_ = new QCheckBox(tr("PM2.5"), filterGroup);
    historyCo2Check_ = new QCheckBox(tr("CO2"), filterGroup);
    historyTempCheck_->setChecked(true);
    historyHumCheck_->setChecked(true);
    historyPmCheck_->setChecked(true);
    historyCo2Check_->setChecked(true);

    auto *queryButton = new QPushButton(tr("查询"), filterGroup);
    connect(queryButton, &QPushButton::clicked, this, &MainWindow::handleHistoryQuery);

    filterLayout->addWidget(new QLabel(tr("开始时间"), filterGroup), 0, 0);
    filterLayout->addWidget(historyStart_, 0, 1);
    filterLayout->addWidget(new QLabel(tr("结束时间"), filterGroup), 0, 2);
    filterLayout->addWidget(historyEnd_, 0, 3);

    filterLayout->addWidget(new QLabel(tr("参数"), filterGroup), 1, 0);
    auto *paramBox = new QWidget(filterGroup);
    auto *paramLayout = new QHBoxLayout(paramBox);
    paramLayout->setContentsMargins(0, 0, 0, 0);
    paramLayout->addWidget(historyTempCheck_);
    paramLayout->addWidget(historyHumCheck_);
    paramLayout->addWidget(historyPmCheck_);
    paramLayout->addWidget(historyCo2Check_);
    paramLayout->addStretch();
    filterLayout->addWidget(paramBox, 1, 1, 1, 3);

    filterLayout->addWidget(queryButton, 0, 4, 2, 1);

    layout->addWidget(filterGroup);

    historyTable_ = new QTableWidget(page);
    historyTable_->setColumnCount(5);
    historyTable_->setHorizontalHeaderLabels({
        tr("时间"), tr("温度"), tr("湿度"), tr("PM2.5"), tr("CO2")
    });
    historyTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    historyTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    historyTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(historyTable_);

    auto *statsGroup = new QGroupBox(tr("统计结果"), page);
    auto *statsLayout = new QVBoxLayout(statsGroup);
    statsTable_ = new QTableWidget(statsGroup);
    statsTable_->setRowCount(4);
    statsTable_->setColumnCount(3);
    statsTable_->setHorizontalHeaderLabels({tr("最小值"), tr("最大值"), tr("平均值")});
    statsTable_->setVerticalHeaderLabels({tr("温度"), tr("湿度"), tr("PM2.5"), tr("CO2")});
    statsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    statsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsLayout->addWidget(statsTable_);
    layout->addWidget(statsGroup);

    historyCountLabel_ = new QLabel(tr("查询记录: 0"), page);
    historyCountLabel_->setStyleSheet(QStringLiteral("color: #666;"));
    layout->addWidget(historyCountLabel_);

    return page;
}

QWidget *MainWindow::createAlarmPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *thresholdGroup = new QGroupBox(tr("阈值设置"), page);
    auto *thresholdLayout = new QGridLayout(thresholdGroup);

    auto makeThreshold = [&](int row, const QString &label, QDoubleSpinBox *&spin, double min, double max, const QString &suffix) {
        spin = new QDoubleSpinBox(thresholdGroup);
        spin->setRange(min, max);
        spin->setDecimals(1);
        spin->setSuffix(" " + suffix);
        thresholdLayout->addWidget(new QLabel(label, thresholdGroup), row, 0);
        thresholdLayout->addWidget(spin, row, 1);
    };

    makeThreshold(0, tr("温度报警"), tempThreshold_, 0, 60, tr("C"));
    makeThreshold(1, tr("湿度报警"), humThreshold_, 0, 100, tr("%"));
    makeThreshold(2, tr("PM2.5报警"), pmThreshold_, 0, 500, tr("ug/m3"));
    makeThreshold(3, tr("CO2报警"), co2Threshold_, 300, 5000, tr("ppm"));

    alarmEnabledCheck_ = new QCheckBox(tr("启用报警"), thresholdGroup);
    thresholdLayout->addWidget(alarmEnabledCheck_, 0, 2);

    alarmCooldownSpin_ = new QSpinBox(thresholdGroup);
    alarmCooldownSpin_->setRange(5, 600);
    alarmCooldownSpin_->setSuffix(tr(" 秒"));
    thresholdLayout->addWidget(new QLabel(tr("报警间隔"), thresholdGroup), 1, 2);
    thresholdLayout->addWidget(alarmCooldownSpin_, 1, 3);

    auto *saveThresholdBtn = new QPushButton(tr("保存设置"), thresholdGroup);
    connect(saveThresholdBtn, &QPushButton::clicked, this, &MainWindow::handleSaveThresholds);
    thresholdLayout->addWidget(saveThresholdBtn, 2, 2, 1, 2);

    layout->addWidget(thresholdGroup);

    auto *logGroup = new QGroupBox(tr("报警记录"), page);
    auto *logLayout = new QVBoxLayout(logGroup);

    auto *logFilter = new QHBoxLayout();
    alarmStart_ = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), logGroup);
    alarmEnd_ = new QDateTimeEdit(QDateTime::currentDateTime(), logGroup);
    alarmStart_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    alarmEnd_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    alarmStart_->setCalendarPopup(true);
    alarmEnd_->setCalendarPopup(true);

    auto *refreshBtn = new QPushButton(tr("刷新"), logGroup);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::handleRefreshAlarms);
    auto *clearBtn = new QPushButton(tr("清空"), logGroup);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::handleClearAlarms);

    logFilter->addWidget(new QLabel(tr("开始"), logGroup));
    logFilter->addWidget(alarmStart_);
    logFilter->addWidget(new QLabel(tr("结束"), logGroup));
    logFilter->addWidget(alarmEnd_);
    logFilter->addStretch();
    logFilter->addWidget(refreshBtn);
    logFilter->addWidget(clearBtn);

    logLayout->addLayout(logFilter);

    alarmTable_ = new QTableWidget(logGroup);
    alarmTable_->setColumnCount(4);
    alarmTable_->setHorizontalHeaderLabels({tr("时间"), tr("参数"), tr("值"), tr("阈值")});
    alarmTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    alarmTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    alarmTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    logLayout->addWidget(alarmTable_);

    layout->addWidget(logGroup);
    return page;
}

QWidget *MainWindow::createExportPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *historyGroup = new QGroupBox(tr("导出历史数据"), page);
    auto *historyLayout = new QGridLayout(historyGroup);
    exportStart_ = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), historyGroup);
    exportEnd_ = new QDateTimeEdit(QDateTime::currentDateTime(), historyGroup);
    exportStart_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    exportEnd_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    exportStart_->setCalendarPopup(true);
    exportEnd_->setCalendarPopup(true);

    exportTempCheck_ = new QCheckBox(tr("温度"), historyGroup);
    exportHumCheck_ = new QCheckBox(tr("湿度"), historyGroup);
    exportPmCheck_ = new QCheckBox(tr("PM2.5"), historyGroup);
    exportCo2Check_ = new QCheckBox(tr("CO2"), historyGroup);
    exportTempCheck_->setChecked(true);
    exportHumCheck_->setChecked(true);
    exportPmCheck_->setChecked(true);
    exportCo2Check_->setChecked(true);

    auto *exportHistoryBtn = new QPushButton(tr("导出CSV"), historyGroup);
    connect(exportHistoryBtn, &QPushButton::clicked, this, &MainWindow::handleExportHistory);

    historyLayout->addWidget(new QLabel(tr("开始时间"), historyGroup), 0, 0);
    historyLayout->addWidget(exportStart_, 0, 1);
    historyLayout->addWidget(new QLabel(tr("结束时间"), historyGroup), 0, 2);
    historyLayout->addWidget(exportEnd_, 0, 3);

    auto *exportParamBox = new QWidget(historyGroup);
    auto *exportParamLayout = new QHBoxLayout(exportParamBox);
    exportParamLayout->setContentsMargins(0, 0, 0, 0);
    exportParamLayout->addWidget(exportTempCheck_);
    exportParamLayout->addWidget(exportHumCheck_);
    exportParamLayout->addWidget(exportPmCheck_);
    exportParamLayout->addWidget(exportCo2Check_);
    exportParamLayout->addStretch();
    historyLayout->addWidget(new QLabel(tr("导出参数"), historyGroup), 1, 0);
    historyLayout->addWidget(exportParamBox, 1, 1, 1, 3);

    historyLayout->addWidget(exportHistoryBtn, 0, 4, 2, 1);
    layout->addWidget(historyGroup);

    auto *statsGroup = new QGroupBox(tr("导出统计结果"), page);
    auto *statsLayout = new QHBoxLayout(statsGroup);
    auto *exportStatsBtn = new QPushButton(tr("导出统计CSV"), statsGroup);
    connect(exportStatsBtn, &QPushButton::clicked, this, &MainWindow::handleExportStats);
    statsLayout->addWidget(new QLabel(tr("使用上方时间范围"), statsGroup));
    statsLayout->addStretch();
    statsLayout->addWidget(exportStatsBtn);
    layout->addWidget(statsGroup);

    auto *alarmGroup = new QGroupBox(tr("导出报警记录"), page);
    auto *alarmLayout = new QGridLayout(alarmGroup);
    exportAlarmStart_ = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), alarmGroup);
    exportAlarmEnd_ = new QDateTimeEdit(QDateTime::currentDateTime(), alarmGroup);
    exportAlarmStart_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    exportAlarmEnd_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    exportAlarmStart_->setCalendarPopup(true);
    exportAlarmEnd_->setCalendarPopup(true);

    auto *exportAlarmBtn = new QPushButton(tr("导出CSV"), alarmGroup);
    connect(exportAlarmBtn, &QPushButton::clicked, this, &MainWindow::handleExportAlarms);

    alarmLayout->addWidget(new QLabel(tr("开始时间"), alarmGroup), 0, 0);
    alarmLayout->addWidget(exportAlarmStart_, 0, 1);
    alarmLayout->addWidget(new QLabel(tr("结束时间"), alarmGroup), 0, 2);
    alarmLayout->addWidget(exportAlarmEnd_, 0, 3);
    alarmLayout->addWidget(exportAlarmBtn, 0, 4);

    layout->addWidget(alarmGroup);
    layout->addStretch();

    return page;
}

QWidget *MainWindow::createSettingsPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *settingsGroup = new QGroupBox(tr("系统设置"), page);
    auto *settingsLayout = new QGridLayout(settingsGroup);

    settingsIntervalCombo_ = new QComboBox(settingsGroup);
    settingsIntervalCombo_->addItem(tr("1 秒"), 1000);
    settingsIntervalCombo_->addItem(tr("5 秒"), 5000);
    settingsIntervalCombo_->addItem(tr("10 秒"), 10000);
    connect(settingsIntervalCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::handleIntervalComboChanged);

    soundCombo_ = new QComboBox(settingsGroup);
    soundCombo_->addItem(tr("提示音"), QStringLiteral("beep"));
    soundCombo_->addItem(tr("静音"), QStringLiteral("mute"));

    settingsLayout->addWidget(new QLabel(tr("刷新频率"), settingsGroup), 0, 0);
    settingsLayout->addWidget(settingsIntervalCombo_, 0, 1);
    settingsLayout->addWidget(new QLabel(tr("主题"), settingsGroup), 1, 0);
    themeStatusLabel_ = new QLabel(tr("跟随系统主题 (qt6ct)"), settingsGroup);
    themeStatusLabel_->setStyleSheet(QStringLiteral("color: #666;"));
    settingsLayout->addWidget(themeStatusLabel_, 1, 1);
    auto *reloadThemeButton = new QPushButton(tr("重新加载主题"), settingsGroup);
    connect(reloadThemeButton, &QPushButton::clicked, this, [this]() {
        if (themeManager_) {
            themeManager_->reload();
        }
    });
    settingsLayout->addWidget(reloadThemeButton, 1, 2);
    settingsLayout->addWidget(new QLabel(tr("报警提示"), settingsGroup), 2, 0);
    settingsLayout->addWidget(soundCombo_, 2, 1);

    auto *applyButton = new QPushButton(tr("应用设置"), settingsGroup);
    connect(applyButton, &QPushButton::clicked, this, &MainWindow::handleApplySettings);
    settingsLayout->addWidget(applyButton, 3, 0, 1, 3);

    layout->addWidget(settingsGroup);

    auto *dbGroup = new QGroupBox(tr("数据库维护"), page);
    auto *dbLayout = new QGridLayout(dbGroup);
    dbPathLabel_ = new QLabel(dbGroup);
    dbPathLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto *backupBtn = new QPushButton(tr("备份数据库"), dbGroup);
    auto *restoreBtn = new QPushButton(tr("恢复数据库"), dbGroup);
    connect(backupBtn, &QPushButton::clicked, this, &MainWindow::handleBackupDatabase);
    connect(restoreBtn, &QPushButton::clicked, this, &MainWindow::handleRestoreDatabase);

    cleanupDaysSpin_ = new QSpinBox(dbGroup);
    cleanupDaysSpin_->setRange(1, 365);
    cleanupDaysSpin_->setValue(30);
    cleanupDaysSpin_->setSuffix(tr(" 天"));
    auto *cleanupBtn = new QPushButton(tr("清理历史数据"), dbGroup);
    connect(cleanupBtn, &QPushButton::clicked, this, &MainWindow::handleCleanupData);

    dbLayout->addWidget(new QLabel(tr("数据库路径"), dbGroup), 0, 0);
    dbLayout->addWidget(dbPathLabel_, 0, 1, 1, 3);
    dbLayout->addWidget(backupBtn, 1, 0);
    dbLayout->addWidget(restoreBtn, 1, 1);
    dbLayout->addWidget(new QLabel(tr("保留周期"), dbGroup), 2, 0);
    dbLayout->addWidget(cleanupDaysSpin_, 2, 1);
    dbLayout->addWidget(cleanupBtn, 2, 2);

    layout->addWidget(dbGroup);
    layout->addStretch();

    return page;
}

QWidget *MainWindow::createUserPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);

    auto *addGroup = new QGroupBox(tr("添加用户"), page);
    auto *addLayout = new QGridLayout(addGroup);

    newUserName_ = new QLineEdit(addGroup);
    newUserPass_ = new QLineEdit(addGroup);
    newUserPass_->setEchoMode(QLineEdit::Password);
    newUserRole_ = new QComboBox(addGroup);
    newUserRole_->addItem(tr("普通用户"), QStringLiteral("user"));
    newUserRole_->addItem(tr("管理员"), QStringLiteral("admin"));
    auto *addBtn = new QPushButton(tr("添加"), addGroup);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::handleUserAdd);

    addLayout->addWidget(new QLabel(tr("用户名"), addGroup), 0, 0);
    addLayout->addWidget(newUserName_, 0, 1);
    addLayout->addWidget(new QLabel(tr("密码"), addGroup), 0, 2);
    addLayout->addWidget(newUserPass_, 0, 3);
    addLayout->addWidget(new QLabel(tr("角色"), addGroup), 0, 4);
    addLayout->addWidget(newUserRole_, 0, 5);
    addLayout->addWidget(addBtn, 0, 6);

    layout->addWidget(addGroup);

    userTable_ = new QTableWidget(page);
    userTable_->setColumnCount(4);
    userTable_->setHorizontalHeaderLabels({tr("ID"), tr("用户名"), tr("角色"), tr("创建时间")});
    userTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    userTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(userTable_);

    auto *actionsLayout = new QHBoxLayout();
    auto *deleteBtn = new QPushButton(tr("删除用户"), page);
    auto *toggleRoleBtn = new QPushButton(tr("切换权限"), page);
    auto *resetPassBtn = new QPushButton(tr("重置密码"), page);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::handleUserDelete);
    connect(toggleRoleBtn, &QPushButton::clicked, this, &MainWindow::handleUserToggleRole);
    connect(resetPassBtn, &QPushButton::clicked, this, &MainWindow::handleUserResetPassword);

    actionsLayout->addStretch();
    actionsLayout->addWidget(deleteBtn);
    actionsLayout->addWidget(toggleRoleBtn);
    actionsLayout->addWidget(resetPassBtn);
    layout->addLayout(actionsLayout);

    updateUserTable();

    return page;
}

void MainWindow::handleDataTick()
{
    if (!simulationRunning_) {
        return;
    }

    const EnvSample sample = generateSample();
    if (db_) {
        db_->insertSample(sample);
    }

    updateRealtimeUi(sample);
    if (chart_) {
        chart_->addSample(sample);
    }

    checkAlarms(sample);
    lastSample_ = sample;
}

void MainWindow::handleToggleSimulation()
{
    simulationRunning_ = !simulationRunning_;
    toggleSimButton_->setText(simulationRunning_ ? tr("暂停采集") : tr("继续采集"));
}

void MainWindow::handleHistoryQuery()
{
    if (!db_) {
        return;
    }

    if (historyStart_->dateTime() > historyEnd_->dateTime()) {
        QMessageBox::warning(this, tr("提示"), tr("开始时间不能晚于结束时间"));
        return;
    }

    const QList<EnvSample> samples = db_->querySamples(historyStart_->dateTime(), historyEnd_->dateTime(), kDefaultLimit);
    updateHistoryTable(samples);

    const EnvStats stats = db_->queryStats(historyStart_->dateTime(), historyEnd_->dateTime());
    updateStatsTable(stats);
}

void MainWindow::handleRefreshAlarms()
{
    if (!db_) {
        return;
    }
    const QList<AlarmRecord> records = db_->queryAlarms(alarmStart_->dateTime(), alarmEnd_->dateTime(), kDefaultLimit);
    updateAlarmTable(records);
}

void MainWindow::handleClearAlarms()
{
    if (!db_) {
        return;
    }
    if (QMessageBox::question(this, tr("确认"), tr("确定要清空报警记录吗？")) != QMessageBox::Yes) {
        return;
    }
    if (!db_->clearAlarms()) {
        QMessageBox::warning(this, tr("失败"), db_->lastError());
        return;
    }
    handleRefreshAlarms();
}

void MainWindow::handleSaveThresholds()
{
    saveSettings();
    QMessageBox::information(this, tr("已保存"), tr("报警阈值已保存"));
}

void MainWindow::handleExportHistory()
{
    if (!db_) {
        return;
    }

    if (exportStart_->dateTime() > exportEnd_->dateTime()) {
        QMessageBox::warning(this, tr("提示"), tr("开始时间不能晚于结束时间"));
        return;
    }

    const QList<EnvSample> samples = db_->querySamples(exportStart_->dateTime(), exportEnd_->dateTime(), kDefaultLimit);
    const QString path = QFileDialog::getSaveFileName(this, tr("导出历史数据"), QString(), tr("CSV 文件 (*.csv)"));
    if (path.isEmpty()) {
        return;
    }

    if (!exportSamplesToCsv(samples, path,
                             exportTempCheck_->isChecked(),
                             exportHumCheck_->isChecked(),
                             exportPmCheck_->isChecked(),
                             exportCo2Check_->isChecked())) {
        QMessageBox::warning(this, tr("导出失败"), tr("无法写入CSV文件"));
        return;
    }

    QMessageBox::information(this, tr("导出成功"), tr("历史数据已导出"));
}

void MainWindow::handleExportStats()
{
    if (!db_) {
        return;
    }
    if (exportStart_->dateTime() > exportEnd_->dateTime()) {
        QMessageBox::warning(this, tr("提示"), tr("开始时间不能晚于结束时间"));
        return;
    }

    const EnvStats stats = db_->queryStats(exportStart_->dateTime(), exportEnd_->dateTime());
    const QString path = QFileDialog::getSaveFileName(this, tr("导出统计结果"), QString(), tr("CSV 文件 (*.csv)"));
    if (path.isEmpty()) {
        return;
    }

    if (!exportStatsToCsv(stats, path)) {
        QMessageBox::warning(this, tr("导出失败"), tr("无法写入CSV文件"));
        return;
    }

    QMessageBox::information(this, tr("导出成功"), tr("统计结果已导出"));
}

void MainWindow::handleExportAlarms()
{
    if (!db_) {
        return;
    }
    if (exportAlarmStart_->dateTime() > exportAlarmEnd_->dateTime()) {
        QMessageBox::warning(this, tr("提示"), tr("开始时间不能晚于结束时间"));
        return;
    }

    const QList<AlarmRecord> records = db_->queryAlarms(exportAlarmStart_->dateTime(), exportAlarmEnd_->dateTime(), kDefaultLimit);
    const QString path = QFileDialog::getSaveFileName(this, tr("导出报警记录"), QString(), tr("CSV 文件 (*.csv)"));
    if (path.isEmpty()) {
        return;
    }

    if (!exportAlarmToCsv(records, path)) {
        QMessageBox::warning(this, tr("导出失败"), tr("无法写入CSV文件"));
        return;
    }

    QMessageBox::information(this, tr("导出成功"), tr("报警记录已导出"));
}

void MainWindow::handleApplySettings()
{
    saveSettings();
    QMessageBox::information(this, tr("提示"), tr("设置已应用"));
}

void MainWindow::handleBackupDatabase()
{
    if (!db_) {
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

    if (dataTimer_ && wasRunning) {
        dataTimer_->start(refreshIntervalMs_);
    }
    QMessageBox::information(this, tr("成功"), tr("数据库已恢复"));
}

void MainWindow::handleIntervalComboChanged(int index)
{
    auto *combo = qobject_cast<QComboBox *>(sender());
    if (!combo) {
        return;
    }
    const int ms = combo->itemData(index).toInt();
    setRefreshInterval(ms);
    saveSettings();
}

void MainWindow::handleThemeUpdated()
{
    applyMenuPalette();
    if (chart_) {
        chart_->update();
    }
    updateThemeStatus();
}

void MainWindow::handleUserAdd()
{
    if (!db_) {
        return;
    }
    const QString username = newUserName_->text().trimmed();
    const QString password = newUserPass_->text();
    const QString role = newUserRole_->currentData().toString();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请输入用户名和密码"));
        return;
    }

    if (!db_->addUser(username, password, role)) {
        QMessageBox::warning(this, tr("添加失败"), db_->lastError());
        return;
    }

    newUserName_->clear();
    newUserPass_->clear();
    updateUserTable();
}

void MainWindow::handleUserDelete()
{
    if (!db_ || !userTable_) {
        return;
    }

    const int row = userTable_->currentRow();
    if (row < 0) {
        return;
    }

    const int id = userTable_->item(row, 0)->text().toInt();
    if (id == currentUser_.id) {
        QMessageBox::warning(this, tr("提示"), tr("不能删除当前登录用户"));
        return;
    }

    if (QMessageBox::question(this, tr("确认"), tr("确定删除该用户吗？")) != QMessageBox::Yes) {
        return;
    }

    if (!db_->deleteUser(id)) {
        QMessageBox::warning(this, tr("失败"), db_->lastError());
        return;
    }

    updateUserTable();
}

void MainWindow::handleUserToggleRole()
{
    if (!db_ || !userTable_) {
        return;
    }

    const int row = userTable_->currentRow();
    if (row < 0) {
        return;
    }

    const int id = userTable_->item(row, 0)->text().toInt();
    const QString role = userTable_->item(row, 2)->text();

    const QString newRole = (role == QStringLiteral("admin")) ? QStringLiteral("user") : QStringLiteral("admin");
    if (!db_->updateUserRole(id, newRole)) {
        QMessageBox::warning(this, tr("失败"), db_->lastError());
        return;
    }

    updateUserTable();
}

void MainWindow::handleUserResetPassword()
{
    if (!db_ || !userTable_) {
        return;
    }

    const int row = userTable_->currentRow();
    if (row < 0) {
        return;
    }

    const int id = userTable_->item(row, 0)->text().toInt();
    bool ok = false;
    const QString password = QInputDialog::getText(this, tr("重置密码"), tr("输入新密码"),
                                                   QLineEdit::Password, QString(), &ok);
    if (!ok || password.isEmpty()) {
        return;
    }

    if (!db_->setUserPassword(id, password)) {
        QMessageBox::warning(this, tr("失败"), db_->lastError());
        return;
    }

    QMessageBox::information(this, tr("成功"), tr("密码已更新"));
}

void MainWindow::handleCleanupData()
{
    if (!db_) {
        return;
    }

    const int days = cleanupDaysSpin_->value();
    if (QMessageBox::question(this, tr("确认"), tr("将删除%1天之前的历史数据，是否继续？").arg(days))
        != QMessageBox::Yes) {
        return;
    }

    const QDateTime cutoff = QDateTime::currentDateTime().addDays(-days);
    if (!db_->deleteSamplesBefore(cutoff)) {
        QMessageBox::warning(this, tr("失败"), db_->lastError());
        return;
    }

    QMessageBox::information(this, tr("完成"), tr("历史数据已清理"));
}

void MainWindow::updateRealtimeUi(const EnvSample &sample)
{
    tempValue_->setText(QString::number(sample.temperature, 'f', 1));
    humValue_->setText(QString::number(sample.humidity, 'f', 1));
    pmValue_->setText(QString::number(sample.pm25, 'f', 1));
    co2Value_->setText(QString::number(sample.co2, 'f', 0));

    tempBar_->setValue(static_cast<int>(sample.temperature * 10));
    humBar_->setValue(static_cast<int>(sample.humidity));
    pmBar_->setValue(static_cast<int>(sample.pm25));
    co2Bar_->setValue(static_cast<int>(sample.co2));

    lastUpdateLabel_->setText(tr("最近更新时间: %1").arg(sample.ts.toString("yyyy-MM-dd HH:mm:ss")));
}

void MainWindow::updateHistoryTable(const QList<EnvSample> &samples)
{
    historyTable_->setRowCount(samples.size());

    for (int row = 0; row < samples.size(); ++row) {
        const EnvSample &sample = samples.at(row);
        historyTable_->setItem(row, 0, new QTableWidgetItem(sample.ts.toString("yyyy-MM-dd HH:mm:ss")));
        historyTable_->setItem(row, 1, new QTableWidgetItem(QString::number(sample.temperature, 'f', 1)));
        historyTable_->setItem(row, 2, new QTableWidgetItem(QString::number(sample.humidity, 'f', 1)));
        historyTable_->setItem(row, 3, new QTableWidgetItem(QString::number(sample.pm25, 'f', 1)));
        historyTable_->setItem(row, 4, new QTableWidgetItem(QString::number(sample.co2, 'f', 0)));
    }

    historyTable_->setColumnHidden(1, !historyTempCheck_->isChecked());
    historyTable_->setColumnHidden(2, !historyHumCheck_->isChecked());
    historyTable_->setColumnHidden(3, !historyPmCheck_->isChecked());
    historyTable_->setColumnHidden(4, !historyCo2Check_->isChecked());

    historyCountLabel_->setText(tr("查询记录: %1 (最多显示%2条)").arg(samples.size()).arg(kDefaultLimit));
}

void MainWindow::updateStatsTable(const EnvStats &stats)
{
    auto setRow = [&](int row, double minValue, double maxValue, double avgValue) {
        statsTable_->setItem(row, 0, new QTableWidgetItem(QString::number(minValue, 'f', 1)));
        statsTable_->setItem(row, 1, new QTableWidgetItem(QString::number(maxValue, 'f', 1)));
        statsTable_->setItem(row, 2, new QTableWidgetItem(QString::number(avgValue, 'f', 1)));
    };

    if (!stats.valid) {
        statsTable_->clearContents();
        return;
    }

    setRow(0, stats.minTemp, stats.maxTemp, stats.avgTemp);
    setRow(1, stats.minHum, stats.maxHum, stats.avgHum);
    setRow(2, stats.minPm, stats.maxPm, stats.avgPm);
    setRow(3, stats.minCo2, stats.maxCo2, stats.avgCo2);
}

void MainWindow::updateAlarmTable(const QList<AlarmRecord> &records)
{
    alarmTable_->setRowCount(records.size());
    for (int row = 0; row < records.size(); ++row) {
        const AlarmRecord &record = records.at(row);
        alarmTable_->setItem(row, 0, new QTableWidgetItem(record.ts.toString("yyyy-MM-dd HH:mm:ss")));
        alarmTable_->setItem(row, 1, new QTableWidgetItem(record.param));
        alarmTable_->setItem(row, 2, new QTableWidgetItem(QString::number(record.value, 'f', 1)));
        alarmTable_->setItem(row, 3, new QTableWidgetItem(QString::number(record.threshold, 'f', 1)));
    }
}

void MainWindow::updateUserTable()
{
    if (!db_ || !userTable_) {
        return;
    }

    const QList<UserInfo> users = db_->listUsers();
    userTable_->setRowCount(users.size());
    for (int row = 0; row < users.size(); ++row) {
        const UserInfo &user = users.at(row);
        userTable_->setItem(row, 0, new QTableWidgetItem(QString::number(user.id)));
        userTable_->setItem(row, 1, new QTableWidgetItem(user.username));
        userTable_->setItem(row, 2, new QTableWidgetItem(user.role));
        userTable_->setItem(row, 3, new QTableWidgetItem(user.createdAt.toString("yyyy-MM-dd HH:mm:ss")));
    }
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
    if (!themeStatusLabel_) {
        return;
    }
    if (!themeManager_) {
        themeStatusLabel_->setText(tr("跟随系统主题 (qt6ct)"));
        return;
    }
    const QString schemeName = themeManager_->currentSchemeName().isEmpty()
        ? tr("未检测到")
        : themeManager_->currentSchemeName();
    const QString styleName = themeManager_->currentStyleName().isEmpty()
        ? QApplication::style()->objectName()
        : themeManager_->currentStyleName();
    themeStatusLabel_->setText(tr("跟随系统主题 (qt6ct)，样式: %1，配色: %2")
                                   .arg(styleName, schemeName));
}

void MainWindow::loadSettings()
{
    QSettings settings;
    refreshIntervalMs_ = settings.value(QStringLiteral("refresh_interval_ms"), 1000).toInt();

    const QString sound = settings.value(QStringLiteral("alarm_sound"), QStringLiteral("beep")).toString();

    const double tempThreshold = settings.value(QStringLiteral("thresholds/temperature"), 35.0).toDouble();
    const double humThreshold = settings.value(QStringLiteral("thresholds/humidity"), 85.0).toDouble();
    const double pmThreshold = settings.value(QStringLiteral("thresholds/pm25"), 150.0).toDouble();
    const double co2Threshold = settings.value(QStringLiteral("thresholds/co2"), 1200.0).toDouble();

    const bool alarmEnabled = settings.value(QStringLiteral("alarm_enabled"), true).toBool();
    const int alarmCooldown = settings.value(QStringLiteral("alarm_cooldown_sec"), 30).toInt();

    tempThreshold_->setValue(tempThreshold);
    humThreshold_->setValue(humThreshold);
    pmThreshold_->setValue(pmThreshold);
    co2Threshold_->setValue(co2Threshold);
    alarmEnabledCheck_->setChecked(alarmEnabled);
    alarmCooldownSpin_->setValue(alarmCooldown);

    auto applyCombo = [](QComboBox *combo, const QVariant &value) {
        const int idx = combo->findData(value);
        if (idx >= 0) {
            QSignalBlocker blocker(combo);
            combo->setCurrentIndex(idx);
        }
    };

    applyCombo(realtimeIntervalCombo_, refreshIntervalMs_);
    applyCombo(settingsIntervalCombo_, refreshIntervalMs_);
    applyCombo(soundCombo_, sound);

    if (dbPathLabel_ && db_) {
        dbPathLabel_->setText(db_->databasePath());
    }
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue(QStringLiteral("refresh_interval_ms"), refreshIntervalMs_);
    settings.setValue(QStringLiteral("alarm_sound"), soundCombo_->currentData().toString());
    settings.setValue(QStringLiteral("thresholds/temperature"), tempThreshold_->value());
    settings.setValue(QStringLiteral("thresholds/humidity"), humThreshold_->value());
    settings.setValue(QStringLiteral("thresholds/pm25"), pmThreshold_->value());
    settings.setValue(QStringLiteral("thresholds/co2"), co2Threshold_->value());
    settings.setValue(QStringLiteral("alarm_enabled"), alarmEnabledCheck_->isChecked());
    settings.setValue(QStringLiteral("alarm_cooldown_sec"), alarmCooldownSpin_->value());

}

void MainWindow::setRefreshInterval(int ms)
{
    refreshIntervalMs_ = ms;
    if (dataTimer_) {
        dataTimer_->setInterval(refreshIntervalMs_);
    }

    auto setCombo = [&](QComboBox *combo) {
        const int idx = combo->findData(ms);
        if (idx >= 0 && combo->currentIndex() != idx) {
            QSignalBlocker blocker(combo);
            combo->setCurrentIndex(idx);
        }
    };

    setCombo(realtimeIntervalCombo_);
    setCombo(settingsIntervalCombo_);
}

EnvSample MainWindow::generateSample()
{
    EnvSample sample;
    sample.ts = QDateTime::currentDateTime();

    auto randBetween = [](double min, double max) {
        return min + QRandomGenerator::global()->generateDouble() * (max - min);
    };

    if (!lastSample_.ts.isValid()) {
        sample.temperature = randBetween(18, 28);
        sample.humidity = randBetween(40, 60);
        sample.pm25 = randBetween(20, 80);
        sample.co2 = randBetween(450, 800);
        return sample;
    }

    sample.temperature = qBound(0.0, lastSample_.temperature + randBetween(-0.6, 0.6), 40.0);
    sample.humidity = qBound(10.0, lastSample_.humidity + randBetween(-1.5, 1.5), 90.0);
    sample.pm25 = qBound(0.0, lastSample_.pm25 + randBetween(-5, 5), 200.0);
    sample.co2 = qBound(300.0, lastSample_.co2 + randBetween(-30, 30), 2000.0);

    return sample;
}

void MainWindow::checkAlarms(const EnvSample &sample)
{
    if (!alarmEnabledCheck_->isChecked()) {
        return;
    }

    const QDateTime now = QDateTime::currentDateTime();
    const int cooldown = alarmCooldownSpin_->value();

    auto triggerAlarm = [&](const QString &param, double value, double threshold, QDateTime &lastTime) {
        if (value <= threshold) {
            return;
        }
        if (lastTime.isValid() && lastTime.secsTo(now) < cooldown) {
            return;
        }

        lastTime = now;
        if (db_) {
            db_->insertAlarm(param, value, threshold);
        }

        if (soundCombo_->currentData().toString() == QStringLiteral("beep")) {
            QApplication::beep();
        }

        auto *box = new QMessageBox(QMessageBox::Warning,
                                    tr("报警"),
                                    tr("%1 超过阈值: %2 / %3").arg(param)
                                        .arg(QString::number(value, 'f', 1))
                                        .arg(QString::number(threshold, 'f', 1)),
                                    QMessageBox::Ok,
                                    this);
        box->setAttribute(Qt::WA_DeleteOnClose);
        box->show();
    };

    triggerAlarm(tr("温度"), sample.temperature, tempThreshold_->value(), lastAlarmTemp_);
    triggerAlarm(tr("湿度"), sample.humidity, humThreshold_->value(), lastAlarmHum_);
    triggerAlarm(tr("PM2.5"), sample.pm25, pmThreshold_->value(), lastAlarmPm_);
    triggerAlarm(tr("CO2"), sample.co2, co2Threshold_->value(), lastAlarmCo2_);
}

bool MainWindow::exportSamplesToCsv(const QList<EnvSample> &samples, const QString &path,
                                   bool useTemp, bool useHum, bool usePm, bool useCo2)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    setUtf8(out);

    QStringList headers;
    headers << tr("时间");
    if (useTemp) headers << tr("温度");
    if (useHum) headers << tr("湿度");
    if (usePm) headers << tr("PM2.5");
    if (useCo2) headers << tr("CO2");
    out << headers.join(',') << "\n";

    for (const auto &sample : samples) {
        QStringList row;
        row << sample.ts.toString("yyyy-MM-dd HH:mm:ss");
        if (useTemp) row << QString::number(sample.temperature, 'f', 1);
        if (useHum) row << QString::number(sample.humidity, 'f', 1);
        if (usePm) row << QString::number(sample.pm25, 'f', 1);
        if (useCo2) row << QString::number(sample.co2, 'f', 0);
        out << row.join(',') << "\n";
    }

    return true;
}

bool MainWindow::exportAlarmToCsv(const QList<AlarmRecord> &records, const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    setUtf8(out);
    out << tr("时间,参数,值,阈值") << "\n";

    for (const auto &record : records) {
        out << record.ts.toString("yyyy-MM-dd HH:mm:ss") << ","
            << record.param << ","
            << QString::number(record.value, 'f', 1) << ","
            << QString::number(record.threshold, 'f', 1)
            << "\n";
    }

    return true;
}

bool MainWindow::exportStatsToCsv(const EnvStats &stats, const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    setUtf8(out);
    out << tr("参数,最小值,最大值,平均值") << "\n";

    if (!stats.valid) {
        return true;
    }

    auto writeRow = [&](const QString &name, double minValue, double maxValue, double avgValue) {
        out << name << ","
            << QString::number(minValue, 'f', 1) << ","
            << QString::number(maxValue, 'f', 1) << ","
            << QString::number(avgValue, 'f', 1)
            << "\n";
    };

    writeRow(tr("温度"), stats.minTemp, stats.maxTemp, stats.avgTemp);
    writeRow(tr("湿度"), stats.minHum, stats.maxHum, stats.avgHum);
    writeRow(tr("PM2.5"), stats.minPm, stats.maxPm, stats.avgPm);
    writeRow(tr("CO2"), stats.minCo2, stats.maxCo2, stats.avgCo2);

    return true;
}
