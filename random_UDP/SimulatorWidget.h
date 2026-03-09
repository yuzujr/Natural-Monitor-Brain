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
    void updateSimulatedValues(); // 新增：专门用来更新带有连续性的环境数据
    double generateRandomDouble(double min, double max);
    int generateRandomInt(int min, int max);
    void logMessage(const QString &msg);

    // UDP Socket
    QUdpSocket *udpSocket;
    quint16 port;
    bool isRunning;

    // 新增：保存当前传感器数值的变量
    double currentTemp;
    double currentHum;
    int currentPm25;
    int currentCo2;
    bool isFirstRequest; // 标记是否是第一次请求

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
