#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <cstdint>
#include "protocol/MessageTypes.h"

using chatroom::protocol::MessageType;

namespace chatroom::server {

class TcpServer;

class ClientSession : public QObject {
    Q_OBJECT
public:
    explicit ClientSession(QTcpSocket* socket, TcpServer* server,
                           QObject* parent = nullptr);
    ~ClientSession() override;

    uint64_t userId() const { return m_userId; }
    bool isAuthenticated() const { return m_authenticated; }
    QTcpSocket* socket() const { return m_socket; }

    void setAuthenticated(uint64_t userId);

    void sendPacket(MessageType type, uint8_t flags,
                    uint32_t sequence, const QByteArray& body);

signals:
    void packetReceived(MessageType type, uint8_t flags, uint32_t sequence,
                        const QByteArray& body);
    void disconnected();
    void pendingDisconnected(qintptr socketDescriptor);

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    void processBuffer();

    QTcpSocket* m_socket = nullptr;
    TcpServer* m_server = nullptr;
    QByteArray m_readBuffer;
    bool m_authenticated = false;
    uint64_t m_userId = 0;
    qintptr m_socketDescriptor = -1;
};

} // namespace chatroom::server
