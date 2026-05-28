#include "UserController.h"
#include "core/AppManager.h"
#include "core/SessionManager.h"
#include "database/DatabaseManager.h"
#include "network/TcpClient.h"
#include "network/MessageHandler.h"
#include "network/HeartbeatManager.h"
#include "protocol/ChatProtocol.h"
#include "protocol/MessageTypes.h"
#include "crypto/CryptoUtils.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace chatroom::client {

UserController::UserController(AppManager* appManager, QObject* parent)
    : QObject(parent)
    , m_appManager(appManager)
{
    // Register handlers for login and register responses
    auto* handler = m_appManager->messageHandler();

    uint8_t loginRespType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::LoginResponse);
    handler->registerHandler(loginRespType,
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleLoginResponse(flags, seq, body);
        });

    uint8_t regRespType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::RegisterResponse);
    handler->registerHandler(regRespType,
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleRegisterResponse(flags, seq, body);
        });
}

void UserController::login(const QString& username, const QString& password)
{
    if (username.isEmpty() || password.isEmpty()) {
        emit loginFailed("Username and password cannot be empty");
        return;
    }

    // Hash the password using SHA-256 (same as server-side PasswordHasher)
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hash = chatroom::crypto::CryptoUtils::sha256(passwordBytes);
    QString passwordHash = QString(hash.toHex());

    // Build login request JSON
    QJsonObject bodyObj;
    bodyObj["username"] = username;
    bodyObj["passwordHash"] = passwordHash;
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::LoginRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "UserController: login request sent for user:" << username;

    // Save credentials for auto-reconnect
    m_appManager->sessionManager()->saveCredentials(username, passwordHash);
}

void UserController::registerUser(const QString& username, const QString& password,
                                   const QString& nickname)
{
    if (username.isEmpty() || password.isEmpty()) {
        emit registerFailed("Username and password cannot be empty");
        return;
    }

    // Hash the password
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hash = chatroom::crypto::CryptoUtils::sha256(passwordBytes);
    QString passwordHash = QString(hash.toHex());

    // Build register request JSON
    QJsonObject bodyObj;
    bodyObj["username"] = username;
    bodyObj["passwordHash"] = passwordHash;
    bodyObj["nickname"] = nickname.isEmpty() ? username : nickname;
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::RegisterRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "UserController: register request sent for user:" << username;
}

void UserController::logout()
{
    auto* session = m_appManager->sessionManager();
    auto* tcp = m_appManager->tcpClient();

    // Send logout request to server
    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::LogoutRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();
    tcp->sendPacket(type, flags, seq, QByteArray());

    // Stop heartbeat
    // (handled by AppManager's disconnected signal)

    // Update local state
    session->logout();

    // Save key pair to database before clearing
    m_appManager->databaseManager()->saveKeyPair(
        session->getLocalPublicKey(),
        session->getLocalPrivateKey());

    // Disconnect from server
    tcp->disconnectFromServer();

    qDebug() << "UserController: logged out";
}

void UserController::handleLoginResponse(uint8_t flags, uint32_t sequence,
                                          const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit loginFailed("Invalid server response");
        return;
    }

    QJsonObject obj = doc.object();
    bool success = obj["success"].toBool(false);

    if (!success) {
        QString reason = obj["reason"].toString("Login failed");
        emit loginFailed(reason);
        return;
    }

    quint64 userId = static_cast<quint64>(obj["userId"].toInteger(0));
    QString username = obj["username"].toString();
    QString nickname = obj["nickname"].toString();
    QString avatar = obj["avatar"].toString();
    int adminPort = obj["adminPort"].toInt(0);

    // Save admin port for avatar URL construction
    if (adminPort > 0) {
        m_appManager->setAdminPort(static_cast<quint16>(adminPort));
    }

    // Convert relative avatar path to full URL
    if (!avatar.isEmpty()) {
        avatar = m_appManager->toAvatarUrl(avatar);
    }

    auto* session = m_appManager->sessionManager();
    session->setLoggedIn(true, userId, username, nickname);
    session->setAvatar(avatar);

    // Generate key pair if not already present
    if (session->getLocalPublicKey().isEmpty()) {
        session->generateKeyPair();
        // Save to database
        m_appManager->databaseManager()->saveKeyPair(
            session->getLocalPublicKey(),
            session->getLocalPrivateKey());
    }

    // Start heartbeat
    // (We need access to HeartbeatManager - it's in AppManager, let's handle this
    //  through a signal or direct access)
    // For now, the heartbeat is started after login success is emitted

    qDebug() << "UserController: login successful, userId=" << userId
             << "username=" << username;
    emit loginSuccess();
}

void UserController::handleRegisterResponse(uint8_t flags, uint32_t sequence,
                                             const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit registerFailed("Invalid server response");
        return;
    }

    QJsonObject obj = doc.object();
    bool success = obj["success"].toBool(false);

    if (!success) {
        QString reason = obj["reason"].toString("Registration failed");
        emit registerFailed(reason);
        return;
    }

    qDebug() << "UserController: registration successful";
    emit registerSuccess();
}

} // namespace chatroom::client
