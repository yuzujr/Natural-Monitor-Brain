#include "historypagewidget.h"

#include <QCheckBox>
#include <QDateTimeEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "common/uistyles.h"

HistoryPageWidget::HistoryPageWidget(QWidget *parent)
    : QWidget(parent)
{
    UiStyles::applyPageStyle(this);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(14);

    auto *introLabel = new QLabel(tr("按时间范围检索历史数据，并查看同一时间段内各项指标的统计结果。"), this);
    introLabel->setStyleSheet(UiStyles::secondaryTextStyle());
    layout->addWidget(introLabel);

    auto *filterGroup = new QGroupBox(tr("查询条件"), this);
    auto *filterLayout = new QGridLayout(filterGroup);

    startEdit_ = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(-3600), filterGroup);
    endEdit_ = new QDateTimeEdit(QDateTime::currentDateTime(), filterGroup);
    startEdit_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    endEdit_->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    startEdit_->setCalendarPopup(true);
    endEdit_->setCalendarPopup(true);

    tempCheck_ = new QCheckBox(tr("温度"), filterGroup);
    humCheck_ = new QCheckBox(tr("湿度"), filterGroup);
    pmCheck_ = new QCheckBox(tr("PM2.5"), filterGroup);
    co2Check_ = new QCheckBox(tr("CO2"), filterGroup);
    tempCheck_->setChecked(true);
    humCheck_->setChecked(true);
    pmCheck_->setChecked(true);
    co2Check_->setChecked(true);

    auto *queryButton = new QPushButton(tr("查询"), filterGroup);
    UiStyles::applyButtonVariant(queryButton, QStringLiteral("primary"));
    connect(queryButton, &QPushButton::clicked, this, [this]() {
        emit queryRequested(startEdit_->dateTime(), endEdit_->dateTime());
    });

    filterLayout->addWidget(new QLabel(tr("开始时间"), filterGroup), 0, 0);
    filterLayout->addWidget(startEdit_, 0, 1);
    filterLayout->addWidget(new QLabel(tr("结束时间"), filterGroup), 0, 2);
    filterLayout->addWidget(endEdit_, 0, 3);

    filterLayout->addWidget(new QLabel(tr("参数"), filterGroup), 1, 0);
    auto *paramBox = new QWidget(filterGroup);
    auto *paramLayout = new QHBoxLayout(paramBox);
    paramLayout->setContentsMargins(0, 0, 0, 0);
    paramLayout->addWidget(tempCheck_);
    paramLayout->addWidget(humCheck_);
    paramLayout->addWidget(pmCheck_);
    paramLayout->addWidget(co2Check_);
    paramLayout->addStretch();
    filterLayout->addWidget(paramBox, 1, 1, 1, 3);

    filterLayout->addWidget(queryButton, 0, 4, 2, 1);

    layout->addWidget(filterGroup);

    historyTable_ = new QTableWidget(this);
    historyTable_->setColumnCount(5);
    historyTable_->setHorizontalHeaderLabels({tr("时间"), tr("温度"), tr("湿度"), tr("PM2.5"), tr("CO2")});
    historyTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    historyTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    historyTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    historyTable_->setAlternatingRowColors(true);
    layout->addWidget(historyTable_);

    auto *statsGroup = new QGroupBox(tr("统计结果"), this);
    auto *statsLayout = new QVBoxLayout(statsGroup);
    statsTable_ = new QTableWidget(statsGroup);
    statsTable_->setRowCount(4);
    statsTable_->setColumnCount(3);
    statsTable_->setHorizontalHeaderLabels({tr("最小值"), tr("最大值"), tr("平均值")});
    statsTable_->setVerticalHeaderLabels({tr("温度"), tr("湿度"), tr("PM2.5"), tr("CO2")});
    statsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    statsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable_->setAlternatingRowColors(true);
    statsLayout->addWidget(statsTable_);
    layout->addWidget(statsGroup);

    countLabel_ = new QLabel(tr("查询记录: 0"), this);
    countLabel_->setStyleSheet(UiStyles::secondaryTextStyle());
    layout->addWidget(countLabel_);
}

QDateTime HistoryPageWidget::startDateTime() const
{
    return startEdit_->dateTime();
}

QDateTime HistoryPageWidget::endDateTime() const
{
    return endEdit_->dateTime();
}

void HistoryPageWidget::setQueryResult(const QList<EnvSample> &samples, const EnvStats &stats, int limit)
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

    historyTable_->setColumnHidden(1, !tempCheck_->isChecked());
    historyTable_->setColumnHidden(2, !humCheck_->isChecked());
    historyTable_->setColumnHidden(3, !pmCheck_->isChecked());
    historyTable_->setColumnHidden(4, !co2Check_->isChecked());

    auto setRow = [&](int row, double minValue, double maxValue, double avgValue) {
        statsTable_->setItem(row, 0, new QTableWidgetItem(QString::number(minValue, 'f', 1)));
        statsTable_->setItem(row, 1, new QTableWidgetItem(QString::number(maxValue, 'f', 1)));
        statsTable_->setItem(row, 2, new QTableWidgetItem(QString::number(avgValue, 'f', 1)));
    };

    if (!stats.valid) {
        statsTable_->clearContents();
    } else {
        setRow(0, stats.minTemp, stats.maxTemp, stats.avgTemp);
        setRow(1, stats.minHum, stats.maxHum, stats.avgHum);
        setRow(2, stats.minPm, stats.maxPm, stats.avgPm);
        setRow(3, stats.minCo2, stats.maxCo2, stats.avgCo2);
    }

    countLabel_->setText(tr("查询记录: %1 (最多显示%2条)").arg(samples.size()).arg(limit));
}