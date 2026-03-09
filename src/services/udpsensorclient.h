#ifndef UDPSENSORCLIENT_H
#define UDPSENSORCLIENT_H

#include <QObject>

#include "databasemanager.h"

class QTimer;
class QUdpSocket;

class UdpSensorClient : public QObject
{
    Q_OBJECT

public:
    explicit UdpSensorClient(QObject *parent = nullptr);

    void requestSample();
    bool isWaitingResponse() const;

signals:
    void sampleReceived(const EnvSample &sample);
    void requestFailed(const QString &message);

private slots:
    void handleReadyRead();
    void handleTimeout();

private:
    bool parseSample(const QByteArray &payload, EnvSample *sample, QString *errorMessage) const;

    QUdpSocket *socket_ = nullptr;
    QTimer *timeoutTimer_ = nullptr;
    bool waitingResponse_ = false;
};

#endif // UDPSENSORCLIENT_H