#include "linechartwidget.h"

#include "databasemanager.h"

#ifdef NATURAL_MONITOR_USE_QCUSTOMPLOT
#include "qcustomplot.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#endif

#include <QPainter>
#include <QPainterPath>
#include <QtGlobal>
#include <QtMath>

namespace {
constexpr double kTempMin = 0.0;
constexpr double kTempMax = 40.0;
constexpr double kHumMin = 0.0;
constexpr double kHumMax = 100.0;
constexpr double kPmMin = 0.0;
constexpr double kPmMax = 200.0;
constexpr double kCo2Min = 300.0;
constexpr double kCo2Max = 2000.0;

double normalize(double value, double minValue, double maxValue)
{
    if (qFuzzyCompare(minValue, maxValue)) {
        return 0.0;
    }
    const double clamped = qBound(minValue, value, maxValue);
    return (clamped - minValue) / (maxValue - minValue);
}

double clampValue(double value, double minValue, double maxValue)
{
    return qBound(minValue, value, maxValue);
}
}

LineChartWidget::LineChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(320);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

#ifdef NATURAL_MONITOR_USE_QCUSTOMPLOT
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    customPlot_ = new QCustomPlot(this);
    customPlot_->legend->setVisible(false);
    customPlot_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    customPlot_->setMinimumHeight(260);
    customPlot_->axisRect()->setMinimumMargins(QMargins(48, 12, 18, 42));
    customPlot_->axisRect()->setAutoMargins(QCP::msLeft | QCP::msBottom);
    customPlot_->xAxis->setLabel(tr("采样序列"));
    customPlot_->yAxis->setLabel(tr("归一化值"));
    customPlot_->yAxis->setRange(0.0, 1.0);
    layout->addWidget(customPlot_, 1);

    auto *legendRow = new QWidget(this);
    legendRow->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    legendRow->setMaximumHeight(30);
    auto *legendLayout = new QHBoxLayout(legendRow);
    legendLayout->setContentsMargins(4, 0, 4, 0);
    legendLayout->setSpacing(12);

    auto addLegendItem = [&](const QString &labelText, const QColor &color, Qt::PenStyle penStyle) {
        auto *item = new QWidget(legendRow);
        item->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        auto *itemLayout = new QHBoxLayout(item);
        itemLayout->setContentsMargins(0, 0, 0, 0);
        itemLayout->setSpacing(4);

        auto *swatch = new QFrame(item);
        swatch->setFixedSize(20, 8);
        swatch->setStyleSheet(QString(
            "QFrame {"
            "  background: transparent;"
            "  border: none;"
            "  border-top: 3px %1 %2;"
            "}"
        )
            .arg(penStyle == Qt::DashLine ? QStringLiteral("dashed") : QStringLiteral("solid"))
            .arg(color.name(QColor::HexRgb)));

        auto *label = new QLabel(labelText, item);
        QFont labelFont = label->font();
        labelFont.setPointSize(qMax(9, labelFont.pointSize() - 1));
        label->setFont(labelFont);
        itemLayout->addWidget(swatch);
        itemLayout->addWidget(label);
        legendLayout->addWidget(item);
    };

    addLegendItem(tr("温度"), QColor(220, 80, 80), Qt::SolidLine);
    addLegendItem(tr("湿度"), QColor(80, 140, 220), Qt::SolidLine);
    addLegendItem(tr("PM2.5"), QColor(240, 170, 60), Qt::SolidLine);
    addLegendItem(tr("CO2"), QColor(100, 180, 120), Qt::SolidLine);
    addLegendItem(tr("预测"), palette().color(QPalette::Text), Qt::DashLine);
    legendLayout->addStretch(1);
    layout->addWidget(legendRow, 0);
#endif
}

