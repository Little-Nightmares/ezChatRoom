#include "AppManager.h"
#include "network/TcpClient.h"
#include "network/MessageHandler.h"
#include "network/HeartbeatManager.h"
#include "SessionManager.h"
#include "database/DatabaseManager.h"
#include "protocol/MessageTypes.h"

#include <QStandardPaths>
#include <QDebug>

namespace chatroom::client {

AppManager::AppManager(QObject* parent)
    : QObject(parent)
    , m_tcpClient(std::make_unique<TcpClient>(this))
    , m_messageHandler(std::make_unique<MessageHandler>(this))
    , m_heartbeatManager(std::make_unique<HeartbeatManager>(m_tcpClient.get(), this))
    , m_sessionManager(std::make_unique<SessionManager>(this))
    , m_databaseManager(std::make_unique<DatabaseManager>())
{
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &AppManager::attemptReconnect);
}

AppManager::~AppManager()
{
}

void AppManager::initialize()
{
    // Initialize database
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                     + "/chatroom_client.db";
    if (!m_databaseManager->initialize(dbPath)) {
        qCritical() << "AppManager: failed to initialize database at" << dbPath;
    } else {
        qDebug() << "AppManager: database initialized at" << dbPath;
    }

    // Load saved key pair from database
    auto [pubKey, privKey] = m_databaseManager->loadKeyPair();
    if (!pubKey.isEmpty() && !privKey.isEmpty()) {
        m_sessionManager->setLocalPublicKey(pubKey);
        m_sessionManager->setLocalPrivateKey(privKey);
        qDebug() << "AppManager: loaded saved key pair from database";
    } else {
        qDebug() << "AppManager: no saved key pair found, will generate on login";
    }

    // Wire TcpClient::packetReceived -> MessageHandler::handlePacket
    connect(m_tcpClient.get(), &TcpClient::packetReceived,
            m_messageHandler.get(), &MessageHandler::handlePacket);

    // Register heartbeat ack handler
    uint8_t hbAckType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::HeartbeatAck);
    m_messageHandler->registerHandler(hbAckType,
        [this](uint8_t /*flags*/, uint32_t /*sequence*/, const QByteArray& /*body*/) {
            m_heartbeatManager->onHeartbeatAckReceived();
        });

    // Handle heartbeat timeout -> disconnect
    connect(m_heartbeatManager.get(), &HeartbeatManager::heartbeatTimeout,
            this, [this]() {
        qWarning() << "AppManager: heartbeat timeout, disconnecting";
        m_tcpClient->disconnectFromServer();
    });

    // Handle disconnection -> stop heartbeat and trigger reconnect
    connect(m_tcpClient.get(), &TcpClient::disconnected,
            this, [this]() {
        m_heartbeatManager->stop();
        // Trigger reconnect if not intentional logout and not already reconnecting
        if (m_sessionManager->isLoggedIn() && !m_lastHost.isEmpty() && !m_reconnecting) {
            m_reconnecting = true;
            m_reconnectTimer->start(1000); // Initial 1s delay, subsequent delays managed by attemptReconnect
            emit reconnectingChanged();
        }
    });

    // Handle connection success -> reset reconnect state
    connect(m_tcpClient.get(), &TcpClient::connected,
            this, [this]() {
        bool wasReconnecting = m_reconnecting;
        m_reconnectAttempt = 0;
        m_reconnecting = false;
        m_reconnectTimer->stop();
        emit reconnectingChanged();
        // If this was a reconnect (not initial login), notify to re-authenticate
        if (wasReconnecting) {
            qDebug() << "AppManager: reconnected successfully, triggering re-authentication";
            emit reconnectSuccess();
        }
    });

    qDebug() << "AppManager: initialized";
}

void AppManager::connectToServer(const QString& host, quint16 port)
{
    m_lastHost = host;
    m_lastPort = port;
    m_tcpClient->connectToServer(host, port);
}

void AppManager::attemptReconnect()
{
    if (m_reconnectAttempt >= MAX_RECONNECT_ATTEMPTS) {
        m_reconnecting = false;
        m_reconnectTimer->stop();
        emit reconnectFailed();
        emit reconnectingChanged();
        return;
    }

    m_reconnecting = true;
    m_reconnectAttempt++;
    emit reconnectingChanged();

    // Exponential backoff: 1s, 2s, 4s, 8s, 16s
    int delay = (1 << (m_reconnectAttempt - 1)) * 1000;
    m_reconnectTimer->start(delay);

    qDebug() << "Reconnect attempt" << m_reconnectAttempt << "in" << delay << "ms";

    // Attempt connection
    m_tcpClient->connectToServer(m_lastHost, m_lastPort);
}

QString AppManager::toAvatarUrl(const QString& relativePath) const
{
    if (relativePath.isEmpty()) return "";
    if (relativePath.startsWith("http://") || relativePath.startsWith("https://") ||
        relativePath.startsWith("file:///") || relativePath.startsWith("qrc:/")) {
        return relativePath;
    }
    if (m_adminPort > 0 && !m_lastHost.isEmpty()) {
        return QString("http://%1:%2/%3").arg(m_lastHost).arg(m_adminPort).arg(relativePath);
    }
    return relativePath;
}

void AppManager::setReconnecting(bool reconnecting)
{
    if (m_reconnecting != reconnecting) {
        m_reconnecting = reconnecting;
        emit reconnectingChanged();
    }
}

TcpClient* AppManager::tcpClient() const
{
    return m_tcpClient.get();
}

SessionManager* AppManager::sessionManager() const
{
    return m_sessionManager.get();
}

DatabaseManager* AppManager::databaseManager() const
{
    return m_databaseManager.get();
}

MessageHandler* AppManager::messageHandler() const
{
    return m_messageHandler.get();
}

HeartbeatManager* AppManager::heartbeatManager() const
{
    return m_heartbeatManager.get();
}

} // namespace chatroom::client
