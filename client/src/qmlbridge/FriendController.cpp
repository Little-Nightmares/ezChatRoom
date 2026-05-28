#include "FriendController.h"
#include "core/AppManager.h"
#include "core/SessionManager.h"
#include "database/DatabaseManager.h"
#include "network/TcpClient.h"
#include "network/MessageHandler.h"
#include "models/UserModel.h"
#include "models/FriendRequestModel.h"
#include "protocol/ChatProtocol.h"
#include "protocol/MessageTypes.h"
#include "models/UserInfo.h"
#include "models/FriendRequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace chatroom::client {

FriendController::FriendController(AppManager* appManager, QObject* parent)
    : QObject(parent)
    , m_appManager(appManager)
    , m_userModel(new UserModel(this))
    , m_friendRequestModel(new FriendRequestModel(this))
{
    auto* handler = m_appManager->messageHandler();

    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FriendSearchResponse),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            handleFriendSearchResponse(f, s, b);
        });

    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FriendAddResponse),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            handleFriendAddResponse(f, s, b);
        });

    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FriendAcceptResponse),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            handleFriendAcceptResponse(f, s, b);
        });

    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FriendRejectResponse),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            handleFriendRejectResponse(f, s, b);
        });

    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FriendDeleteResponse),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            handleFriendDeleteResponse(f, s, b);
        });

    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FriendListResponse),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            handleFriendListResponse(f, s, b);
        });

    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FriendRequestListResponse),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            handleFriendRequestListResponse(f, s, b);
        });

    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FriendRequestNotify),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            handleFriendRequestNotify(f, s, b);
        });

    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FriendOnlineNotify),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            handleFriendOnlineNotify(f, s, b);
        });

    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FriendOfflineNotify),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            handleFriendOfflineNotify(f, s, b);
        });
}

UserModel* FriendController::userModel() const
{
    return m_userModel;
}

FriendRequestModel* FriendController::friendRequestModel() const
{
    return m_friendRequestModel;
}

void FriendController::searchUser(const QString& keyword)
{
    if (keyword.isEmpty()) {
        return;
    }

    QJsonObject bodyObj;
    bodyObj["keyword"] = keyword;
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::FriendSearchRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "FriendController: search user request sent, keyword:" << keyword;
}

void FriendController::addFriend(quint64 userId, const QString& message)
{
    QJsonObject bodyObj;
    bodyObj["toUserId"] = static_cast<qint64>(userId);
    bodyObj["message"] = message;
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::FriendAddRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "FriendController: add friend request sent, targetUserId:" << userId;
}

void FriendController::acceptFriendRequest(quint64 requestId)
{
    QJsonObject bodyObj;
    bodyObj["requestId"] = static_cast<qint64>(requestId);
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::FriendAcceptRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "FriendController: accept friend request, requestId:" << requestId;
}

void FriendController::rejectFriendRequest(quint64 requestId)
{
    QJsonObject bodyObj;
    bodyObj["requestId"] = static_cast<qint64>(requestId);
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::FriendRejectRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "FriendController: reject friend request, requestId:" << requestId;
}

void FriendController::deleteFriend(quint64 friendId)
{
    QJsonObject bodyObj;
    bodyObj["friendId"] = static_cast<qint64>(friendId);
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::FriendDeleteRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "FriendController: delete friend request sent, friendId:" << friendId;
}

void FriendController::requestFriendList()
{
    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::FriendListRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, QByteArray());
    qDebug() << "FriendController: friend list request sent";
}

void FriendController::requestFriendRequestList()
{
    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::FriendRequestListRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, QByteArray());
    qDebug() << "FriendController: friend request list request sent";
}

void FriendController::handleFriendSearchResponse(uint8_t flags, uint32_t sequence,
                                                   const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        qWarning() << "FriendController: invalid friend search response";
        return;
    }

    QJsonObject obj = doc.object();
    QJsonArray usersArray = obj["users"].toArray();

    QVector<chatroom::models::UserInfo> users;
    users.reserve(usersArray.size());

    for (const auto& item : usersArray) {
        QJsonObject userObj = item.toObject();
        chatroom::models::UserInfo user;
        user.userId = static_cast<uint64_t>(userObj["userId"].toInteger(0));
        user.username = userObj["username"].toString();
        user.nickname = userObj["nickname"].toString();
        user.avatar = m_appManager->toAvatarUrl(userObj["avatar"].toString());
        user.isOnline = userObj["isOnline"].toBool(false);
        users.append(user);
    }

    m_userModel->setUsers(users);
    emit searchResultReady();
    qDebug() << "FriendController: search returned" << users.size() << "results";
}

void FriendController::handleFriendAddResponse(uint8_t flags, uint32_t sequence,
                                                const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        emit friendAdded(); // Still emit so UI can react
        return;
    }

    QJsonObject obj = doc.object();
    bool success = obj["success"].toBool(false);
    if (success) {
        qDebug() << "FriendController: friend request sent successfully";
    } else {
        qWarning() << "FriendController: friend request failed:"
                   << obj["reason"].toString();
    }
    emit friendAdded();
}

