#include "exportpagewidget.h"

#include <QCheckBox>
#include <QDateTimeEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "common/uistyles.h"

ExportPageWidget::ExportPageWidget(QWidget *parent)
    : QWidget(parent)
{
    UiStyles::applyPageStyle(this);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(14);

    auto *introLabel = new QLabel(tr("将历史数据、统计结果和报警记录导出为 CSV，便于归档和二次分析。"), this);
    introLabel->setStyleSheet(UiStyles::secondaryTextStyle());
    layout->addWidget(introLabel);

    auto *historyGroup = new QGroupBox(tr("导出历史数据"), this);
    auto *historyLayout = new QGridLayout(historyGroup);
    historyStartEdit_ = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), historyGroup);
    historyEndEdit_ = new QDateTimeEdit(QDateTime::currentDateTime(), historyGroup);
    historyStartEdit_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    historyEndEdit_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    historyStartEdit_->setCalendarPopup(true);
    historyEndEdit_->setCalendarPopup(true);

    tempCheck_ = new QCheckBox(tr("温度"), historyGroup);
    humCheck_ = new QCheckBox(tr("湿度"), historyGroup);
    pmCheck_ = new QCheckBox(tr("PM2.5"), historyGroup);
    co2Check_ = new QCheckBox(tr("CO2"), historyGroup);
    tempCheck_->setChecked(true);
    humCheck_->setChecked(true);
    pmCheck_->setChecked(true);
    co2Check_->setChecked(true);

    auto *exportHistoryButton = new QPushButton(tr("导出CSV"), historyGroup);
    connect(exportHistoryButton, &QPushButton::clicked, this, [this]() {
        emit exportHistoryRequested(historyStartEdit_->dateTime(), historyEndEdit_->dateTime(), currentSelection());
    });

    historyLayout->addWidget(new QLabel(tr("开始时间"), historyGroup), 0, 0);
    historyLayout->addWidget(historyStartEdit_, 0, 1);
    historyLayout->addWidget(new QLabel(tr("结束时间"), historyGroup), 0, 2);
    historyLayout->addWidget(historyEndEdit_, 0, 3);

    auto *paramBox = new QWidget(historyGroup);
    auto *paramLayout = new QHBoxLayout(paramBox);
    paramLayout->setContentsMargins(0, 0, 0, 0);
    paramLayout->addWidget(tempCheck_);
    paramLayout->addWidget(humCheck_);
    paramLayout->addWidget(pmCheck_);
    paramLayout->addWidget(co2Check_);
    paramLayout->addStretch();

    historyLayout->addWidget(new QLabel(tr("导出参数"), historyGroup), 1, 0);
    historyLayout->addWidget(paramBox, 1, 1, 1, 3);
    historyLayout->addWidget(exportHistoryButton, 0, 4, 2, 1);
    layout->addWidget(historyGroup);

    auto *statsGroup = new QGroupBox(tr("导出统计结果"), this);
    auto *statsLayout = new QHBoxLayout(statsGroup);
    auto *exportStatsButton = new QPushButton(tr("导出统计CSV"), statsGroup);
    connect(exportStatsButton, &QPushButton::clicked, this, [this]() {
        emit exportStatsRequested(historyStartEdit_->dateTime(), historyEndEdit_->dateTime());
    });
    statsLayout->addWidget(new QLabel(tr("使用上方时间范围"), statsGroup));
    statsLayout->addStretch();
    statsLayout->addWidget(exportStatsButton);
    layout->addWidget(statsGroup);

    auto *alarmGroup = new QGroupBox(tr("导出报警记录"), this);
    auto *alarmLayout = new QGridLayout(alarmGroup);
    alarmStartEdit_ = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), alarmGroup);
    alarmEndEdit_ = new QDateTimeEdit(QDateTime::currentDateTime(), alarmGroup);
    alarmStartEdit_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    alarmEndEdit_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    alarmStartEdit_->setCalendarPopup(true);
    alarmEndEdit_->setCalendarPopup(true);

    auto *exportAlarmButton = new QPushButton(tr("导出CSV"), alarmGroup);
    connect(exportAlarmButton, &QPushButton::clicked, this, [this]() {
        emit exportAlarmsRequested(alarmStartEdit_->dateTime(), alarmEndEdit_->dateTime());
    });

    alarmLayout->addWidget(new QLabel(tr("开始时间"), alarmGroup), 0, 0);
    alarmLayout->addWidget(alarmStartEdit_, 0, 1);
    alarmLayout->addWidget(new QLabel(tr("结束时间"), alarmGroup), 0, 2);
    alarmLayout->addWidget(alarmEndEdit_, 0, 3);
    alarmLayout->addWidget(exportAlarmButton, 0, 4);

    layout->addWidget(alarmGroup);
    layout->addStretch();
}

SampleSelection ExportPageWidget::currentSelection() const
{
    SampleSelection selection;
    selection.temperature = tempCheck_->isChecked();
    selection.humidity = humCheck_->isChecked();
    selection.pm25 = pmCheck_->isChecked();
    selection.co2 = co2Check_->isChecked();
    return selection;
}