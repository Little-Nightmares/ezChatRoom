#include "UserController.h"

#include "network/TcpClient.h"

namespace client {

UserController::UserController(QObject *parent)
    : QObject(parent)
    , m_client(new TcpClient(this))
{
    connect(m_client, &TcpClient::connected, this, &UserController::serverConnected);
    connect(m_client, &TcpClient::disconnected, this, &UserController::serverDisconnected);
    connect(m_client, &TcpClient::errorOccurred, this, &UserController::loginFailed);
}

UserController::~UserController()
{
}

void UserController::login(const QString &username,
                           const QString &password,
                           const QString &host,
                           int port)
{
    const QString trimmedUsername = username.trimmed();
    const QString trimmedHost = host.trimmed();

    if (trimmedUsername.isEmpty()) {
        emit loginFailed(tr("Username is required."));
        return;
    }

    if (password.isEmpty()) {
        emit loginFailed(tr("Password is required."));
        return;
    }

    if (trimmedHost.isEmpty()) {
        emit loginFailed(tr("Server address is required."));
        return;
    }

    if (port < 1 || port > 65535) {
        emit loginFailed(tr("Server port is invalid."));
        return;
    }

    emit loginStarted(trimmedUsername, trimmedHost, port);
    m_client->connectToServer(trimmedHost, static_cast<quint16>(port));
}

void UserController::logout()
{
    m_client->disconnectFromServer();
    emit logoutRequested();
}

} // namespace client
