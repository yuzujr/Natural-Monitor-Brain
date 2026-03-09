#include "realtimepagewidget.h"

#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include "common/uistyles.h"
#include "linechartwidget.h"

namespace {
constexpr int kDefaultMaxPoints = 50;
}

RealtimePageWidget::RealtimePageWidget(QWidget *parent)
    : QWidget(parent)
{
    UiStyles::applyPageStyle(this);

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(14);

    headerCard_ = new QFrame(this);
    auto *headerLayout = new QVBoxLayout(headerCard_);
    titleLabel_ = new QLabel(tr("环境态势总览"), headerCard_);
    subtitleLabel_ = new QLabel(tr("实时监控温度、湿度、PM2.5 与 CO2 变化，支持快速切换刷新节奏。"), headerCard_);
    headerLayout->addWidget(titleLabel_);
    headerLayout->addWidget(subtitleLabel_);
    layout->addWidget(headerCard_);

    auto *toolbar = new QHBoxLayout();
    sourceLabel_ = new QLabel(tr("数据来源: 模拟传感器"), this);

    toggleSimButton_ = new QPushButton(tr("暂停采集"), this);
    UiStyles::applyButtonVariant(toggleSimButton_, QStringLiteral("secondary"));
    connect(toggleSimButton_, &QPushButton::clicked, this, &RealtimePageWidget::simulationToggleRequested);

    intervalCombo_ = new QComboBox(this);
    intervalCombo_->addItem(tr("1 秒"), 1000);
    intervalCombo_->addItem(tr("5 秒"), 5000);
    intervalCombo_->addItem(tr("10 秒"), 10000);
    connect(intervalCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                emit refreshIntervalChanged(intervalCombo_->itemData(index).toInt());
            });

    toolbar->addWidget(sourceLabel_);
    toolbar->addStretch();
    toolbar->addWidget(new QLabel(tr("刷新频率"), this));
    toolbar->addWidget(intervalCombo_);
    toolbar->addWidget(toggleSimButton_);
    layout->addLayout(toolbar);

    auto *dataGroup = new QGroupBox(tr("当前环境数据"), this);
    auto *grid = new QGridLayout(dataGroup);
    grid->setHorizontalSpacing(14);
    grid->setVerticalSpacing(14);

    auto createCard = [&](int row, int column, const QString &name, const QString &accent,
                          QFrame *&card, QLabel *&nameLabel, QLabel *&valueLabel,
                          QProgressBar *&bar, int min, int max) {
        card = new QFrame(dataGroup);
        auto *cardLayout = new QVBoxLayout(card);
        nameLabel = new QLabel(name, card);
        valueLabel = new QLabel("--", card);
        bar = new QProgressBar(card);
        bar->setRange(min, max);
        bar->setTextVisible(false);

        cardLayout->addWidget(nameLabel);
        cardLayout->addWidget(valueLabel);
        cardLayout->addWidget(bar);
        grid->addWidget(card, row, column);
    };

    createCard(0, 0, tr("温度 (C)"), QStringLiteral("#ef6f6c"), tempCard_, tempTitleLabel_, tempValue_, tempBar_, 0, 400);
    createCard(0, 1, tr("湿度 (%)"), QStringLiteral("#3f88c5"), humCard_, humTitleLabel_, humValue_, humBar_, 0, 100);
    createCard(1, 0, tr("PM2.5 (ug/m3)"), QStringLiteral("#f4a259"), pmCard_, pmTitleLabel_, pmValue_, pmBar_, 0, 200);
    createCard(1, 1, tr("CO2 (ppm)"), QStringLiteral("#5bba6f"), co2Card_, co2TitleLabel_, co2Value_, co2Bar_, 300, 2000);

    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    layout->addWidget(dataGroup);

    chart_ = new LineChartWidget(this);
    chart_->setMaxPoints(kDefaultMaxPoints);
    auto *chartGroup = new QGroupBox(tr("趋势概览"), this);
    auto *chartLayout = new QVBoxLayout(chartGroup);
    chartLayout->addWidget(chart_);
    layout->addWidget(chartGroup);

    lastUpdateLabel_ = new QLabel(tr("最近更新时间: --"), this);
    layout->addWidget(lastUpdateLabel_);

    layout->addStretch();

    refreshThemeStyles();
}

void RealtimePageWidget::refreshThemeStyles()
{
    UiStyles::applyPageStyle(this);
    headerCard_->setStyleSheet(UiStyles::metricCardStyle(QStringLiteral("#2e86c1")));
    titleLabel_->setStyleSheet(UiStyles::titleTextStyle());
    subtitleLabel_->setStyleSheet(UiStyles::secondaryTextStyle());
    sourceLabel_->setStyleSheet(UiStyles::secondaryTextStyle());
    lastUpdateLabel_->setStyleSheet(UiStyles::secondaryTextStyle());

    applyMetricCardStyle(tempCard_, tempTitleLabel_, tempValue_, tempBar_, QStringLiteral("#ef6f6c"));
    applyMetricCardStyle(humCard_, humTitleLabel_, humValue_, humBar_, QStringLiteral("#3f88c5"));
    applyMetricCardStyle(pmCard_, pmTitleLabel_, pmValue_, pmBar_, QStringLiteral("#f4a259"));
    applyMetricCardStyle(co2Card_, co2TitleLabel_, co2Value_, co2Bar_, QStringLiteral("#5bba6f"));
}

void RealtimePageWidget::applyMetricCardStyle(QFrame *card, QLabel *nameLabel, QLabel *valueLabel,
                                              QProgressBar *bar, const QString &accentColor)
{
    if (!card || !nameLabel || !valueLabel || !bar) {
        return;
    }

    card->setStyleSheet(UiStyles::metricCardStyle(accentColor));
    nameLabel->setStyleSheet(UiStyles::metricTitleStyle());
    valueLabel->setStyleSheet(UiStyles::metricValueStyle());
    bar->setStyleSheet(UiStyles::progressBarStyle(accentColor));
}

void RealtimePageWidget::setRefreshInterval(int ms)
{
    const int index = intervalCombo_->findData(ms);
    if (index >= 0 && intervalCombo_->currentIndex() != index) {
        QSignalBlocker blocker(intervalCombo_);
        intervalCombo_->setCurrentIndex(index);
    }
}

void RealtimePageWidget::setSimulationRunning(bool running)
{
    toggleSimButton_->setText(running ? tr("暂停采集") : tr("继续采集"));
}

void RealtimePageWidget::updateSample(const EnvSample &sample)
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

void RealtimePageWidget::addSample(const EnvSample &sample)
{
    if (chart_) {
        chart_->addSample(sample);
    }
}