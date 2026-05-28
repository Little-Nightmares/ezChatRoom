#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QTimer>
#include <QQueue>
#include <QDateTime>
#include <cstdint>

namespace chatroom::client {

struct PendingPacket {
    uint8_t type;
    uint8_t flags;
    uint32_t sequence;
    QByteArray body;
    QDateTime createdAt;
};

class TcpClient : public QObject {
    Q_OBJECT
public:
    explicit TcpClient(QObject* parent = nullptr);
    ~TcpClient();

    void connectToServer(const QString& host, quint16 port = 6667);
    void disconnectFromServer();
    bool isConnected() const;

    void setAutoReconnect(bool enabled);
    bool autoReconnect() const { return m_autoReconnect; }
    int reconnectAttempt() const { return m_reconnectAttempt; }
    void resetReconnectAttempt() { m_reconnectAttempt = 0; }

    void sendRaw(const QByteArray& data);
    void sendPacket(uint8_t messageType, uint8_t flags,
                    uint32_t sequence, const QByteArray& body);
    int pendingQueueSize() const { return m_pendingQueue.size(); }

signals:
    void connected();
    void disconnected();
    void connectionError(const QString& error);
    void packetReceived(uint8_t messageType, uint8_t flags,
                        uint32_t sequence, const QByteArray& body);
    void reconnectStarted(int attempt);
    void reconnectFailed();

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    void processBuffer();

    QTcpSocket* m_socket = nullptr;
    QByteArray m_readBuffer;
    QString m_host;
    quint16 m_port = 6667;
    int m_reconnectAttempt = 0;
    bool m_autoReconnect = true;
    QQueue<PendingPacket> m_pendingQueue;
};

} // namespace chatroom::client
