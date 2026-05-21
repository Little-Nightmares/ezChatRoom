#include "TcpServer.h"

#include <QDebug>
#include <QTcpSocket>

namespace server {

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
{
}

TcpServer::~TcpServer()
{
    stop();
}

bool TcpServer::start(quint16 port, const QHostAddress &address)
{
    if (isListening()) {
        qInfo() << "ChatRoom server already listening on"
                << serverAddress().toString() << serverPort();
        return true;
    }

    if (!listen(address, port)) {
        qCritical() << "Failed to start ChatRoom server on port" << port
                    << ":" << errorString();
        return false;
    }

    qInfo() << "ChatRoom server listening on"
            << serverAddress().toString() << serverPort();
    emit started(serverPort());
    return true;
}

void TcpServer::stop()
{
    if (!isListening())
        return;

    close();
    qInfo() << "ChatRoom server stopped";
    emit stopped();
}

bool TcpServer::isRunning() const
{
    return isListening();
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpServer::incomingConnection(socketDescriptor);
    emit clientAccepted(socketDescriptor);

    while (hasPendingConnections()) {
        QTcpSocket *socket = nextPendingConnection();
        socket->setParent(this);
        qInfo() << "Client connected from"
                << socket->peerAddress().toString() << socket->peerPort();
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    }
}

} // namespace server
