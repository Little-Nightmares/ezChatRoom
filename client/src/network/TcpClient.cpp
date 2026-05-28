#include "TcpClient.h"
#include "protocol/ChatProtocol.h"

#include <QDebug>

namespace chatroom::client {

TcpClient::TcpClient(QObject* parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpClient::onError);
}

TcpClient::~TcpClient()
{
    disconnectFromServer();
}

void TcpClient::connectToServer(const QString& host, quint16 port)
{
    m_host = host;
    m_port = port;
    m_reconnectAttempt = 0;
    qDebug() << "Connecting to server:" << host << "port:" << port;
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }
    m_socket->connectToHost(host, port);
}

void TcpClient::disconnectFromServer()
{
    m_autoReconnect = false; // Don't auto-reconnect on manual disconnect
    m_pendingQueue.clear();
    m_reconnectAttempt = 0;
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

bool TcpClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void TcpClient::sendRaw(const QByteArray& data)
{
    if (!isConnected()) {
        qWarning() << "TcpClient::sendRaw: not connected";
        return;
    }
    m_socket->write(data);
    m_socket->flush();
}

void TcpClient::sendPacket(uint8_t messageType, uint8_t flags,
                           uint32_t sequence, const QByteArray& body)
{
    if (!isConnected() && m_autoReconnect) {
        // Queue packet for later delivery when disconnected
        PendingPacket p;
        p.type = messageType;
        p.flags = flags;
        p.sequence = sequence;
        p.body = body;
        p.createdAt = QDateTime::currentDateTime();
        m_pendingQueue.enqueue(p);
        return;
    }

    QByteArray packet = chatroom::protocol::ChatProtocol::pack(
        static_cast<chatroom::protocol::MessageType>(messageType),
        flags, sequence, body);
    sendRaw(packet);
}

void TcpClient::onConnected()
{
    qDebug() << "TcpClient: connected to server";
    m_readBuffer.clear();
    m_reconnectAttempt = 0;

    // Flush any queued packets from disconnection period
    while (!m_pendingQueue.isEmpty()) {
        auto p = m_pendingQueue.dequeue();
        QByteArray packet = chatroom::protocol::ChatProtocol::pack(
            static_cast<chatroom::protocol::MessageType>(p.type),
            p.flags, p.sequence, p.body);
        m_socket->write(packet);
    }
    m_socket->flush();

    emit connected();
}

void TcpClient::onDisconnected()
{
    qDebug() << "TcpClient: disconnected from server";
    m_readBuffer.clear();
    emit disconnected();
}

void TcpClient::setAutoReconnect(bool enabled)
{
    m_autoReconnect = enabled;
    if (!enabled) {
        m_pendingQueue.clear();
    }
}

void TcpClient::onReadyRead()
{
    m_readBuffer.append(m_socket->readAll());
    processBuffer();
}

void TcpClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    QString errorString = m_socket->errorString();
    qWarning() << "TcpClient error:" << errorString;
    emit connectionError(errorString);
}

void TcpClient::processBuffer()
{
    using namespace chatroom::protocol;

    while (true) {
        PacketHeader header;
        QByteArray body;

        int consumed = ChatProtocol::parse(m_readBuffer, header, body);
        if (consumed < 0) {
            // Protocol error, clear buffer to recover
            m_readBuffer.clear();
            break;
        }
        if (consumed == 0) {
            break; // Not enough data
        }

        emit packetReceived(header.messageType, header.flags,
                            header.sequence, body);
        m_readBuffer.remove(0, consumed);
    }
}

} // namespace chatroom::client
