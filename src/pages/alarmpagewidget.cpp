#include "alarmpagewidget.h"

#include <QCheckBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

#include "common/uistyles.h"

AlarmPageWidget::AlarmPageWidget(QWidget *parent)
    : QWidget(parent)
{
    UiStyles::applyPageStyle(this);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(14);

    auto *introLabel = new QLabel(tr("配置异常阈值与冷却周期，系统将在实时采集阶段自动判定并记录报警。"), this);
    introLabel->setStyleSheet(UiStyles::secondaryTextStyle());
    layout->addWidget(introLabel);

    auto *thresholdGroup = new QGroupBox(tr("阈值设置"), this);
    auto *thresholdLayout = new QGridLayout(thresholdGroup);

    auto makeThreshold = [&](int row, const QString &label, QDoubleSpinBox *&spin,
                             double min, double max, const QString &suffix) {
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

    enabledCheck_ = new QCheckBox(tr("启用报警"), thresholdGroup);
    thresholdLayout->addWidget(enabledCheck_, 0, 2);

    cooldownSpin_ = new QSpinBox(thresholdGroup);
    cooldownSpin_->setRange(5, 600);
    cooldownSpin_->setSuffix(tr(" 秒"));
    thresholdLayout->addWidget(new QLabel(tr("报警间隔"), thresholdGroup), 1, 2);
    thresholdLayout->addWidget(cooldownSpin_, 1, 3);

    auto *saveButton = new QPushButton(tr("保存设置"), thresholdGroup);
    connect(saveButton, &QPushButton::clicked, this, &AlarmPageWidget::saveThresholdsRequested);
    thresholdLayout->addWidget(saveButton, 2, 2, 1, 2);

    layout->addWidget(thresholdGroup);

    auto *logGroup = new QGroupBox(tr("报警记录"), this);
    auto *logLayout = new QVBoxLayout(logGroup);

    auto *filterLayout = new QHBoxLayout();
    startEdit_ = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), logGroup);
    endEdit_ = new QDateTimeEdit(QDateTime::currentDateTime(), logGroup);
    startEdit_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    endEdit_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    startEdit_->setCalendarPopup(true);
    endEdit_->setCalendarPopup(true);

    auto *refreshButton = new QPushButton(tr("刷新"), logGroup);
    auto *clearButton = new QPushButton(tr("清空"), logGroup);
    connect(refreshButton, &QPushButton::clicked, this, [this]() {
        emit refreshRequested(startEdit_->dateTime(), endEdit_->dateTime());
    });
    connect(clearButton, &QPushButton::clicked, this, &AlarmPageWidget::clearRequested);

    filterLayout->addWidget(new QLabel(tr("开始"), logGroup));
    filterLayout->addWidget(startEdit_);
    filterLayout->addWidget(new QLabel(tr("结束"), logGroup));
    filterLayout->addWidget(endEdit_);
    filterLayout->addStretch();
    filterLayout->addWidget(refreshButton);
    filterLayout->addWidget(clearButton);

    logLayout->addLayout(filterLayout);

    alarmTable_ = new QTableWidget(logGroup);
    alarmTable_->setColumnCount(4);
    alarmTable_->setHorizontalHeaderLabels({tr("时间"), tr("参数"), tr("值"), tr("阈值")});
    alarmTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    alarmTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    alarmTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    alarmTable_->setAlternatingRowColors(true);
    logLayout->addWidget(alarmTable_);

    layout->addWidget(logGroup);
}

AlarmSettings AlarmPageWidget::alarmSettings() const
{
    AlarmSettings settings;
    settings.enabled = enabledCheck_->isChecked();
    settings.cooldownSeconds = cooldownSpin_->value();
    settings.temperatureThreshold = tempThreshold_->value();
    settings.humidityThreshold = humThreshold_->value();
    settings.pm25Threshold = pmThreshold_->value();
    settings.co2Threshold = co2Threshold_->value();
    return settings;
}

QDateTime AlarmPageWidget::startDateTime() const
{
    return startEdit_->dateTime();
}

QDateTime AlarmPageWidget::endDateTime() const
{
    return endEdit_->dateTime();
}

void AlarmPageWidget::setAlarmSettings(const AlarmSettings &settings)
{
    enabledCheck_->setChecked(settings.enabled);
    cooldownSpin_->setValue(settings.cooldownSeconds);
    tempThreshold_->setValue(settings.temperatureThreshold);
    humThreshold_->setValue(settings.humidityThreshold);
    pmThreshold_->setValue(settings.pm25Threshold);
    co2Threshold_->setValue(settings.co2Threshold);
}

void AlarmPageWidget::setAlarmRecords(const QList<AlarmRecord> &records)
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