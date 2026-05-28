#include "core/ClientSession.h"
#include "core/TcpServer.h"
#include "protocol/ChatProtocol.h"

#include <QDebug>

namespace chatroom::server {

ClientSession::ClientSession(QTcpSocket* socket, TcpServer* server, QObject* parent)
    : QObject(parent)
    , m_socket(socket)
    , m_server(server)
{
    Q_ASSERT(m_socket != nullptr);

    // Store the socket descriptor for cleanup purposes
    if (m_socket) {
        m_socketDescriptor = m_socket->socketDescriptor();
    }

    connect(m_socket, &QTcpSocket::readyRead,
            this, &ClientSession::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &ClientSession::onDisconnected);

    qDebug() << "ClientSession created for socket";
}

ClientSession::~ClientSession() {
    if (m_socket) {
        m_socket->disconnect(this);
        m_socket->abort();
        m_socket->deleteLater();
    }
    qDebug() << "ClientSession destroyed for user" << m_userId;
}

void ClientSession::setAuthenticated(uint64_t userId) {
    m_authenticated = true;
    m_userId = userId;
}

void ClientSession::sendPacket(MessageType type, uint8_t flags,
                               uint32_t sequence, const QByteArray& body) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send packet: socket not connected";
        return;
    }

    QByteArray packet = chatroom::protocol::ChatProtocol::pack(type, flags, sequence, body);
    qint64 written = m_socket->write(packet);
    if (written < 0) {
        qWarning() << "Failed to write packet to socket:" << m_socket->errorString();
    } else {
        m_socket->flush();
    }
}

void ClientSession::onReadyRead() {
    m_readBuffer.append(m_socket->readAll());
    processBuffer();
}

void ClientSession::onDisconnected() {
    qDebug() << "Client disconnected, user:" << m_userId;

    // If authenticated, remove from server's authenticated sessions
    if (m_authenticated && m_server) {
        m_server->removeSession(m_userId);
    }

    // If not authenticated, notify TcpServer to clean up pending session
    if (!m_authenticated) {
        emit pendingDisconnected(m_socketDescriptor);
    }

    emit disconnected();
}

void ClientSession::processBuffer() {
    using namespace chatroom::protocol;

    while (!m_readBuffer.isEmpty()) {
        PacketHeader header;
        QByteArray body;

        int consumed = ChatProtocol::parse(m_readBuffer, header, body);
        if (consumed <= 0) {
            // Not enough data for a complete packet, wait for more
            break;
        }

        // Validate magic number
        if (header.magic != MAGIC_NUMBER) {
            qWarning() << "Invalid magic number in packet, dropping connection";
            if (m_socket) {
                m_socket->disconnectFromHost();
            }
            m_readBuffer.clear();
            break;
        }

        // Emit the parsed packet
        auto msgType = static_cast<MessageType>(header.messageType);
        emit packetReceived(msgType, header.flags, header.sequence, body);

        // Remove consumed bytes from the buffer
        m_readBuffer.remove(0, consumed);
    }
}

} // namespace chatroom::server
