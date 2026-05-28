#include "HeartbeatManager.h"
#include "TcpClient.h"
#include "protocol/ChatProtocol.h"
#include "protocol/MessageTypes.h"

#include <QDebug>

namespace chatroom::client {

HeartbeatManager::HeartbeatManager(TcpClient* tcpClient, QObject* parent)
    : QObject(parent)
    , m_tcpClient(tcpClient)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(INTERVAL_MS);
    connect(m_timer, &QTimer::timeout, this, &HeartbeatManager::sendHeartbeat);
}

void HeartbeatManager::start()
{
    m_missedCount = 0;
    m_timer->start();
    qDebug() << "HeartbeatManager: started (interval:" << INTERVAL_MS << "ms)";
}

void HeartbeatManager::stop()
{
    m_timer->stop();
    m_missedCount = 0;
    qDebug() << "HeartbeatManager: stopped";
}

void HeartbeatManager::sendHeartbeat()
{
    if (!m_tcpClient || !m_tcpClient->isConnected()) {
        stop();
        return;
    }

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::Heartbeat);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();
    m_tcpClient->sendPacket(type, 0, seq, QByteArray());

    m_missedCount++;
    qDebug() << "HeartbeatManager: sent heartbeat, missed count:" << m_missedCount;

    if (m_missedCount >= MAX_MISSES) {
        qWarning() << "HeartbeatManager: timeout after" << MAX_MISSES << "missed heartbeats";
        stop();
        emit heartbeatTimeout();
    }
}

void HeartbeatManager::onHeartbeatAckReceived()
{
    m_missedCount = 0;
    qDebug() << "HeartbeatManager: heartbeat ack received, reset missed count";
}

} // namespace chatroom::client
