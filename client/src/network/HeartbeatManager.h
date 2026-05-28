#pragma once

#include <QObject>
#include <QTimer>

namespace chatroom::client {

class TcpClient;

class HeartbeatManager : public QObject {
    Q_OBJECT
public:
    explicit HeartbeatManager(TcpClient* tcpClient, QObject* parent = nullptr);

    void start();
    void stop();
    void onHeartbeatAckReceived();

signals:
    void heartbeatTimeout();

private slots:
    void sendHeartbeat();

private:
    TcpClient* m_tcpClient = nullptr;
    QTimer* m_timer = nullptr;
    int m_missedCount = 0;
    static constexpr int INTERVAL_MS = 30000;   // 30 seconds
    static constexpr int MAX_MISSES = 3;         // 3 misses = timeout
};

} // namespace chatroom::client
