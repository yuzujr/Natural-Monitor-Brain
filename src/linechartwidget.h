#ifndef LINECHARTWIDGET_H
#define LINECHARTWIDGET_H

#include <QWidget>
#include <QVector>

struct EnvSample;
class QCustomPlot;
class QWidget;

class LineChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LineChartWidget(QWidget *parent = nullptr);

    void addSample(const EnvSample &sample);
    void clear();
    void setMaxPoints(int maxPoints);
    void refreshThemeStyles();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> predictSeries(const QVector<double> &series, double minValue, double maxValue) const;
    void refreshCustomPlot();

    QVector<double> temp_;
    QVector<double> hum_;
    QVector<double> pm_;
    QVector<double> co2_;
    int maxPoints_ = 60;
    int predictionPoints_ = 5;
    QCustomPlot *customPlot_ = nullptr;
    QWidget *legendRow_ = nullptr;
};

#endif // LINECHARTWIDGET_H
