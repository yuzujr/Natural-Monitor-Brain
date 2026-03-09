#include "linechartwidget.h"

#include "databasemanager.h"

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
}

LineChartWidget::LineChartWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(220);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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

    update();
}

void LineChartWidget::clear()
{
    temp_.clear();
    hum_.clear();
    pm_.clear();
    co2_.clear();
    update();
}

void LineChartWidget::setMaxPoints(int maxPoints)
{
    if (maxPoints <= 1) {
        return;
    }
    maxPoints_ = maxPoints;
}

void LineChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

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

    auto drawSeries = [&](const QVector<double> &series, const QColor &color, double minValue, double maxValue) {
        if (series.size() < 2) {
            return;
        }
        QPainterPath path;
        for (int i = 0; i < series.size(); ++i) {
            const double x = plot.left() + (plot.width() * i / qMax(1, maxPoints_ - 1));
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
    };

    drawSeries(temp_, QColor(220, 80, 80), kTempMin, kTempMax);
    drawSeries(hum_, QColor(80, 140, 220), kHumMin, kHumMax);
    drawSeries(pm_, QColor(240, 170, 60), kPmMin, kPmMax);
    drawSeries(co2_, QColor(100, 180, 120), kCo2Min, kCo2Max);

    painter.setPen(QPen(palette().color(QPalette::Text), 1));
    painter.drawText(QRectF(plot.left(), plot.bottom() + 8, plot.width(), 20),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     tr("最近%1条采样数据").arg(maxPoints_));

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
