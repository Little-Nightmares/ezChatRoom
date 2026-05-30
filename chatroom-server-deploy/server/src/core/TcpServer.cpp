#include "core/TcpServer.h"
#include "core/ClientSession.h"
#include "core/MessageRouter.h"

#include <QTcpSocket>
#include <QDebug>

namespace chatroom::server {

TcpServer::TcpServer(QObject* parent)
    : QTcpServer(parent)
{
}

TcpServer::~TcpServer() {
    stop();
}

bool TcpServer::start(quint16 port) {
    if (!listen(QHostAddress::Any, port)) {
        qCritical() << "Failed to start server:" << errorString();
        return false;
    }
    qDebug() << "Server started on port" << port;
    return true;
}

void TcpServer::stop() {
    // Disconnect and delete all pending sessions
    for (auto it = m_pendingSessions.begin(); it != m_pendingSessions.end(); ++it) {
        ClientSession* session = it.value();
        if (session) {
            session->disconnect(this);
            session->deleteLater();
        }
    }
    m_pendingSessions.clear();

    // Disconnect and delete all authenticated sessions
    for (auto it = m_authenticatedSessions.begin(); it != m_authenticatedSessions.end(); ++it) {
        ClientSession* session = it.value();
        if (session) {
            session->disconnect(this);
            session->deleteLater();
        }
    }
    m_authenticatedSessions.clear();
    m_onlineUserIds.clear();

    close();
    qDebug() << "Server stopped";
}

ClientSession* TcpServer::getSession(uint64_t userId) const {
    return m_authenticatedSessions.value(userId, nullptr);
}

void TcpServer::removeSession(uint64_t userId) {
    auto it = m_authenticatedSessions.find(userId);
    if (it != m_authenticatedSessions.end()) {
        ClientSession* session = it.value();
        m_authenticatedSessions.erase(it);
        m_onlineUserIds.remove(userId);
        if (session) {
            session->deleteLater();
        }
        emit clientDisconnected(userId);
        qDebug() << "Session removed for user" << userId;
    }
}

void TcpServer::setRouter(MessageRouter* router) {
    m_router = router;
}

QMap<uint64_t, ClientSession*> TcpServer::getAuthenticatedSessions() const {
    return m_authenticatedSessions;
}

void TcpServer::moveToAuthenticated(qintptr socketDesc, uint64_t userId) {
    auto it = m_pendingSessions.find(socketDesc);
    if (it == m_pendingSessions.end()) {
        qWarning() << "Cannot move session to authenticated: pending session not found for socket descriptor" << socketDesc;
        return;
    }

    ClientSession* session = it.value();
    m_pendingSessions.erase(it);
    m_authenticatedSessions.insert(userId, session);
    m_onlineUserIds.insert(userId);

    emit clientConnected(userId);
    qDebug() << "Session moved to authenticated for user" << userId;
}

void TcpServer::incomingConnection(qintptr socketDescriptor) {
    // Create QTcpSocket and set the socket descriptor
    auto* socket = new QTcpSocket();
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qWarning() << "Failed to set socket descriptor:" << socket->errorString();
        delete socket;
        return;
    }

    // Create ClientSession for this connection
    // Note: socket is NOT parented to session here; ClientSession::onDisconnected
    // handles socket cleanup. The socket will be deleted when the session is deleted
    // because session->deleteLater() triggers ~ClientSession which calls socket->deleteLater().
    auto* session = new ClientSession(socket, this, socket);
    m_pendingSessions.insert(socketDescriptor, session);

    // Connect pendingDisconnected signal to clean up pending sessions when
    // an unauthenticated client disconnects
    connect(session, &ClientSession::pendingDisconnected, this, [this](qintptr desc) {
        m_pendingSessions.remove(desc);
    });

    // Register session with router if available
    if (m_router) {
        m_router->registerSession(session);
    }

    qDebug() << "New connection from"
             << socket->peerAddress().toString()
             << "port" << socket->peerPort();
}

} // namespace chatroom::server
