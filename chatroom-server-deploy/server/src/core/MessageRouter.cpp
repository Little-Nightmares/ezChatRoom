#include "core/MessageRouter.h"
#include "core/TcpServer.h"
#include "core/ClientSession.h"
#include "database/ServerDatabase.h"
#include "protocol/ChatProtocol.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QCryptographicHash>
#include <QTimer>
#include <QSet>
#include "core/BotService.h"
#include "crypto/CryptoUtils.h"

using namespace chatroom::protocol;
using chatroom::models::UserInfo;
using chatroom::models::ChatMessage;
using chatroom::models::FriendRequest;
using chatroom::models::RequestStatus;
using chatroom::models::ChatMessageType;
namespace chatroom::server {

MessageRouter::MessageRouter(ServerDatabase* db, TcpServer* server, QObject* parent)
    : QObject(parent)
    , m_db(db)
    , m_server(server)
{
}

void MessageRouter::registerSession(ClientSession* session) {
    // Connect session's packetReceived signal to onPacketReceived via lambda
    // The lambda captures the session pointer so we know which session sent the packet
    connect(session, &ClientSession::packetReceived,
            this, [this, session](MessageType type, uint8_t flags, uint32_t sequence,
                                  const QByteArray& body) {
                onPacketReceived(type, flags, sequence, body, session);
            });
}

void MessageRouter::onPacketReceived(MessageType type, uint8_t flags, uint32_t sequence,
                                     const QByteArray& body, ClientSession* session) {
    switch (type) {
    case MessageType::LoginRequest:
        handleLogin(session, sequence, body);
        break;
    case MessageType::RegisterRequest:
        handleRegister(session, sequence, body);
        break;
    case MessageType::LogoutRequest:
        handleLogout(session, sequence);
        break;
    case MessageType::Heartbeat:
        handleHeartbeat(session, sequence);
        break;
    case MessageType::ChatMessage:
        handleChatMessage(session, sequence, body);
        break;
    case MessageType::FriendSearchRequest:
        handleFriendSearch(session, sequence, body);
        break;
    case MessageType::FriendAddRequest:
        handleFriendAdd(session, sequence, body);
        break;
    case MessageType::FriendAcceptRequest:
        handleFriendAccept(session, sequence, body);
        break;
    case MessageType::FriendRejectRequest:
        handleFriendReject(session, sequence, body);
        break;
    case MessageType::FriendDeleteRequest:
        handleFriendDelete(session, sequence, body);
        break;
    case MessageType::FriendListRequest:
        handleFriendList(session, sequence);
        break;
    case MessageType::FriendRequestListRequest:
        handleFriendRequestList(session, sequence);
        break;
    case MessageType::KeyExchangeRequest:
        handleKeyExchange(session, sequence, body);
        break;
    case MessageType::MessageRecallRequest:
        handleMessageRecall(session, sequence, body);
        break;
    case MessageType::MessageRecallNotify:
        // MessageRecallNotify is a client-to-server forwarding message.
        // The server forwards the recall notification to the target user.
        // Actual forwarding logic is handled in handleMessageRecall.
        qWarning() << "Received MessageRecallNotify directly from client, ignoring";
        break;
    case MessageType::AvatarUploadRequest:
        handleAvatarUpload(session, sequence, body);
        break;
    case MessageType::GroupCreateRequest:
        handleGroupCreate(session, sequence, body);
        break;
    case MessageType::GroupListRequest:
        handleGroupList(session, sequence);
        break;
    case MessageType::GroupInfoRequest:
        handleGroupInfo(session, sequence, body);
        break;
    case MessageType::GroupMessage:
        handleGroupMessage(session, sequence, body);
        // Auto-respond to @bot mentions
        if (m_botService) {
            // Bot mention handling is done inside handleGroupMessage
        }
        break;
    case MessageType::GroupMemberAddRequest:
        handleGroupMemberAdd(session, sequence, body);
        break;
    case MessageType::GroupMemberRemoveRequest:
        handleGroupMemberRemove(session, sequence, body);
        break;
    case MessageType::GroupUpdateRequest:
        handleGroupUpdate(session, sequence, body);
        break;
    case MessageType::GroupLeaveRequest:
        handleGroupLeave(session, sequence, body);
        break;
    case MessageType::GroupDissolveRequest:
        handleGroupDissolve(session, sequence, body);
        break;
    case MessageType::GroupAvatarUploadReq:
        handleGroupAvatarUpload(session, sequence, body);
        break;
    case MessageType::GroupTransferOwnerReq:
        handleGroupTransferOwner(session, sequence, body);
        break;

    // --- File transfer ---
    case MessageType::FileUploadRequest:
        handleFileUpload(session, sequence, body);
        break;
    case MessageType::FileDownloadRequest:
        handleFileDownload(session, sequence, body);
        break;
    case MessageType::FileChunkTransfer:
        handleFileChunk(session, sequence, body);
        break;

    // --- Message search ---
    case MessageType::MessageSearchRequest:
        handleMessageSearch(session, sequence, body);
        break;

    // --- Group enhancements ---
    case MessageType::GroupAnnouncementRequest:
        handleGroupAnnouncement(session, sequence, body);
        break;
    case MessageType::GroupMessageReadNotify:
        handleGroupMessageRead(session, sequence, body);
        break;
    case MessageType::GroupKeyRequest:
        handleGroupKeyRequest(session, sequence, body);
        break;

    // --- Message features ---
    case MessageType::MessageReply:
        handleMessageReply(session, sequence, body);
        break;

    default:
        qWarning() << "Unknown message type:" << static_cast<int>(type);
        break;
    }
}

void MessageRouter::handleLogin(ClientSession* session, uint32_t seq, const QByteArray& body) {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::LoginResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    QString username = obj["username"].toString();
    QString passwordHash = obj["passwordHash"].toString();

    if (username.isEmpty() || passwordHash.isEmpty()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Username and password are required";
        sendResponse(session, MessageType::LoginResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t userId = m_db->authenticateUser(username, passwordHash);

    QJsonObject response;
    if (userId != 0) {
        UserInfo userInfo = m_db->getUserInfo(userId);
        response["success"] = true;
        response["userId"] = static_cast<qint64>(userInfo.userId);
        response["nickname"] = userInfo.nickname;
        response["username"] = userInfo.username;
        response["avatar"] = avatarUrl(userInfo.avatar);
        if (m_adminPort > 0) {
            response["adminPort"] = m_adminPort;
        }

        // Mark session as authenticated and move to authenticated map
        session->setAuthenticated(userId);

        // Find the socket descriptor to move the session
        if (session->socket()) {
            m_server->moveToAuthenticated(session->socket()->socketDescriptor(), userId);
        }

        // Send offline messages after authentication
        QVector<ChatMessage> offlineMsgs = m_db->getOfflineMessages(userId);
        for (const auto& msg : offlineMsgs) {
            QJsonObject msgObj;
            msgObj["senderId"] = static_cast<qint64>(msg.senderId);
            msgObj["receiverId"] = static_cast<qint64>(msg.receiverId);
            msgObj["content"] = msg.content;
            msgObj["timestamp"] = msg.timestamp.toSecsSinceEpoch();
            msgObj["type"] = static_cast<int>(msg.type);

            session->sendPacket(MessageType::OfflineMessagePush, 0,
                                ChatProtocol::nextSequence(),
                                QJsonDocument(msgObj).toJson(QJsonDocument::Compact));
        }

        // Notify client that all offline messages have been sent
        session->sendPacket(MessageType::OfflineMessageDone, 0,
                            ChatProtocol::nextSequence(), QByteArray());

        // Clear offline messages from database
        m_db->clearOfflineMessages(userId);

        // Auto-join default group
        auto defaultGroup = m_db->getDefaultGroup();
        if (defaultGroup.groupId > 0) {
            if (!m_db->isGroupMember(defaultGroup.groupId, userId)) {
                int memberCount = m_db->getGroupMemberCount(defaultGroup.groupId);
                if (memberCount < defaultGroup.maxMembers) {
                    m_db->addGroupMember(defaultGroup.groupId, userId);
                }
            }
        }

        // Push group offline messages
        {
            auto userGroups = m_db->getUserGroups(userId);
            QSqlQuery lastReadQuery(QSqlDatabase::database("chatroom_server"));
            for (const auto& group : userGroups) {
                lastReadQuery.prepare("SELECT last_read_msg_id FROM group_members WHERE group_id = ? AND user_id = ?");
                lastReadQuery.addBindValue(static_cast<qint64>(group.groupId));
                lastReadQuery.addBindValue(static_cast<qint64>(userId));
                if (lastReadQuery.exec() && lastReadQuery.next()) {
                    uint64_t lastReadId = lastReadQuery.value(0).toULongLong();
                    auto offlineMsgs = m_db->getGroupOfflineMessages(group.groupId, lastReadId);
                    for (const auto& msg : offlineMsgs) {
                        QJsonObject pushObj;
                        pushObj["groupId"] = static_cast<qint64>(group.groupId);
                        pushObj["senderId"] = static_cast<qint64>(msg.senderId);
                        pushObj["senderNickname"] = msg.senderNickname;
                        pushObj["content"] = msg.content;
                        pushObj["timestamp"] = static_cast<qint64>(msg.timestamp.toSecsSinceEpoch());
                        pushObj["type"] = static_cast<int>(msg.type);
                        QByteArray pushBody = QJsonDocument(pushObj).toJson(QJsonDocument::Compact);
                        session->sendPacket(MessageType::GroupOfflineMsgPush, 0, 0, pushBody);
                        lastReadId = msg.messageId;
                    }
                    if (!offlineMsgs.isEmpty()) {
                        m_db->updateLastReadMsgId(group.groupId, userId, lastReadId);
                    }
                }
            }
            // Send GroupOfflineMsgDone
            session->sendPacket(MessageType::GroupOfflineMsgDone, 0, 0, QByteArray());
        }

        qDebug() << "User logged in:" << username << "(userId:" << userId << ")";
    } else {
        response["success"] = false;
        response["message"] = "Invalid username or password";
        qDebug() << "Login failed for user:" << username;
    }

    sendResponse(session, MessageType::LoginResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleRegister(ClientSession* session, uint32_t seq, const QByteArray& body) {
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::RegisterResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    QString username = obj["username"].toString();
    QString passwordHash = obj["passwordHash"].toString();
    QString nickname = obj["nickname"].toString();

    if (username.isEmpty() || passwordHash.isEmpty()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Username and password are required";
        sendResponse(session, MessageType::RegisterResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    if (nickname.isEmpty()) {
        nickname = username;
    }

    int result = m_db->registerUser(username, passwordHash, nickname);

    QJsonObject response;
    if (result == 0) {
        response["success"] = true;
        response["message"] = "Registration successful";
        qDebug() << "User registered:" << username;
    } else if (result == 1) {
        response["success"] = false;
        response["message"] = "Username already exists";
        qDebug() << "Registration failed - username exists:" << username;
    } else {
        response["success"] = false;
        response["message"] = "Registration failed due to server error";
        qWarning() << "Registration failed - server error for user:" << username;
    }

    sendResponse(session, MessageType::RegisterResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleLogout(ClientSession* session, uint32_t seq) {
    if (session->isAuthenticated()) {
        uint64_t userId = session->userId();
        qDebug() << "User logging out:" << userId;
        m_server->removeSession(userId);
    }
}

void MessageRouter::handleHeartbeat(ClientSession* session, uint32_t seq) {
    // Respond with HeartbeatAck
    session->sendPacket(MessageType::HeartbeatAck, 0, seq, QByteArray());
}

void MessageRouter::handleChatMessage(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Invalid chat message JSON";
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t receiverId = static_cast<uint64_t>(obj["receiverId"].toInteger(0));
    QString content = obj["content"].toString();
    qint64 timestamp = obj["timestamp"].toInteger(0);
    int msgType = obj["type"].toInt(0);

    if (receiverId == 0 || content.isEmpty()) {
        qWarning() << "Invalid chat message: missing receiverId or content";
        return;
    }

    // Build the message object to forward
    QJsonObject forwardObj;
    forwardObj["senderId"] = static_cast<qint64>(session->userId());
    forwardObj["receiverId"] = static_cast<qint64>(receiverId);
    forwardObj["content"] = content;
    forwardObj["timestamp"] = timestamp;
    forwardObj["type"] = msgType;

    QByteArray forwardData = QJsonDocument(forwardObj).toJson(QJsonDocument::Compact);

    // Persist the message for search
    m_db->storeChatMessage(session->userId(), receiverId, content, msgType, timestamp);

    // Check if receiver is online
    ClientSession* receiverSession = m_server->getSession(receiverId);
    if (receiverSession) {
        // Receiver is online, forward the message directly
        receiverSession->sendPacket(MessageType::ChatMessage, 0,
                                    ChatProtocol::nextSequence(), forwardData);
    } else {
        // Receiver is offline, store the message in the database
        ChatMessage msg;
        msg.senderId = session->userId();
        msg.receiverId = receiverId;
        msg.content = content;
        msg.type = static_cast<ChatMessageType>(msgType);
        msg.timestamp = QDateTime::fromSecsSinceEpoch(timestamp);

        if (!m_db->storeOfflineMessage(msg)) {
            qWarning() << "Failed to store offline message for user" << receiverId;
        } else {
            qDebug() << "Offline message stored for user" << receiverId;
        }
    }

    // Send acknowledgment to sender
    QJsonObject ackObj;
    ackObj["success"] = true;
    sendResponse(session, MessageType::ChatMessageAck, seq,
                 QJsonDocument(ackObj).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleFriendSearch(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["users"] = QJsonArray();
        sendResponse(session, MessageType::FriendSearchResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    QString keyword = obj["keyword"].toString();

    QJsonObject response;
    QJsonArray usersArray;

    if (!keyword.isEmpty()) {
        // Use fuzzy search and filter out self and existing friends
        QVector<UserInfo> foundUsers = m_db->searchUsers(keyword);
        QVector<UserInfo> friendList = m_db->getFriendList(session->userId());

        // Build a set of friend user IDs for quick lookup
        QSet<uint64_t> friendIds;
        for (const auto& f : friendList) {
            friendIds.insert(f.userId);
        }

        for (const auto& u : foundUsers) {
            // Exclude self and existing friends
            if (u.userId == session->userId() || friendIds.contains(u.userId)) {
                continue;
            }
            QJsonObject userObj;
            userObj["userId"] = static_cast<qint64>(u.userId);
            userObj["username"] = u.username;
            userObj["nickname"] = u.nickname;
            userObj["avatar"] = avatarUrl(u.avatar);
            usersArray.append(userObj);
        }
    }

    response["users"] = usersArray;
    sendResponse(session, MessageType::FriendSearchResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleFriendAdd(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::FriendAddResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t toUserId = static_cast<uint64_t>(obj["toUserId"].toInteger(0));
    QString message = obj["message"].toString();

    if (toUserId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid target user ID";
        sendResponse(session, MessageType::FriendAddResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Check if already friends
    if (m_db->isFriend(session->userId(), toUserId)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Already friends with this user";
        sendResponse(session, MessageType::FriendAddResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t requestId = m_db->sendFriendRequest(session->userId(), toUserId, message);

    QJsonObject response;
    if (requestId > 0) {
        response["success"] = true;
        response["message"] = "Friend request sent";
        qDebug() << "Friend request sent from user" << session->userId() << "to user" << toUserId;

        // Notify target user if online
        ClientSession* targetSession = m_server->getSession(toUserId);
        if (targetSession) {
            UserInfo fromUser = m_db->getUserInfo(session->userId());
            QJsonObject notifyObj;
            notifyObj["requestId"] = static_cast<qint64>(requestId);
            notifyObj["fromUserId"] = static_cast<qint64>(session->userId());
            notifyObj["fromUsername"] = fromUser.username;
            notifyObj["message"] = message;
            notifyObj["createdAt"] = static_cast<qint64>(QDateTime::currentSecsSinceEpoch());
            targetSession->sendPacket(
                MessageType::FriendRequestNotify, 0,
                ChatProtocol::nextSequence(),
                QJsonDocument(notifyObj).toJson(QJsonDocument::Compact));
            qDebug() << "Friend request notify sent to online user" << toUserId;
        }
    } else {
        response["success"] = false;
        response["message"] = "Failed to send friend request";
    }

    sendResponse(session, MessageType::FriendAddResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleFriendAccept(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::FriendAcceptResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t requestId = static_cast<uint64_t>(obj["requestId"].toInteger(0));

    if (requestId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid request ID";
        sendResponse(session, MessageType::FriendAcceptResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    bool ok = m_db->acceptFriendRequest(requestId);

    QJsonObject response;
    if (ok) {
        response["success"] = true;
        response["message"] = "Friend request accepted";
        qDebug() << "Friend request accepted:" << requestId;
    } else {
        response["success"] = false;
        response["message"] = "Failed to accept friend request";
    }

    sendResponse(session, MessageType::FriendAcceptResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleFriendReject(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::FriendRejectResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t requestId = static_cast<uint64_t>(obj["requestId"].toInteger(0));

    if (requestId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid request ID";
        sendResponse(session, MessageType::FriendRejectResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    bool ok = m_db->rejectFriendRequest(requestId);

    QJsonObject response;
    if (ok) {
        response["success"] = true;
        response["message"] = "Friend request rejected";
        qDebug() << "Friend request rejected:" << requestId;
    } else {
        response["success"] = false;
        response["message"] = "Failed to reject friend request";
    }

    sendResponse(session, MessageType::FriendRejectResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleFriendDelete(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::FriendDeleteResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t friendId = static_cast<uint64_t>(obj["friendId"].toInteger(0));

    if (friendId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid friend ID";
        sendResponse(session, MessageType::FriendDeleteResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    bool ok = m_db->deleteFriend(session->userId(), friendId);

    QJsonObject response;
    if (ok) {
        response["success"] = true;
        response["message"] = "Friend deleted";
        qDebug() << "Friend deleted: user" << session->userId() << "removed friend" << friendId;
    } else {
        response["success"] = false;
        response["message"] = "Failed to delete friend";
    }

    sendResponse(session, MessageType::FriendDeleteResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleFriendList(ClientSession* session, uint32_t seq) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QVector<UserInfo> friends = m_db->getFriendList(session->userId());

    QJsonObject response;
    QJsonArray friendsArray;

    for (const auto& friendInfo : friends) {
        QJsonObject friendObj;
        friendObj["userId"] = static_cast<qint64>(friendInfo.userId);
        friendObj["username"] = friendInfo.username;
        friendObj["nickname"] = friendInfo.nickname;
        friendObj["avatar"] = avatarUrl(friendInfo.avatar);

        // Check if friend is online
        ClientSession* friendSession = m_server->getSession(friendInfo.userId);
        friendObj["isOnline"] = (friendSession != nullptr);

        friendsArray.append(friendObj);
    }

    response["friends"] = friendsArray;
    sendResponse(session, MessageType::FriendListResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleFriendRequestList(ClientSession* session, uint32_t seq) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QVector<FriendRequest> requests = m_db->getPendingFriendRequests(session->userId());

    QJsonObject response;
    QJsonArray requestsArray;

    for (const auto& req : requests) {
        QJsonObject reqObj;
        reqObj["requestId"] = static_cast<qint64>(req.requestId);
        reqObj["fromUserId"] = static_cast<qint64>(req.fromUserId);
        reqObj["fromUsername"] = req.fromUsername;
        reqObj["message"] = req.message;
        requestsArray.append(reqObj);
    }

    response["requests"] = requestsArray;
    sendResponse(session, MessageType::FriendRequestListResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleKeyExchange(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Invalid key exchange JSON";
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t targetUserId = static_cast<uint64_t>(obj["targetUserId"].toInteger(0));
    QString publicKey = obj["publicKey"].toString();

    if (targetUserId == 0 || publicKey.isEmpty()) {
        qWarning() << "Invalid key exchange: missing targetUserId or publicKey";
        return;
    }

    // Find the target session
    ClientSession* targetSession = m_server->getSession(targetUserId);
    if (targetSession) {
        // Target is online, forward the key exchange
        QJsonObject forwardObj;
        forwardObj["fromUserId"] = static_cast<qint64>(session->userId());

        // Get sender's username
        UserInfo senderInfo = m_db->getUserInfo(session->userId());
        forwardObj["fromUsername"] = senderInfo.username;

        forwardObj["publicKey"] = publicKey;

        QByteArray forwardData = QJsonDocument(forwardObj).toJson(QJsonDocument::Compact);
        targetSession->sendPacket(MessageType::KeyExchangeResponse, 0,
                                  ChatProtocol::nextSequence(), forwardData);

        qDebug() << "Key exchange forwarded from user" << session->userId()
                 << "to user" << targetUserId;
    } else {
        qWarning() << "Key exchange target user" << targetUserId << "is not online";
    }
}

void MessageRouter::handleMessageRecall(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::MessageRecallNotify, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t messageId = static_cast<uint64_t>(obj["messageId"].toInteger(0));
    uint64_t receiverId = static_cast<uint64_t>(obj["receiverId"].toInteger(0));

    if (messageId == 0 || receiverId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid messageId or receiverId";
        sendResponse(session, MessageType::MessageRecallNotify, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t senderId = session->userId();

    // Forward recall notify to the receiver if online
    ClientSession* receiverSession = m_server->getSession(receiverId);
    if (receiverSession) {
        QJsonObject notifyObj;
        notifyObj["messageId"] = static_cast<qint64>(messageId);
        notifyObj["senderId"] = static_cast<qint64>(senderId);

        receiverSession->sendPacket(MessageType::MessageRecallNotify, 0,
                                    ChatProtocol::nextSequence(),
                                    QJsonDocument(notifyObj).toJson(QJsonDocument::Compact));
    }

    // Send success response to sender
    QJsonObject response;
    response["success"] = true;
    response["message"] = "Message recalled";
    sendResponse(session, MessageType::MessageRecallNotify, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleAvatarUpload(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::AvatarUploadResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    QString avatarDataBase64 = obj["avatarData"].toString();

    if (avatarDataBase64.isEmpty()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Empty avatar data";
        sendResponse(session, MessageType::AvatarUploadResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Decode base64 to binary data
    QByteArray avatarData = QByteArray::fromBase64(avatarDataBase64.toUtf8());

    // Check size: base64 decoded data should not exceed 64KB
    if (avatarData.size() > 64 * 1024) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Avatar image too large (max 64KB)";
        sendResponse(session, MessageType::AvatarUploadResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Save to avatars/{userId}.png using absolute path
    uint64_t userId = session->userId();
    QString dir = QCoreApplication::applicationDirPath() + "/avatars";
    QDir().mkpath(dir);

    QString avatarPath = dir + "/" + QString::number(static_cast<qulonglong>(userId)) + ".png";
    QFile avatarFile(avatarPath);
    if (!avatarFile.open(QIODevice::WriteOnly)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Failed to save avatar file";
        sendResponse(session, MessageType::AvatarUploadResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }
    avatarFile.write(avatarData);
    avatarFile.close();

    // Send success response
    QJsonObject response;
    response["success"] = true;
    response["avatarUrl"] = "avatars/" + QString::number(static_cast<qulonglong>(userId)) + ".png";
    sendResponse(session, MessageType::AvatarUploadResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));

    qDebug() << "Avatar uploaded for user" << userId << "to" << avatarPath;
}

void MessageRouter::handleGroupCreate(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::GroupCreateResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    QString name = obj["name"].toString();
    QJsonArray memberIdsArray = obj["memberIds"].toArray();

    if (name.isEmpty() || name.length() > 50) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Group name must be 1-50 characters";
        sendResponse(session, MessageType::GroupCreateResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t userId = session->userId();
    uint64_t groupId = m_db->createGroup(name, userId);
    if (groupId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Failed to create group";
        sendResponse(session, MessageType::GroupCreateResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Add creator as owner
    m_db->addGroupMember(groupId, userId, 1);

    // Add invited members
    QVector<quint64> memberIds;
    for (const auto& val : memberIdsArray) {
        uint64_t mid = static_cast<uint64_t>(val.toInteger(0));
        if (mid > 0 && mid != userId) {
            memberIds.append(mid);
        }
    }
    if (!memberIds.isEmpty()) {
        m_db->addGroupMembers(groupId, memberIds);
    }

    // Generate group encryption key for E2E group chat
    {
        QByteArray groupKey = chatroom::crypto::CryptoUtils::generateAesKey();
        QString groupKeyB64 = QString::fromUtf8(groupKey.toBase64());
        // Store the same key for all members (client-side handles encryption/decryption)
        m_db->saveGroupEncryptedKey(groupId, userId, groupKeyB64);
        for (uint64_t mid : memberIds) {
            m_db->saveGroupEncryptedKey(groupId, mid, groupKeyB64);
        }
        qDebug() << "Group" << groupId << "encryption key generated for"
                 << (1 + memberIds.size()) << "members";
    }

    // Notify online members
    const auto& onlineIds = m_server->onlineUserIds();
    for (uint64_t mid : memberIds) {
        if (onlineIds.contains(mid)) {
            ClientSession* memberSession = m_server->getSession(mid);
            if (memberSession) {
                QJsonObject notifyObj;
                notifyObj["groupId"] = static_cast<qint64>(groupId);
                notifyObj["groupName"] = name;
                notifyObj["addedBy"] = static_cast<qint64>(userId);
                memberSession->sendPacket(MessageType::GroupMemberAddNotify, 0,
                                          ChatProtocol::nextSequence(),
                                          QJsonDocument(notifyObj).toJson(QJsonDocument::Compact));
            }
        }
    }

    QJsonObject response;
    response["success"] = true;
    response["groupId"] = static_cast<qint64>(groupId);
    response["name"] = name;
    sendResponse(session, MessageType::GroupCreateResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));

    qDebug() << "Group created:" << name << "(groupId:" << groupId << ") by user" << userId;
}

void MessageRouter::handleGroupList(ClientSession* session, uint32_t seq) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    uint64_t userId = session->userId();
    QVector<ServerDatabase::GroupInfo> groups = m_db->getUserGroups(userId);

    QJsonObject response;
    QJsonArray groupsArray;

    for (const auto& g : groups) {
        QJsonObject groupObj;
        groupObj["groupId"] = static_cast<qint64>(g.groupId);
        groupObj["name"] = g.name;
        groupObj["avatar"] = avatarUrl(g.avatar);
        groupObj["ownerId"] = static_cast<qint64>(g.ownerId);
        groupObj["memberCount"] = g.memberCount;
        groupObj["isDefault"] = g.isDefault;
        groupObj["maxMembers"] = g.maxMembers;
        groupObj["createdAt"] = g.createdAt;
        groupsArray.append(groupObj);
    }

    response["groups"] = groupsArray;
    sendResponse(session, MessageType::GroupListResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleGroupInfo(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::GroupInfoResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));

    if (groupId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid group ID";
        sendResponse(session, MessageType::GroupInfoResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    ServerDatabase::GroupInfo groupInfo = m_db->getGroupInfo(groupId);
    if (groupInfo.groupId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Group not found";
        sendResponse(session, MessageType::GroupInfoResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QVector<ServerDatabase::GroupMemberInfo> members = m_db->getGroupMembers(groupId);

    QJsonObject response;
    response["groupId"] = static_cast<qint64>(groupInfo.groupId);
    response["name"] = groupInfo.name;
    response["avatar"] = avatarUrl(groupInfo.avatar);
    response["ownerId"] = static_cast<qint64>(groupInfo.ownerId);
    response["memberCount"] = groupInfo.memberCount;
    response["isDefault"] = groupInfo.isDefault;
    response["maxMembers"] = groupInfo.maxMembers;
    response["createdAt"] = groupInfo.createdAt;

    QJsonArray membersArray;
    for (const auto& m : members) {
        QJsonObject memberObj;
        memberObj["userId"] = static_cast<qint64>(m.userId);
        memberObj["username"] = m.username;
        memberObj["nickname"] = m.nickname;
        memberObj["avatar"] = avatarUrl(m.avatar);
        memberObj["role"] = m.role;
        membersArray.append(memberObj);
    }
    response["members"] = membersArray;

    sendResponse(session, MessageType::GroupInfoResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleGroupMessage(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Invalid group message JSON";
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));
    QString content = obj["content"].toString();
    qint64 timestamp = obj["timestamp"].toInteger(0);
    int msgType = obj["type"].toInt(0);

    if (groupId == 0 || content.isEmpty()) {
        qWarning() << "Invalid group message: missing groupId or content";
        return;
    }

    uint64_t userId = session->userId();

    // Verify sender is a group member
    if (!m_db->isGroupMember(groupId, userId)) {
        qWarning() << "User" << userId << "is not a member of group" << groupId;
        return;
    }

    // Store the message
    uint64_t msgId = m_db->storeGroupMessage(groupId, userId, content, msgType, timestamp);
    if (msgId == 0) {
        qWarning() << "Failed to store group message in group" << groupId;
        return;
    }

    // Get sender nickname
    UserInfo senderInfo = m_db->getUserInfo(userId);

    // Build notify object
    QJsonObject notifyObj;
    notifyObj["groupId"] = static_cast<qint64>(groupId);
    notifyObj["messageId"] = static_cast<qint64>(msgId);
    notifyObj["senderId"] = static_cast<qint64>(userId);
    notifyObj["senderNickname"] = senderInfo.nickname;
    notifyObj["content"] = content;
    notifyObj["timestamp"] = timestamp;
    notifyObj["type"] = msgType;

    QByteArray notifyData = QJsonDocument(notifyObj).toJson(QJsonDocument::Compact);

    // Get all member IDs and online set
    QVector<quint64> memberIds = m_db->getGroupMemberIds(groupId);
    const auto& onlineIds = m_server->onlineUserIds();

    for (uint64_t mid : memberIds) {
        if (mid == userId) continue; // Skip sender
        if (onlineIds.contains(mid)) {
            ClientSession* memberSession = m_server->getSession(mid);
            if (memberSession) {
                memberSession->sendPacket(MessageType::GroupMessageNotify, 0,
                                          ChatProtocol::nextSequence(), notifyData);
            }
        }
    }

    // Send acknowledgment to sender
    QJsonObject ackObj;
    ackObj["success"] = true;
    ackObj["messageId"] = static_cast<qint64>(msgId);
    sendResponse(session, MessageType::GroupMessageAck, seq,
                 QJsonDocument(ackObj).toJson(QJsonDocument::Compact));

    // Parse @mentions in message content
    {
        // Match patterns like @nickname followed by whitespace, punctuation, or end-of-string
        // This handles Chinese-style @ments where there may be no space after
        QSet<QString> mentionedNames;
        int atPos = 0;
        while ((atPos = content.indexOf('@', atPos)) != -1) {
            int nameStart = atPos + 1;
            if (nameStart >= content.length()) break;
            // Collect consecutive characters until whitespace, punctuation, or end
            int nameEnd = nameStart;
            while (nameEnd < content.length()) {
                QChar ch = content[nameEnd];
                if (ch.isSpace() || ch.isPunct()) break;
                ++nameEnd;
            }
            if (nameEnd > nameStart) {
                QString name = content.mid(nameStart, nameEnd - nameStart);
                mentionedNames.insert(name);
            }
            atPos = nameEnd;
        }

        if (!mentionedNames.isEmpty()) {
            // Get all members of this group to match nicknames
            auto members = m_db->getGroupMembers(groupId);
            QVector<uint64_t> matchedUserIds;
            for (const auto& member : members) {
                if (member.userId == userId) continue; // skip self-mention
                if (mentionedNames.contains(member.nickname) ||
                    mentionedNames.contains(member.username)) {
                    matchedUserIds.append(member.userId);
                }
            }

            if (!matchedUserIds.isEmpty()) {
                QJsonObject atNotifyObj;
                atNotifyObj["groupId"] = static_cast<qint64>(groupId);
                atNotifyObj["senderId"] = static_cast<qint64>(userId);
                atNotifyObj["senderNickname"] = senderInfo.nickname;
                atNotifyObj["messageId"] = static_cast<qint64>(msgId);
                QByteArray atNotifyData = QJsonDocument(atNotifyObj).toJson(QJsonDocument::Compact);

                for (uint64_t mentionedId : matchedUserIds) {
                    if (onlineIds.contains(mentionedId)) {
                        ClientSession* memberSession = m_server->getSession(mentionedId);
                        if (memberSession) {
                            memberSession->sendPacket(MessageType::GroupAtNotify, 0,
                                                      ChatProtocol::nextSequence(), atNotifyData);
                        }
                    }
                }
            }
        }
    }

    // Auto-respond to @bot mentions
    if (m_botService && content.contains("@bot")) {
        QString q = content;
        q = q.replace(QStringLiteral("@bot"), "").trimmed();
        if (!q.isEmpty()) m_botService->handleBotMention(groupId, session->userId(), q);
    }
}

void MessageRouter::handleGroupMemberAdd(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::GroupMemberAddResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));
    QJsonArray memberIdsArray = obj["memberIds"].toArray();

    if (groupId == 0 || memberIdsArray.isEmpty()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid groupId or memberIds";
        sendResponse(session, MessageType::GroupMemberAddResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t userId = session->userId();

    // Verify caller is group owner
    if (!m_db->isGroupOwner(groupId, userId)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Only group owner can add members";
        sendResponse(session, MessageType::GroupMemberAddResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Check member count
    ServerDatabase::GroupInfo groupInfo = m_db->getGroupInfo(groupId);
    int currentCount = m_db->getGroupMemberCount(groupId);
    if (currentCount + memberIdsArray.size() > groupInfo.maxMembers) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Group member limit reached";
        sendResponse(session, MessageType::GroupMemberAddResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Collect member IDs
    QVector<quint64> memberIds;
    for (const auto& val : memberIdsArray) {
        uint64_t mid = static_cast<uint64_t>(val.toInteger(0));
        if (mid > 0) {
            memberIds.append(mid);
        }
    }

    bool ok = m_db->addGroupMembers(groupId, memberIds);

    QJsonObject response;
    if (ok) {
        response["success"] = true;
        response["message"] = "Members added";

        // Notify all online members
        QVector<quint64> allMemberIds = m_db->getGroupMemberIds(groupId);
        const auto& onlineIds = m_server->onlineUserIds();
        for (uint64_t mid : allMemberIds) {
            if (mid == userId) continue; // Skip the owner who initiated
            if (onlineIds.contains(mid)) {
                ClientSession* memberSession = m_server->getSession(mid);
                if (memberSession) {
                    QJsonObject notifyObj;
                    notifyObj["groupId"] = static_cast<qint64>(groupId);
                    notifyObj["addedBy"] = static_cast<qint64>(userId);
                    QJsonArray addedIdsArray;
                    for (uint64_t addedId : memberIds) {
                        addedIdsArray.append(static_cast<qint64>(addedId));
                    }
                    notifyObj["addedMemberIds"] = addedIdsArray;
                    memberSession->sendPacket(MessageType::GroupMemberAddNotify, 0,
                                              ChatProtocol::nextSequence(),
                                              QJsonDocument(notifyObj).toJson(QJsonDocument::Compact));
                }
            }
        }

        qDebug() << "Members added to group" << groupId << "by user" << userId;
    } else {
        response["success"] = false;
        response["message"] = "Failed to add members";
    }

    sendResponse(session, MessageType::GroupMemberAddResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleGroupMemberRemove(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::GroupMemberRemoveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));
    uint64_t memberId = static_cast<uint64_t>(obj["memberId"].toInteger(0));

    if (groupId == 0 || memberId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid groupId or memberId";
        sendResponse(session, MessageType::GroupMemberRemoveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t userId = session->userId();

    // Verify caller is group owner
    if (!m_db->isGroupOwner(groupId, userId)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Only group owner can remove members";
        sendResponse(session, MessageType::GroupMemberRemoveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Verify target is not the owner
    if (m_db->isGroupOwner(groupId, memberId)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Cannot remove group owner";
        sendResponse(session, MessageType::GroupMemberRemoveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    bool ok = m_db->removeGroupMember(groupId, memberId);

    QJsonObject response;
    if (ok) {
        response["success"] = true;
        response["message"] = "Member removed";

        // Build notify object
        QJsonObject notifyObj;
        notifyObj["groupId"] = static_cast<qint64>(groupId);
        notifyObj["removedUserId"] = static_cast<qint64>(memberId);
        notifyObj["removedBy"] = static_cast<qint64>(userId);
        QByteArray notifyData = QJsonDocument(notifyObj).toJson(QJsonDocument::Compact);

        // Notify all remaining online members
        QVector<quint64> allMemberIds = m_db->getGroupMemberIds(groupId);
        const auto& onlineIds = m_server->onlineUserIds();
        for (uint64_t mid : allMemberIds) {
            if (onlineIds.contains(mid)) {
                ClientSession* memberSession = m_server->getSession(mid);
                if (memberSession) {
                    memberSession->sendPacket(MessageType::GroupMemberRemoveNotify, 0,
                                              ChatProtocol::nextSequence(), notifyData);
                }
            }
        }

        // Notify the removed user separately if online
        if (onlineIds.contains(memberId)) {
            ClientSession* removedSession = m_server->getSession(memberId);
            if (removedSession) {
                removedSession->sendPacket(MessageType::GroupMemberRemoveNotify, 0,
                                           ChatProtocol::nextSequence(), notifyData);
            }
        }

        qDebug() << "Member" << memberId << "removed from group" << groupId << "by user" << userId;
    } else {
        response["success"] = false;
        response["message"] = "Failed to remove member";
    }

    sendResponse(session, MessageType::GroupMemberRemoveResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleGroupUpdate(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::GroupUpdateResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));
    QString name = obj["name"].toString();
    int maxMembers = obj["maxMembers"].toInt(-1);

    if (groupId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid group ID";
        sendResponse(session, MessageType::GroupUpdateResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t userId = session->userId();

    // Verify caller is group owner
    if (!m_db->isGroupOwner(groupId, userId)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Only group owner can update group";
        sendResponse(session, MessageType::GroupUpdateResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    bool ok = m_db->updateGroup(groupId, name, "", maxMembers);

    QJsonObject response;
    if (ok) {
        response["success"] = true;
        response["message"] = "Group updated";

        // Broadcast update notify to all online members
        QJsonObject notifyObj;
        notifyObj["groupId"] = static_cast<qint64>(groupId);
        if (!name.isEmpty()) {
            notifyObj["name"] = name;
        }
        if (maxMembers > 0) {
            notifyObj["maxMembers"] = maxMembers;
        }
        QByteArray notifyData = QJsonDocument(notifyObj).toJson(QJsonDocument::Compact);

        QVector<quint64> allMemberIds = m_db->getGroupMemberIds(groupId);
        const auto& onlineIds = m_server->onlineUserIds();
        for (uint64_t mid : allMemberIds) {
            if (onlineIds.contains(mid)) {
                ClientSession* memberSession = m_server->getSession(mid);
                if (memberSession) {
                    memberSession->sendPacket(MessageType::GroupUpdateNotify, 0,
                                              ChatProtocol::nextSequence(), notifyData);
                }
            }
        }

        qDebug() << "Group" << groupId << "updated by user" << userId;
    } else {
        response["success"] = false;
        response["message"] = "Failed to update group";
    }

    sendResponse(session, MessageType::GroupUpdateResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleGroupLeave(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::GroupLeaveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));

    if (groupId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid group ID";
        sendResponse(session, MessageType::GroupLeaveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t userId = session->userId();

    // Verify caller is a group member
    if (!m_db->isGroupMember(groupId, userId)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Not a member of this group";
        sendResponse(session, MessageType::GroupLeaveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Verify caller is NOT the owner
    if (m_db->isGroupOwner(groupId, userId)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Owner cannot leave group, use dissolve instead";
        sendResponse(session, MessageType::GroupLeaveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    bool ok = m_db->removeGroupMember(groupId, userId);

    QJsonObject response;
    if (ok) {
        response["success"] = true;
        response["message"] = "Left group";

        // Broadcast leave notify to all remaining online members
        QJsonObject notifyObj;
        notifyObj["groupId"] = static_cast<qint64>(groupId);
        notifyObj["userId"] = static_cast<qint64>(userId);
        QByteArray notifyData = QJsonDocument(notifyObj).toJson(QJsonDocument::Compact);

        QVector<quint64> allMemberIds = m_db->getGroupMemberIds(groupId);
        const auto& onlineIds = m_server->onlineUserIds();
        for (uint64_t mid : allMemberIds) {
            if (onlineIds.contains(mid)) {
                ClientSession* memberSession = m_server->getSession(mid);
                if (memberSession) {
                    memberSession->sendPacket(MessageType::GroupLeaveNotify, 0,
                                              ChatProtocol::nextSequence(), notifyData);
                }
            }
        }

        qDebug() << "User" << userId << "left group" << groupId;
    } else {
        response["success"] = false;
        response["message"] = "Failed to leave group";
    }

    sendResponse(session, MessageType::GroupLeaveResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleGroupDissolve(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::GroupDissolveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));

    if (groupId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid group ID";
        sendResponse(session, MessageType::GroupDissolveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t userId = session->userId();

    // Verify caller is group owner
    if (!m_db->isGroupOwner(groupId, userId)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Only group owner can dissolve group";
        sendResponse(session, MessageType::GroupDissolveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Verify group is not default
    ServerDatabase::GroupInfo groupInfo = m_db->getGroupInfo(groupId);
    if (groupInfo.isDefault) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Cannot dissolve default group";
        sendResponse(session, MessageType::GroupDissolveResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Get all member IDs before dissolving (for notification)
    QVector<quint64> allMemberIds = m_db->getGroupMemberIds(groupId);

    bool ok = m_db->dissolveGroup(groupId);

    QJsonObject response;
    if (ok) {
        response["success"] = true;
        response["message"] = "Group dissolved";

        // Broadcast dissolve notify to all online members
        QJsonObject notifyObj;
        notifyObj["groupId"] = static_cast<qint64>(groupId);
        notifyObj["dissolvedBy"] = static_cast<qint64>(userId);
        QByteArray notifyData = QJsonDocument(notifyObj).toJson(QJsonDocument::Compact);

        const auto& onlineIds = m_server->onlineUserIds();
        for (uint64_t mid : allMemberIds) {
            if (onlineIds.contains(mid)) {
                ClientSession* memberSession = m_server->getSession(mid);
                if (memberSession) {
                    memberSession->sendPacket(MessageType::GroupDissolveNotify, 0,
                                              ChatProtocol::nextSequence(), notifyData);
                }
            }
        }

        qDebug() << "Group" << groupId << "dissolved by user" << userId;
    } else {
        response["success"] = false;
        response["message"] = "Failed to dissolve group";
    }

    sendResponse(session, MessageType::GroupDissolveResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::handleGroupAvatarUpload(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::GroupAvatarUploadResp, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));
    QString avatarDataBase64 = obj["avatarData"].toString();

    if (groupId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid group ID";
        sendResponse(session, MessageType::GroupAvatarUploadResp, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t userId = session->userId();

    // Verify caller is group owner
    if (!m_db->isGroupOwner(groupId, userId)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Only group owner can upload group avatar";
        sendResponse(session, MessageType::GroupAvatarUploadResp, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    if (avatarDataBase64.isEmpty()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Empty avatar data";
        sendResponse(session, MessageType::GroupAvatarUploadResp, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Decode base64 to binary data
    QByteArray avatarData = QByteArray::fromBase64(avatarDataBase64.toUtf8());

    // Check size: max 2MB
    if (avatarData.size() > 2 * 1024 * 1024) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Avatar image too large (max 2MB)";
        sendResponse(session, MessageType::GroupAvatarUploadResp, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Save to avatars/groups/group_{groupId}.png
    QString dir = QCoreApplication::applicationDirPath() + "/avatars/groups";
    QDir().mkpath(dir);

    QString avatarPath = dir + "/group_" + QString::number(static_cast<qulonglong>(groupId)) + ".png";
    QFile avatarFile(avatarPath);
    if (!avatarFile.open(QIODevice::WriteOnly)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Failed to save avatar file";
        sendResponse(session, MessageType::GroupAvatarUploadResp, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }
    avatarFile.write(avatarData);
    avatarFile.close();

    // Update DB avatar path
    QString relativePath = "avatars/groups/group_" + QString::number(static_cast<qulonglong>(groupId)) + ".png";
    m_db->updateGroup(groupId, "", relativePath);

    QJsonObject response;
    response["success"] = true;
    response["avatarUrl"] = avatarUrl(relativePath);
    sendResponse(session, MessageType::GroupAvatarUploadResp, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));

    qDebug() << "Group avatar uploaded for group" << groupId << "to" << avatarPath;
}

void MessageRouter::handleGroupTransferOwner(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendResponse(session, MessageType::GroupTransferOwnerResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));
    uint64_t newOwnerId = static_cast<uint64_t>(obj["newOwnerId"].toInteger(0));

    if (groupId == 0 || newOwnerId == 0) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Invalid groupId or newOwnerId";
        sendResponse(session, MessageType::GroupTransferOwnerResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t userId = session->userId();

    // Verify caller is group owner
    if (!m_db->isGroupOwner(groupId, userId)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "Only group owner can transfer ownership";
        sendResponse(session, MessageType::GroupTransferOwnerResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    // Verify new owner is a group member
    if (!m_db->isGroupMember(groupId, newOwnerId)) {
        QJsonObject response;
        response["success"] = false;
        response["message"] = "New owner must be a group member";
        sendResponse(session, MessageType::GroupTransferOwnerResponse, seq,
                     QJsonDocument(response).toJson(QJsonDocument::Compact));
        return;
    }

    bool ok = m_db->transferGroupOwner(groupId, newOwnerId);

    QJsonObject response;
    if (ok) {
        response["success"] = true;
        response["message"] = "Ownership transferred";

        // Broadcast transfer notify to all online members
        QJsonObject notifyObj;
        notifyObj["groupId"] = static_cast<qint64>(groupId);
        notifyObj["previousOwnerId"] = static_cast<qint64>(userId);
        notifyObj["newOwnerId"] = static_cast<qint64>(newOwnerId);
        QByteArray notifyData = QJsonDocument(notifyObj).toJson(QJsonDocument::Compact);

        QVector<quint64> allMemberIds = m_db->getGroupMemberIds(groupId);
        const auto& onlineIds = m_server->onlineUserIds();
        for (uint64_t mid : allMemberIds) {
            if (onlineIds.contains(mid)) {
                ClientSession* memberSession = m_server->getSession(mid);
                if (memberSession) {
                    memberSession->sendPacket(MessageType::GroupTransferOwnerNotify, 0,
                                              ChatProtocol::nextSequence(), notifyData);
                }
            }
        }

        qDebug() << "Group" << groupId << "ownership transferred from" << userId << "to" << newOwnerId;
    } else {
        response["success"] = false;
        response["message"] = "Failed to transfer ownership";
    }

    sendResponse(session, MessageType::GroupTransferOwnerResponse, seq,
                 QJsonDocument(response).toJson(QJsonDocument::Compact));
}

void MessageRouter::sendResponse(ClientSession* session, MessageType respType,
                                 uint32_t seq, const QByteArray& body) {
    session->sendPacket(respType, 0, seq, body);
}

QString MessageRouter::avatarUrl(const QString& relativePath) const {
    if (relativePath.isEmpty()) return "";
    // Return as-is; client will convert relative paths to full URLs
    // using its known server host and admin port from login response
    return relativePath;
}

void MessageRouter::sendError(ClientSession* session, uint32_t seq, const QString& message) {
    QJsonObject errObj;
    errObj["success"] = false;
    errObj["message"] = message;
    session->sendPacket(MessageType::ChatMessageAck,
                        static_cast<uint8_t>(MessageFlag::IsResponse),
                        seq, QJsonDocument(errObj).toJson(QJsonDocument::Compact));
}

// ===================================================================
// File upload handler
// ===================================================================
void MessageRouter::handleFileUpload(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        QJsonObject err; err["success"] = false; err["message"] = "Invalid JSON";
        sendResponse(session, MessageType::FileUploadResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    QString fileName = obj["fileName"].toString();
    qint64 fileSize = obj["fileSize"].toInteger(0);
    QString mimeType = obj["mimeType"].toString();
    QString fileDataB64 = obj["fileData"].toString();
    uint64_t targetUserId = static_cast<uint64_t>(obj["targetUserId"].toInteger(0));
    uint64_t targetGroupId = static_cast<uint64_t>(obj["targetGroupId"].toInteger(0));

    if (fileName.isEmpty() || fileDataB64.isEmpty()) {
        QJsonObject err; err["success"] = false; err["message"] = "Missing fileName or fileData";
        sendResponse(session, MessageType::FileUploadResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }

    // Limit file size to 50MB
    const qint64 MAX_FILE_SIZE = 50 * 1024 * 1024;
    if (fileSize > MAX_FILE_SIZE) {
        QJsonObject err; err["success"] = false; err["message"] = "File too large (max 50MB)";
        sendResponse(session, MessageType::FileUploadResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }

    QByteArray fileData = QByteArray::fromBase64(fileDataB64.toUtf8());
    if (fileData.isEmpty()) {
        QJsonObject err; err["success"] = false; err["message"] = "Invalid file data";
        sendResponse(session, MessageType::FileUploadResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }

    // Generate unique filename
    QString ext = fileName.contains('.') ? fileName.mid(fileName.lastIndexOf('.')) : "";
    QString storageName = QString::number(QDateTime::currentSecsSinceEpoch()) + "_" +
                          QString::number(session->userId()) + ext;
    QString subDir = (mimeType.startsWith("image/") ? "images" : "files");
    QString fullPath = "uploads/" + subDir + "/" + storageName;

    // Save file
    QFile outFile(fullPath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        QJsonObject err; err["success"] = false; err["message"] = "Failed to save file";
        sendResponse(session, MessageType::FileUploadResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }
    outFile.write(fileData);
    outFile.close();

    // Calculate SHA-256
    QString sha256 = QString::fromUtf8(QCryptographicHash::hash(fileData, QCryptographicHash::Sha256).toHex());

    // Save metadata
    uint64_t fileId = m_db->saveFileMeta(session->userId(), targetUserId, targetGroupId,
                                          fileName, fileSize, fullPath, mimeType, sha256);

    QJsonObject response;
    response["success"] = true;
    response["fileId"] = static_cast<qint64>(fileId);
    response["fileName"] = fileName;
    response["fileSize"] = fileSize;
    response["sha256"] = sha256;

    sendResponse(session, MessageType::FileUploadResponse, seq, QJsonDocument(response).toJson(QJsonDocument::Compact));

    // Forward notification to recipient
    if (targetUserId > 0) {
        QJsonObject notify;
        notify["fileId"] = static_cast<qint64>(fileId);
        notify["senderId"] = static_cast<qint64>(session->userId());
        notify["fileName"] = fileName;
        notify["fileSize"] = fileSize;
        notify["mimeType"] = mimeType;
        ClientSession* targetSession = m_server->getSession(targetUserId);
        if (targetSession) {
            targetSession->sendPacket(MessageType::FileUploadResponse, 0,
                                      ChatProtocol::nextSequence(),
                                      QJsonDocument(notify).toJson(QJsonDocument::Compact));
        }
    }
}

// ===================================================================
// File download handler
// ===================================================================
void MessageRouter::handleFileDownload(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        QJsonObject err; err["success"] = false; err["message"] = "Invalid JSON";
        sendResponse(session, MessageType::FileDownloadResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t fileId = static_cast<uint64_t>(doc.object()["fileId"].toInteger(0));
    auto meta = m_db->getFileMeta(fileId);
    if (meta.fileId == 0) {
        QJsonObject err; err["success"] = false; err["message"] = "File not found";
        sendResponse(session, MessageType::FileDownloadResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }

    // Permission check: requester must be sender, receiver, or a member of the target group
    uint64_t myId = session->userId();
    bool authorized = (meta.senderId == myId) || (meta.receiverId == myId);
    if (!authorized && meta.groupId > 0) {
        // Group file: check group membership
        authorized = m_db->isGroupMember(meta.groupId, myId);
    }
    if (!authorized) {
        QJsonObject err; err["success"] = false; err["message"] = "Access denied";
        sendResponse(session, MessageType::FileDownloadResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }

    // Read file
    QFile inFile(meta.filePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        QJsonObject err; err["success"] = false; err["message"] = "Failed to read file";
        sendResponse(session, MessageType::FileDownloadResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }
    QByteArray fileData = inFile.readAll();
    inFile.close();

    QJsonObject response;
    response["success"] = true;
    response["fileId"] = static_cast<qint64>(meta.fileId);
    response["fileName"] = meta.fileName;
    response["fileSize"] = meta.fileSize;
    response["mimeType"] = meta.mimeType;
    response["fileData"] = QString::fromUtf8(fileData.toBase64());
    response["sha256"] = meta.sha256Hash;

    sendResponse(session, MessageType::FileDownloadResponse, seq, QJsonDocument(response).toJson(QJsonDocument::Compact));
}

// ===================================================================
// File chunk handler (for large files)
// ===================================================================
void MessageRouter::handleFileChunk(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    uint64_t fileId = static_cast<uint64_t>(obj["fileId"].toInteger(0));
    uint32_t chunkIndex = static_cast<uint32_t>(obj["chunkIndex"].toInteger(0));
    uint32_t totalChunks = static_cast<uint32_t>(obj["totalChunks"].toInteger(0));
    QString dataB64 = obj["data"].toString();

    // Store chunk in a temp file
    QString chunkPath = QString("uploads/files/%1_%2.chunk").arg(fileId).arg(chunkIndex);
    QFile chunkFile(chunkPath);
    if (chunkFile.open(QIODevice::WriteOnly)) {
        chunkFile.write(QByteArray::fromBase64(dataB64.toUtf8()));
        chunkFile.close();
    }

    QJsonObject ack;
    ack["fileId"] = static_cast<qint64>(fileId);
    ack["chunkIndex"] = static_cast<qint64>(chunkIndex);
    sendResponse(session, MessageType::FileChunkAck, seq, QJsonDocument(ack).toJson(QJsonDocument::Compact));
}

// ===================================================================
// Message search handler
// ===================================================================
void MessageRouter::handleMessageSearch(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        QJsonObject err; err["success"] = false; err["message"] = "Invalid JSON";
        sendResponse(session, MessageType::MessageSearchResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }

    QJsonObject obj = doc.object();
    QString keyword = obj["keyword"].toString();
    int limit = obj["limit"].toInt(50);
    int offset = obj["offset"].toInt(0);

    if (keyword.isEmpty()) {
        QJsonObject err; err["success"] = false; err["message"] = "Keyword required";
        sendResponse(session, MessageType::MessageSearchResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }

    auto results = m_db->searchMessages(session->userId(), keyword, limit, offset);

    QJsonArray arr;
    for (const auto& r : results) {
        QJsonObject item;
        item["messageId"] = static_cast<qint64>(r.messageId);
        item["senderId"] = static_cast<qint64>(r.senderId);
        item["senderName"] = r.senderName;
        item["content"] = r.content;
        item["snippet"] = r.snippet;
        item["timestamp"] = r.timestamp;
        item["isGroup"] = r.isGroup;
        item["groupId"] = static_cast<qint64>(r.groupId);
        item["groupName"] = r.groupName;
        arr.append(item);
    }

    QJsonObject response;
    response["success"] = true;
    response["results"] = arr;
    response["total"] = static_cast<int>(results.size());

    sendResponse(session, MessageType::MessageSearchResponse, seq, QJsonDocument(response).toJson(QJsonDocument::Compact));
}

// ===================================================================
// Message reply handler
// ===================================================================
void MessageRouter::handleMessageReply(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    uint64_t replyToId = static_cast<uint64_t>(obj["replyTo"].toInteger(0));
    uint64_t targetUserId = static_cast<uint64_t>(obj["targetUserId"].toInteger(0));
    uint64_t targetGroupId = static_cast<uint64_t>(obj["targetGroupId"].toInteger(0));
    QString replyContent = obj["replyContent"].toString();
    QString content = obj["content"].toString();

    qint64 now = QDateTime::currentSecsSinceEpoch();

    QJsonObject notify;
    notify["replyTo"] = static_cast<qint64>(replyToId);
    notify["replyContent"] = replyContent;
    notify["senderId"] = static_cast<qint64>(session->userId());
    notify["senderName"] = obj["senderName"];
    notify["content"] = content;
    notify["timestamp"] = now;

    QByteArray notifyData = QJsonDocument(notify).toJson(QJsonDocument::Compact);

    if (targetGroupId > 0) {
        // Group reply: store then broadcast via GroupMessageNotify
        uint64_t msgId = m_db->storeGroupMessage(targetGroupId, session->userId(), content, 0, now);

        // Build GroupMessageNotify payload (consistent with handleGroupMessage)
        UserInfo senderInfo = m_db->getUserInfo(session->userId());
        QJsonObject groupNotify;
        groupNotify["groupId"] = static_cast<qint64>(targetGroupId);
        groupNotify["messageId"] = static_cast<qint64>(msgId);
        groupNotify["senderId"] = static_cast<qint64>(session->userId());
        groupNotify["senderNickname"] = senderInfo.nickname;
        groupNotify["content"] = content;
        groupNotify["timestamp"] = now;
        groupNotify["type"] = 0;
        // Attach reply metadata so client can render as a reply
        groupNotify["replyTo"] = static_cast<qint64>(replyToId);
        groupNotify["replyContent"] = replyContent;

        QByteArray groupNotifyData = QJsonDocument(groupNotify).toJson(QJsonDocument::Compact);

        QVector<quint64> memberIds = m_db->getGroupMemberIds(targetGroupId);
        const auto& onlineIds = m_server->onlineUserIds();
        for (uint64_t mid : memberIds) {
            if (mid == session->userId()) continue;
            if (onlineIds.contains(mid)) {
                ClientSession* memberSession = m_server->getSession(mid);
                if (memberSession) {
                    memberSession->sendPacket(MessageType::GroupMessageNotify, 0,
                                              ChatProtocol::nextSequence(), groupNotifyData);
                }
            }
        }

        // Ack to sender (use GroupMessageAck so client can match sequence)
        QJsonObject ackObj;
        ackObj["success"] = true;
        ackObj["messageId"] = static_cast<qint64>(msgId);
        sendResponse(session, MessageType::GroupMessageAck, seq,
                     QJsonDocument(ackObj).toJson(QJsonDocument::Compact));
        return;
    } else if (targetUserId > 0) {
        // Private reply: persist for search, then forward or store offline
        m_db->storeChatMessage(session->userId(), targetUserId, content, 0, now);

        ClientSession* targetSession = m_server->getSession(targetUserId);
        if (targetSession) {
            targetSession->sendPacket(MessageType::MessageReplyNotify, 0,
                                      ChatProtocol::nextSequence(), notifyData);
        } else {
            // Store as offline message so recipient gets it on next login
            ChatMessage msg;
            msg.senderId = session->userId();
            msg.receiverId = targetUserId;
            msg.content = content;
            msg.timestamp = QDateTime::fromSecsSinceEpoch(now);
            m_db->storeOfflineMessage(msg);
        }
    }

    // Ack to sender
    QJsonObject ackObj;
    ackObj["success"] = true;
    sendResponse(session, MessageType::MessageReplyNotify, seq,
                 QJsonDocument(ackObj).toJson(QJsonDocument::Compact));
}

// ===================================================================
// Group announcement handler
// ===================================================================
void MessageRouter::handleGroupAnnouncement(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));
    QString content = obj["content"].toString();

    if (content.isEmpty()) {
        sendError(session, seq, "Announcement content required");
        return;
    }

    // Only owner/admin can set announcements
    if (!m_db->isGroupOwner(groupId, session->userId())) {
        // Check if user is admin (role >= 2)
        // For now, only owner
        sendError(session, seq, "Only group owner can set announcements");
        return;
    }

    m_db->setAnnouncement(groupId, session->userId(), content);

    QJsonObject response;
    response["success"] = true;
    response["groupId"] = static_cast<qint64>(groupId);
    response["senderName"] = obj["senderName"];
    response["content"] = content;
    response["timestamp"] = static_cast<qint64>(QDateTime::currentSecsSinceEpoch());

    // Broadcast to all group members
    auto memberIds = m_db->getGroupMemberIds(groupId);
    QByteArray pushBody = QJsonDocument(response).toJson(QJsonDocument::Compact);
    for (uint64_t uid : memberIds) {
        ClientSession* memberSession = m_server->getSession(uid);
        if (memberSession && uid != session->userId()) {
            memberSession->sendPacket(MessageType::GroupAnnouncementPush, 0,
                                       ChatProtocol::nextSequence(), pushBody);
        }
    }

    sendResponse(session, MessageType::GroupAnnouncementPush, seq, pushBody);
}

// ===================================================================
// Group message read handler
// ===================================================================
void MessageRouter::handleGroupMessageRead(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) return;

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    uint64_t msgId = static_cast<uint64_t>(obj["msgId"].toInteger(0));
    uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));

    m_db->markGroupMessageRead(msgId, groupId, session->userId());
}

// ===================================================================
// Group key request handler
// ===================================================================
void MessageRouter::handleGroupKeyRequest(ClientSession* session, uint32_t seq, const QByteArray& body) {
    if (!session->isAuthenticated()) {
        sendError(session, seq, "Not authenticated");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        QJsonObject err; err["success"] = false; err["message"] = "Invalid JSON";
        sendResponse(session, MessageType::GroupKeyResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }

    uint64_t groupId = static_cast<uint64_t>(doc.object()["groupId"].toInteger(0));
    uint64_t userId = session->userId();

    // Verify user is a group member
    if (!m_db->isGroupMember(groupId, userId)) {
        QJsonObject err; err["success"] = false; err["message"] = "Not a group member";
        sendResponse(session, MessageType::GroupKeyResponse, seq, QJsonDocument(err).toJson(QJsonDocument::Compact));
        return;
    }

    QString encryptedKey = m_db->getGroupEncryptedKey(groupId, userId);
    if (encryptedKey.isEmpty()) {
        // No key found, member might have just joined - try owner's key
        auto groupInfo = m_db->getGroupInfo(groupId);
        if (groupInfo.ownerId > 0) {
            encryptedKey = m_db->getGroupEncryptedKey(groupId, groupInfo.ownerId);
        }
    }

    QJsonObject response;
    response["success"] = !encryptedKey.isEmpty();
    response["groupId"] = static_cast<qint64>(groupId);
    response["encryptedKey"] = encryptedKey;

    sendResponse(session, MessageType::GroupKeyResponse, seq, QJsonDocument(response).toJson(QJsonDocument::Compact));
}

} // namespace chatroom::server