void FriendController::handleFriendAcceptResponse(uint8_t flags, uint32_t sequence,
                                                   const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        emit friendRequestAccepted();
        return;
    }

    QJsonObject obj = doc.object();
    bool success = obj["success"].toBool(false);

    if (success) {
        // Cache the new friend's info
        quint64 friendId = static_cast<uint64_t>(obj["friendId"].toInteger(0));
        QString username = obj["username"].toString();
        QString nickname = obj["nickname"].toString();

        chatroom::models::UserInfo user;
        user.userId = friendId;
        user.username = username;
        user.nickname = nickname;
        m_appManager->databaseManager()->upsertUser(user);

        qDebug() << "FriendController: friend request accepted, friendId:" << friendId;
    } else {
        qWarning() << "FriendController: accept friend request failed:"
                   << obj["reason"].toString();
    }
    emit friendRequestAccepted();
}

void FriendController::handleFriendRejectResponse(uint8_t flags, uint32_t sequence,
                                                   const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    QJsonObject obj = doc.object();
    bool success = obj["success"].toBool(false);

    if (!success) {
        qWarning() << "FriendController: reject friend request failed:"
                   << obj["reason"].toString();
    }
    emit friendRequestRejected();
}

void FriendController::handleFriendDeleteResponse(uint8_t flags, uint32_t sequence,
                                                   const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    QJsonObject obj = doc.object();
    bool success = obj["success"].toBool(false);

    if (!success) {
        qWarning() << "FriendController: delete friend failed:"
                   << obj["reason"].toString();
    }
    emit friendDeleted();
}

void FriendController::handleFriendListResponse(uint8_t flags, uint32_t sequence,
                                                 const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        qWarning() << "FriendController: invalid friend list response";
        return;
    }

    QJsonObject obj = doc.object();
    QJsonArray friendsArray = obj["friends"].toArray();

    QVector<chatroom::models::UserInfo> friends;
    friends.reserve(friendsArray.size());

    for (const auto& item : friendsArray) {
        QJsonObject friendObj = item.toObject();
        chatroom::models::UserInfo user;
        user.userId = static_cast<uint64_t>(friendObj["userId"].toInteger(0));
        user.username = friendObj["username"].toString();
        user.nickname = friendObj["nickname"].toString();
        user.avatar = m_appManager->toAvatarUrl(friendObj["avatar"].toString());
        user.isOnline = friendObj["isOnline"].toBool(false);
        friends.append(user);

        // Cache in local database
        m_appManager->databaseManager()->upsertUser(user);
    }

    m_userModel->setUsers(friends);
    emit friendListReady();
    qDebug() << "FriendController: received friend list with" << friends.size() << "friends";
}

void FriendController::handleFriendRequestListResponse(uint8_t flags, uint32_t sequence,
                                                        const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        qWarning() << "FriendController: invalid friend request list response";
        return;
    }

    QJsonObject obj = doc.object();
    QJsonArray requestsArray = obj["requests"].toArray();

    QVector<chatroom::models::FriendRequest> requests;
    requests.reserve(requestsArray.size());

    for (const auto& item : requestsArray) {
        QJsonObject reqObj = item.toObject();
        chatroom::models::FriendRequest req;
        req.requestId = static_cast<uint64_t>(reqObj["requestId"].toInteger(0));
        req.fromUserId = static_cast<uint64_t>(reqObj["fromUserId"].toInteger(0));
        req.toUserId = static_cast<uint64_t>(reqObj["toUserId"].toInteger(0));
        req.fromUsername = reqObj["fromUsername"].toString();
        req.message = reqObj["message"].toString();
        int statusInt = reqObj["status"].toInt(0);
        req.status = static_cast<chatroom::models::RequestStatus>(statusInt);
        requests.append(req);
    }

    m_friendRequestModel->setRequests(requests);
    emit friendRequestListReady();
    qDebug() << "FriendController: received" << requests.size() << "friend requests";
}

void FriendController::handleFriendRequestNotify(uint8_t flags, uint32_t sequence,
                                                  const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        qWarning() << "FriendController: invalid friend request notify";
        return;
    }

    QJsonObject obj = doc.object();
    chatroom::models::FriendRequest req;
    req.requestId = static_cast<uint64_t>(obj["requestId"].toInteger(0));
    req.fromUserId = static_cast<uint64_t>(obj["fromUserId"].toInteger(0));
    req.fromUsername = obj["fromUsername"].toString();
    req.message = obj["message"].toString();
    req.status = chatroom::models::RequestStatus::Pending;
    req.createdAt = QDateTime::fromSecsSinceEpoch(obj["createdAt"].toInteger(0));

    // Re-request the full list from server to refresh the UI
    requestFriendRequestList();

    qDebug() << "FriendController: received new friend request from user"
              << req.fromUserId << "(" << req.fromUsername << ")";
    emit newFriendRequestReceived();
}

void FriendController::handleFriendOnlineNotify(uint8_t flags, uint32_t sequence,
                                                  const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        qWarning() << "FriendController: invalid online notify";
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t userId = static_cast<uint64_t>(obj["userId"].toInteger(0));

    m_userModel->updateOnlineStatus(userId, true);
    emit friendOnlineStatusChanged(userId, true);

    qDebug() << "FriendController: friend" << userId << "is now online";
}

void FriendController::handleFriendOfflineNotify(uint8_t flags, uint32_t sequence,
                                                   const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        qWarning() << "FriendController: invalid offline notify";
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t userId = static_cast<uint64_t>(obj["userId"].toInteger(0));

    m_userModel->updateOnlineStatus(userId, false);
    emit friendOnlineStatusChanged(userId, false);

    qDebug() << "FriendController: friend" << userId << "is now offline";
}

bool FriendController::isFriendOnline(quint64 userId) const
{
    return m_userModel ? m_userModel->isOnline(userId) : false;
}

} // namespace chatroom::client
