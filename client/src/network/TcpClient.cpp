#include "TcpClient.h"

#include <QAbstractSocket>

namespace client {

TcpClient::TcpClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, [this]() {
        emit dataReceived(m_socket->readAll());
    });
    connect(m_socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
        emit errorOccurred(m_socket->errorString());
    });
}

TcpClient::~TcpClient()
{
    disconnectFromServer();
}

bool TcpClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void TcpClient::connectToServer(const QString &host, quint16 port)
{
    if (isConnected())
        m_socket->disconnectFromHost();

    m_socket->connectToHost(host, port);
}

void TcpClient::disconnectFromServer()
{
    if (m_socket->state() == QAbstractSocket::UnconnectedState)
        return;

    m_socket->disconnectFromHost();
}

bool TcpClient::sendRawData(const QByteArray &data)
{
    if (!isConnected())
        return false;

    return m_socket->write(data) == data.size();
}

} // namespace client
