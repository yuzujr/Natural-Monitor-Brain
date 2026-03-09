#ifndef SIMULATORWIDGET_H
#define SIMULATORWIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QPushButton>

class SimulatorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SimulatorWidget(QWidget *parent = nullptr);
    ~SimulatorWidget();

private slots:
    void processPendingDatagrams();
    void toggleServer();

private:
    void setupUI();
    double generateRandomDouble(double min, double max);
    int generateRandomInt(int min, int max);
    void logMessage(const QString &msg);

    // UDP Socket
    QUdpSocket *udpSocket;
    quint16 port;
    bool isRunning;

    // UI Elements
    QDoubleSpinBox *tempMinBox, *tempMaxBox;
    QDoubleSpinBox *humMinBox, *humMaxBox;
    QSpinBox *pmMinBox, *pmMaxBox;
    QSpinBox *co2MinBox, *co2MaxBox;
    QSpinBox *portBox;
    QPushButton *startBtn;
    QTextEdit *logConsole;
};

#endif // SIMULATORWIDGET_H