void LineChartWidget::addSample(const EnvSample &sample)
{
    temp_.push_back(sample.temperature);
    hum_.push_back(sample.humidity);
    pm_.push_back(sample.pm25);
    co2_.push_back(sample.co2);

    if (temp_.size() > maxPoints_) {
        temp_.removeFirst();
        hum_.removeFirst();
        pm_.removeFirst();
        co2_.removeFirst();
    }

#ifdef NATURAL_MONITOR_USE_QCUSTOMPLOT
    refreshCustomPlot();
#endif
    update();
}

void LineChartWidget::clear()
{
    temp_.clear();
    hum_.clear();
    pm_.clear();
    co2_.clear();

#ifdef NATURAL_MONITOR_USE_QCUSTOMPLOT
    refreshCustomPlot();
#endif
    update();
}

void LineChartWidget::setMaxPoints(int maxPoints)
{
    if (maxPoints <= 1) {
        return;
    }
    maxPoints_ = maxPoints;

#ifdef NATURAL_MONITOR_USE_QCUSTOMPLOT
    refreshCustomPlot();
#endif
}

QVector<double> LineChartWidget::predictSeries(const QVector<double> &series, double minValue, double maxValue) const
{
    QVector<double> predicted;
    if (series.isEmpty() || predictionPoints_ <= 0) {
        return predicted;
    }

    const int trendWindow = qMin(5, series.size() - 1);
    double averageDelta = 0.0;
    if (trendWindow > 0) {
        for (int i = series.size() - trendWindow; i < series.size(); ++i) {
            averageDelta += series.at(i) - series.at(i - 1);
        }
        averageDelta /= trendWindow;
    }

    double current = series.last();
    for (int i = 0; i < predictionPoints_; ++i) {
        current = clampValue(current + averageDelta, minValue, maxValue);
        predicted.push_back(current);
    }
    return predicted;
}

void LineChartWidget::refreshCustomPlot()
{
#ifdef NATURAL_MONITOR_USE_QCUSTOMPLOT
    if (!customPlot_) {
        return;
    }

    customPlot_->clearGraphs();
    struct SeriesMeta {
        QVector<double> values;
        QColor color;
        QString label;
        double minValue;
        double maxValue;
    };

    const QVector<SeriesMeta> seriesList = {
        {temp_, QColor(220, 80, 80), tr("温度"), kTempMin, kTempMax},
        {hum_, QColor(80, 140, 220), tr("湿度"), kHumMin, kHumMax},
        {pm_, QColor(240, 170, 60), tr("PM2.5"), kPmMin, kPmMax},
        {co2_, QColor(100, 180, 120), tr("CO2"), kCo2Min, kCo2Max}
    };

    const int totalPoints = qMax(maxPoints_, temp_.size() + predictionPoints_);
    customPlot_->xAxis->setRange(0, qMax(1, totalPoints - 1));

    for (const SeriesMeta &meta : seriesList) {
        QVector<double> x;
        QVector<double> y;
        for (int i = 0; i < meta.values.size(); ++i) {
            x.push_back(i);
            y.push_back(normalize(meta.values.at(i), meta.minValue, meta.maxValue));
        }

        auto *actualGraph = customPlot_->addGraph();
        actualGraph->setName(meta.label);
        actualGraph->setPen(QPen(meta.color, 2));
        actualGraph->setData(x, y);

        const QVector<double> predictedValues = predictSeries(meta.values, meta.minValue, meta.maxValue);
        if (!predictedValues.isEmpty() && !y.isEmpty()) {
            QVector<double> px;
            QVector<double> py;
            px.push_back(meta.values.size() - 1);
            py.push_back(y.last());
            for (int i = 0; i < predictedValues.size(); ++i) {
                px.push_back(meta.values.size() + i);
                py.push_back(normalize(predictedValues.at(i), meta.minValue, meta.maxValue));
            }

            auto *predictGraph = customPlot_->addGraph();
            predictGraph->setName(meta.label + tr("(预测)"));
            predictGraph->setPen(QPen(meta.color, 2, Qt::DashLine));
            predictGraph->setData(px, py);
        }
    }

    customPlot_->replot();
#endif
}

void LineChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

#ifdef NATURAL_MONITOR_USE_QCUSTOMPLOT
    QWidget::paintEvent(event);
    return;
