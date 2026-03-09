#include "udpsensorclient.h"

#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTimer>
#include <QUdpSocket>

namespace {
constexpr auto kServerIp = "127.0.0.1";
constexpr quint16 kServerPort = 8888;
constexpr int kRequestTimeoutMs = 1500;
}

UdpSensorClient::UdpSensorClient(QObject *parent)
    : QObject(parent)
    , socket_(new QUdpSocket(this))
    , timeoutTimer_(new QTimer(this))
{
    timeoutTimer_->setSingleShot(true);

    connect(socket_, &QUdpSocket::readyRead, this, &UdpSensorClient::handleReadyRead);
    connect(timeoutTimer_, &QTimer::timeout, this, &UdpSensorClient::handleTimeout);
}

void UdpSensorClient::requestSample()
{
    if (waitingResponse_) {
        return;
    }

    const qint64 written = socket_->writeDatagram("GET", QHostAddress(QString::fromLatin1(kServerIp)), kServerPort);
    if (written < 0) {
        emit requestFailed(tr("UDP 请求发送失败: %1").arg(socket_->errorString()));
        return;
    }

    waitingResponse_ = true;
    timeoutTimer_->start(kRequestTimeoutMs);
}

bool UdpSensorClient::isWaitingResponse() const
{
    return waitingResponse_;
}

void UdpSensorClient::handleReadyRead()
{
    while (socket_->hasPendingDatagrams()) {
        QByteArray payload;
        payload.resize(static_cast<int>(socket_->pendingDatagramSize()));
        socket_->readDatagram(payload.data(), payload.size());

        EnvSample sample;
        QString errorMessage;
        if (!parseSample(payload, &sample, &errorMessage)) {
            waitingResponse_ = false;
            timeoutTimer_->stop();
            emit requestFailed(errorMessage);
            return;
        }

        waitingResponse_ = false;
        timeoutTimer_->stop();
        emit sampleReceived(sample);
        return;
    }
}

void UdpSensorClient::handleTimeout()
{
    if (!waitingResponse_) {
        return;
    }

    waitingResponse_ = false;
    emit requestFailed(tr("UDP 请求超时，请检查本地服务 %1:%2 是否已启动").arg(QString::fromLatin1(kServerIp)).arg(kServerPort));
}

bool UdpSensorClient::parseSample(const QByteArray &payload, EnvSample *sample, QString *errorMessage) const
{
    if (!sample) {
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage) {
            *errorMessage = tr("收到的 UDP 数据不是有效 JSON: %1").arg(parseError.errorString());
        }
        return false;
    }

    const QJsonObject object = document.object();
    if (!object.contains(QStringLiteral("temperature"))
        || !object.contains(QStringLiteral("humidity"))
        || !object.contains(QStringLiteral("pm25"))
        || !object.contains(QStringLiteral("co2"))) {
        if (errorMessage) {
            *errorMessage = tr("JSON 字段不完整，缺少 temperature/humidity/pm25/co2");
        }
        return false;
    }

    sample->ts = QDateTime::currentDateTime();
    sample->temperature = object.value(QStringLiteral("temperature")).toDouble();
    sample->humidity = object.value(QStringLiteral("humidity")).toDouble();
    sample->pm25 = object.value(QStringLiteral("pm25")).toDouble();
    sample->co2 = object.value(QStringLiteral("co2")).toDouble();
    return true;
}