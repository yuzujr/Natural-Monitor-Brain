#include "SimulatorWidget.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

SimulatorWidget::SimulatorWidget(QWidget *parent)
    : QWidget(parent), isRunning(false)
{
    setupUI();
    udpSocket = new QUdpSocket(this);
    connect(udpSocket, &QUdpSocket::readyRead, this, &SimulatorWidget::processPendingDatagrams);
}

SimulatorWidget::~SimulatorWidget()
{
}

void SimulatorWidget::setupUI()
{
    this->setWindowTitle("环境传感器UDP模拟器");
    this->resize(450, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 1. 设置随机数范围区域
    QGroupBox *rangeGroup = new QGroupBox("设定随机数范围", this);
    QGridLayout *gridLayout = new QGridLayout(rangeGroup);

    gridLayout->addWidget(new QLabel("传感器"), 0, 0);
    gridLayout->addWidget(new QLabel("最小值"), 0, 1);
    gridLayout->addWidget(new QLabel("最大值"), 0, 2);

    // 温度 (Double)
    gridLayout->addWidget(new QLabel("温度 (℃):"), 1, 0);
    tempMinBox = new QDoubleSpinBox(); tempMinBox->setRange(-50, 100); tempMinBox->setValue(15.0);
    tempMaxBox = new QDoubleSpinBox(); tempMaxBox->setRange(-50, 100); tempMaxBox->setValue(35.0);
    gridLayout->addWidget(tempMinBox, 1, 1); gridLayout->addWidget(tempMaxBox, 1, 2);

    // 湿度 (Double)
    gridLayout->addWidget(new QLabel("湿度 (%):"), 2, 0);
    humMinBox = new QDoubleSpinBox(); humMinBox->setRange(0, 100); humMinBox->setValue(30.0);
    humMaxBox = new QDoubleSpinBox(); humMaxBox->setRange(0, 100); humMaxBox->setValue(80.0);
    gridLayout->addWidget(humMinBox, 2, 1); gridLayout->addWidget(humMaxBox, 2, 2);

    // PM2.5 (Int)
    gridLayout->addWidget(new QLabel("PM2.5 (ug/m3):"), 3, 0);
    pmMinBox = new QSpinBox(); pmMinBox->setRange(0, 999); pmMinBox->setValue(10);
    pmMaxBox = new QSpinBox(); pmMaxBox->setRange(0, 999); pmMaxBox->setValue(150);
    gridLayout->addWidget(pmMinBox, 3, 1); gridLayout->addWidget(pmMaxBox, 3, 2);

    // CO2 (Int)
    gridLayout->addWidget(new QLabel("CO2 (ppm):"), 4, 0);
    co2MinBox = new QSpinBox(); co2MinBox->setRange(0, 5000); co2MinBox->setValue(400);
    co2MaxBox = new QSpinBox(); co2MaxBox->setRange(0, 5000); co2MaxBox->setValue(1200);
    gridLayout->addWidget(co2MinBox, 4, 1); gridLayout->addWidget(co2MaxBox, 4, 2);

    mainLayout->addWidget(rangeGroup);

    // 2. 控制区域 (端口和启停)
    QGroupBox *ctrlGroup = new QGroupBox("服务控制", this);
    QGridLayout *ctrlLayout = new QGridLayout(ctrlGroup);

    ctrlLayout->addWidget(new QLabel("监听端口:"), 0, 0);
    portBox = new QSpinBox();
    portBox->setRange(1024, 65535);
    portBox->setValue(8888); // 默认端口8888
    ctrlLayout->addWidget(portBox, 0, 1);

    startBtn = new QPushButton("启动服务", this);
    connect(startBtn, &QPushButton::clicked, this, &SimulatorWidget::toggleServer);
    ctrlLayout->addWidget(startBtn, 0, 2);

    mainLayout->addWidget(ctrlGroup);

    // 3. 日志显示区域
    logConsole = new QTextEdit(this);
    logConsole->setReadOnly(true);
    mainLayout->addWidget(new QLabel("运行日志:"));
    mainLayout->addWidget(logConsole);
}

void SimulatorWidget::toggleServer()
{
    if (!isRunning) {
        port = portBox->value();
        if (udpSocket->bind(QHostAddress::Any, port)) {
            isRunning = true;
            isFirstRequest = true; // 新增：每次启动服务都重置状态
            startBtn->setText("停止服务");
            portBox->setEnabled(false);
            logMessage(QString("服务已启动，正在监听端口: %1").arg(port));
        } else {
            logMessage("错误：端口绑定失败，可能被占用！");
        }
    } else {
        udpSocket->close();
        isRunning = false;
        startBtn->setText("启动服务");
        portBox->setEnabled(true);
        logMessage("服务已停止。");
    }
}

void SimulatorWidget::updateSimulatedValues()
{
    if (isFirstRequest) {
        // 第一次请求，以用户设定的范围中间值作为初始基准点
        currentTemp = tempMinBox->value() + (tempMaxBox->value() - tempMinBox->value()) / 2.0;
        currentHum = humMinBox->value() + (humMaxBox->value() - humMinBox->value()) / 2.0;
        currentPm25 = pmMinBox->value() + (pmMaxBox->value() - pmMinBox->value()) / 2;
        currentCo2 = co2MinBox->value() + (co2MaxBox->value() - co2MinBox->value()) / 2;
        isFirstRequest = false;
    } else {
        // 后续请求，在当前值基础上进行小幅度随机游走 (Random Walk)

        // 温度：波动范围 [-0.5, +0.5]
        double tempStep = (QRandomGenerator::global()->generateDouble() * 1.0) - 0.5;
        currentTemp += tempStep;

        // 湿度：波动范围 [-1.0, +1.0]
        double humStep = (QRandomGenerator::global()->generateDouble() * 2.0) - 1.0;
        currentHum += humStep;

        // PM2.5：波动范围 [-3, +3] (注意 bounded 的上限是开区间，所以写 4)
        int pmStep = QRandomGenerator::global()->bounded(-3, 4);
        currentPm25 += pmStep;

        // CO2：波动范围 [-10, +10]
        int co2Step = QRandomGenerator::global()->bounded(-10, 11);
        currentCo2 += co2Step;
    }

    // 关键：使用 qBound 限制数值，防止长时间运行后数据漂移出用户设定的界限
    currentTemp = qBound(tempMinBox->value(), currentTemp, tempMaxBox->value());
    currentHum  = qBound(humMinBox->value(), currentHum, humMaxBox->value());
    currentPm25 = qBound(pmMinBox->value(), currentPm25, pmMaxBox->value());
    currentCo2  = qBound(co2MinBox->value(), currentCo2, co2MaxBox->value());
}


void SimulatorWidget::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(udpSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        logMessage(QString("收到来自 %1:%2 的请求").arg(sender.toString()).arg(senderPort));

        // 1. 更新环境数据状态（平滑渐变）
        updateSimulatedValues();

        // 2. 打包成 JSON
        QJsonObject jsonObj;
        jsonObj["temperature"] = QString::number(currentTemp, 'f', 1).toDouble();
        jsonObj["humidity"]    = QString::number(currentHum, 'f', 1).toDouble();
        jsonObj["pm25"]        = currentPm25;
        jsonObj["co2"]         = currentCo2;

        QJsonDocument jsonDoc(jsonObj);
        QByteArray responseData = jsonDoc.toJson(QJsonDocument::Compact);

        // 3. 返回数据
        udpSocket->writeDatagram(responseData, sender, senderPort);
        logMessage(QString("已返回数据: %1").arg(QString(responseData)));
    }
}




void SimulatorWidget::logMessage(const QString &msg)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    logConsole->append(QString("[%1] %2").arg(timestamp, msg));
}