#endif

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF bounds = rect();
    const QColor windowColor = palette().color(QPalette::Window);
    const QColor baseColor = palette().color(QPalette::Base);
    painter.fillRect(bounds, windowColor);

    const QRectF plot = bounds.adjusted(50, 20, -20, -40);
    painter.fillRect(plot, baseColor);
    painter.setPen(QPen(palette().color(QPalette::Midlight), 1));

    const int gridLines = 5;
    for (int i = 0; i <= gridLines; ++i) {
        const double y = plot.top() + (plot.height() / gridLines) * i;
        painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
    }

    painter.setPen(QPen(palette().color(QPalette::Mid), 1));
    painter.drawRect(plot);

    auto drawSeries = [&](const QVector<double> &series, const QVector<double> &predicted,
                          const QColor &color, double minValue, double maxValue) {
        if (series.size() < 2) {
            return;
        }
        const int totalPoints = qMax(maxPoints_, series.size() + predicted.size());
        QPainterPath path;
        for (int i = 0; i < series.size(); ++i) {
            const double x = plot.left() + (plot.width() * i / qMax(1, totalPoints - 1));
            const double t = normalize(series.at(i), minValue, maxValue);
            const double y = plot.bottom() - t * plot.height();
            if (i == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
        }
        painter.setPen(QPen(color, 2));
        painter.drawPath(path);

        if (!predicted.isEmpty()) {
            QPainterPath predictedPath;
            const double startX = plot.left() + (plot.width() * (series.size() - 1) / qMax(1, totalPoints - 1));
            const double startT = normalize(series.last(), minValue, maxValue);
            const double startY = plot.bottom() - startT * plot.height();
            predictedPath.moveTo(startX, startY);
            for (int i = 0; i < predicted.size(); ++i) {
                const double x = plot.left() + (plot.width() * (series.size() + i) / qMax(1, totalPoints - 1));
                const double t = normalize(predicted.at(i), minValue, maxValue);
                const double y = plot.bottom() - t * plot.height();
                predictedPath.lineTo(x, y);
            }
            painter.setPen(QPen(color, 2, Qt::DashLine));
            painter.drawPath(predictedPath);
        }
    };

    drawSeries(temp_, predictSeries(temp_, kTempMin, kTempMax), QColor(220, 80, 80), kTempMin, kTempMax);
    drawSeries(hum_, predictSeries(hum_, kHumMin, kHumMax), QColor(80, 140, 220), kHumMin, kHumMax);
    drawSeries(pm_, predictSeries(pm_, kPmMin, kPmMax), QColor(240, 170, 60), kPmMin, kPmMax);
    drawSeries(co2_, predictSeries(co2_, kCo2Min, kCo2Max), QColor(100, 180, 120), kCo2Min, kCo2Max);

    painter.setPen(QPen(palette().color(QPalette::Text), 1));
    painter.drawText(QRectF(plot.left(), plot.bottom() + 8, plot.width(), 20),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     tr("最近%1条采样数据，虚线表示接下来%2条预测").arg(maxPoints_).arg(predictionPoints_));

    const QPointF legendTop(plot.left(), plot.top() - 12);
    const int legendSpacing = 90;
    struct LegendItem { QColor color; QString label; };
    const QVector<LegendItem> items = {
        {QColor(220, 80, 80), tr("温度")},
        {QColor(80, 140, 220), tr("湿度")},
        {QColor(240, 170, 60), tr("PM2.5")},
        {QColor(100, 180, 120), tr("CO2")}
    };

    for (int i = 0; i < items.size(); ++i) {
        const double x = legendTop.x() + legendSpacing * i;
        painter.setPen(QPen(items[i].color, 3));
        painter.drawLine(QPointF(x, legendTop.y()), QPointF(x + 18, legendTop.y()));
        painter.setPen(palette().color(QPalette::Text));
        painter.drawText(QPointF(x + 22, legendTop.y() + 4), items[i].label);
    }
}
