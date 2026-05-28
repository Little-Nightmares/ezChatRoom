#include "AppCore.h"
#include "core/AppManager.h"
#include "core/SessionManager.h"
#include "database/DatabaseManager.h"
#include "network/TcpClient.h"
#include "network/MessageHandler.h"
#include "network/HeartbeatManager.h"
#include "ChatController.h"
#include "UserController.h"
#include "FriendController.h"
#include "GroupController.h"
#include "models/MessageModel.h"
#include "models/ConversationModel.h"
#include "models/FriendRequestModel.h"
#include "models/UserModel.h"
#include "models/GroupModel.h"
#include "protocol/ChatProtocol.h"
#include "protocol/MessageTypes.h"
#include "crypto/CryptoUtils.h"
#include "models/ChatMessage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>
#include <QUrl>
#include <QCoreApplication>
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef CHATROOM_HAS_MULTIMEDIA
#include <QSoundEffect>
#endif

namespace chatroom::client {

AppCore::AppCore(QObject* parent)
    : QObject(parent)
    , m_appManager(std::make_unique<AppManager>(this))
    , m_chatController(new ChatController(m_appManager.get(), this))
    , m_userController(new UserController(m_appManager.get(), this))
    , m_friendController(new FriendController(m_appManager.get(), this))
    , m_groupModel(new GroupModel(this))
    , m_groupController(new GroupController(m_appManager.get(), m_groupModel, this))
    , m_themeManager(new ThemeManager(this))
    , m_conversationModel(new ConversationModel(this))
{
    m_chatBackground = m_appManager->databaseManager()->loadSetting("chat_background", "");
#ifdef CHATROOM_HAS_MULTIMEDIA
    m_messageSound = new QSoundEffect(this);
    m_messageSound->setSource(QUrl("qrc:/sounds/message.wav"));
    m_messageSound->setVolume(0.5);
#else
    qWarning() << "Qt6::Multimedia not available, sound notifications disabled";
#endif

    // Forward TcpClient signals
    connect(m_appManager->tcpClient(), &TcpClient::connected,
            this, &AppCore::connected);
    connect(m_appManager->tcpClient(), &TcpClient::disconnected,
            this, &AppCore::disconnected);
    connect(m_appManager->tcpClient(), &TcpClient::connectionError,
            this, &AppCore::connectionError);

    // Forward SessionManager login state
    connect(m_appManager->sessionManager(), &SessionManager::loginStateChanged,
            this, &AppCore::loginStateChanged);

    // Connect key exchange signal
    connect(m_chatController, &ChatController::needKeyExchange,
            this, &AppCore::onNeedKeyExchange);

    // Clear unread count when switching to a conversation
    connect(m_chatController, &ChatController::unreadCleared,
            this, [this](quint64 friendId) {
        m_conversationModel->clearUnreadCount(friendId);
    });

    // On reconnect success, re-send login request
    connect(m_appManager.get(), &AppManager::reconnectSuccess,
            this, [this]() {
        auto* session = m_appManager->sessionManager();
        if (!session->savedUsername().isEmpty() && !session->savedPasswordHash().isEmpty()) {
            m_userController->login(session->savedUsername(), session->savedPasswordHash());
        }
    });

    // On login success, start heartbeat and request friend lists
    connect(m_userController, &UserController::loginSuccess,
            this, [this]() {
        // Start heartbeat
        m_appManager->heartbeatManager()->start();
        qDebug() << "AppCore: login success, starting heartbeat and loading data";

        // Request friend list and friend request list
        m_friendController->requestFriendList();
        m_friendController->requestFriendRequestList();

        // Request group list
        m_groupController->requestGroupList();

        // Load conversations from local database
        auto convInfos = m_appManager->databaseManager()->getConversationList();
        QVector<ConversationModel::ConversationData> convData;
        convData.reserve(convInfos.size());
        for (const auto& info : convInfos) {
            ConversationModel::ConversationData data;
            data.friendId = info.friendId;
            data.nickname = info.nickname;
            data.avatar = info.avatar;
            data.lastMessage = info.lastMessage;
            data.lastTime = info.lastTime;
            data.unreadCount = info.unreadCount;
            convData.append(data);
        }
        m_conversationModel->refresh(convData);
    });

    // On chat message received, update conversation model
    connect(m_chatController, &ChatController::messageSent,
            this, [this]() {
        quint64 friendId = m_chatController->currentChatFriendId();
        // Get last message from model
        // The conversation model will be updated when we get the ack
    });
}

AppCore::~AppCore()
{
}

ChatController* AppCore::chatController() const
{
    return m_chatController;
}

UserController* AppCore::userController() const
{
    return m_userController;
}

FriendController* AppCore::friendController() const
{
    return m_friendController;
}

GroupController* AppCore::groupController() const
{
    return m_groupController;
}

GroupModel* AppCore::groupModel() const
{
    return m_groupModel;
}

MessageModel* AppCore::messageModel() const
{
    return m_chatController->messageModel();
}

ConversationModel* AppCore::conversationModel() const
{
    return m_conversationModel;
}

FriendRequestModel* AppCore::friendRequestModel() const
{
    return m_friendController->friendRequestModel();
}

UserModel* AppCore::userModel() const
{
    return m_friendController->userModel();
}

SessionManager* AppCore::sessionManager() const
{
    return m_appManager->sessionManager();
}

AppManager* AppCore::appManager() const
{
    return m_appManager.get();
}

ThemeManager* AppCore::themeManager() const
{
    return m_themeManager;
}

bool AppCore::isLoggedIn() const
{
    return m_appManager->sessionManager()->isLoggedIn();
}

void AppCore::initialize()
{
    m_appManager->initialize();
    registerMessageHandlers();
    qDebug() << "AppCore: initialized";
}

void AppCore::connectToServer(const QString& host, int port)
{
    m_appManager->connectToServer(host, static_cast<quint16>(port));
}

void AppCore::logout()
{
    m_userController->logout();
    m_chatController->clearCurrentChat();
    m_conversationModel->clear();
    m_friendController->userModel()->clear();
    m_friendController->friendRequestModel()->clear();
}

void AppCore::showToast(const QString& message, const QString& type)
{
    emit showNotification(message, type);
}


void AppCore::registerMessageHandlers()
{
    auto* handler = m_appManager->messageHandler();

    // Chat message received from another user (relayed by server)
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::ChatMessage),
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleChatMessage(flags, seq, body);
        });

    // Chat message ack from server
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::ChatMessageAck),
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleChatMessageAck(flags, seq, body);
        });

    // Offline message push
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::OfflineMessagePush),
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleOfflineMessagePush(flags, seq, body);
        });

    // Offline message done
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::OfflineMessageDone),
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleOfflineMessageDone(flags, seq, body);
        });

    // Key exchange request
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::KeyExchangeRequest),
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleKeyExchangeRequest(flags, seq, body);
        });

    // Key exchange response
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::KeyExchangeResponse),
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleKeyExchangeResponse(flags, seq, body);
        });

    // Message recall notify
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::MessageRecallNotify),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            Q_UNUSED(f) Q_UNUSED(s)
            QJsonDocument doc = QJsonDocument::fromJson(b);
            qint64 messageId = doc.object()["messageId"].toInteger(0);
            qint64 senderId = doc.object()["senderId"].toInteger(0);

            // Update message model
            m_chatController->messageModel()->updateMessageContent(messageId, "[�Է�������һ����Ϣ]");
            // Update database
            m_appManager->databaseManager()->updateMessageContent(messageId, "[�Է�������һ����Ϣ]");
        });

    // Avatar upload response
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::AvatarUploadResponse),
        [this](uint8_t f, uint32_t s, const QByteArray& b) {
            Q_UNUSED(f) Q_UNUSED(s)
            QJsonDocument doc = QJsonDocument::fromJson(b);
            QJsonObject obj = doc.object();
            bool success = obj["success"].toBool(false);
            if (success) {
                QString avatarUrl = obj["avatarUrl"].toString();
                if (!avatarUrl.isEmpty()) {
                    // Save avatar locally and use local file path
                    QString localAvatarDir = QCoreApplication::applicationDirPath() + "/avatars";
                    QDir().mkpath(localAvatarDir);
                    QString localAvatarPath = localAvatarDir + "/" +
                        QString::number(m_appManager->sessionManager()->userId()) + ".png";
                    QString fileUrl = QUrl::fromLocalFile(localAvatarPath).toString();
                    m_appManager->sessionManager()->setAvatar(fileUrl);
                    qDebug() << "Avatar uploaded, local path:" << fileUrl;
                }
            } else {
                QString message = obj["message"].toString("Upload failed");
                emit showNotification(message, "error");
            }
        });

    // Server kick notification
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::ServerKickNotify),
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            Q_UNUSED(flags)
            Q_UNUSED(seq)
            QJsonDocument doc = QJsonDocument::fromJson(body);
            QString reason = doc.object()["reason"].toString("δ֪ԭ��");
            qWarning() << "Kicked by server:" << reason;
            emit kicked(reason);
        });

    // Group message notify (0x68)
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::GroupMessageNotify),
        [this](uint8_t flags, uint32_t sequence, const QByteArray& body) {
            Q_UNUSED(flags)
            Q_UNUSED(sequence)

            QJsonDocument doc = QJsonDocument::fromJson(body);
            if (!doc.isObject()) {
                qWarning() << "AppCore: invalid group message notify format";
                return;
            }

            QJsonObject obj = doc.object();
            uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));
            uint64_t senderId = static_cast<uint64_t>(obj["senderId"].toInteger(0));
            QString senderNickname = obj["senderNickname"].toString();
            QString content = obj["content"].toString();
            qint64 timestamp = obj["timestamp"].toInteger(QDateTime::currentSecsSinceEpoch());
            QDateTime msgTime = QDateTime::fromSecsSinceEpoch(timestamp);

            // Save to database
            chatroom::models::ChatMessage msg;
            msg.senderId = senderId;
            msg.groupId = groupId;
            msg.senderNickname = senderNickname;
            msg.content = content;
            msg.timestamp = msgTime;
            msg.isMine = false;
            msg.isRead = (groupId == m_chatController->currentGroupId());
            msg.type = chatroom::models::ChatMessageType::Text;

            qint64 insertedId = m_appManager->databaseManager()->saveGroupMessage(msg);
            if (insertedId > 0) {
                msg.messageId = static_cast<uint64_t>(insertedId);
            }

            // Add to model if currently viewing this group chat
            if (groupId == m_chatController->currentGroupId()) {
                m_chatController->messageModel()->addMessage(msg);
            }

            // Update conversation model
            QString groupName = obj["groupName"].toString();
            QString groupAvatar = obj["groupAvatar"].toString();
            m_conversationModel->updateGroupConversation(groupId, groupName, groupAvatar,
                                                          content, msgTime, 1);

            // Play notification sound if not currently viewing this group
            if (groupId != m_chatController->currentGroupId()) {
                playNotificationSound();
            }
        });

    // Group @mention notify (0x97)
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::GroupAtNotify),
        [this](uint8_t flags, uint32_t sequence, const QByteArray& body) {
            Q_UNUSED(flags)
            Q_UNUSED(sequence)

            QJsonDocument doc = QJsonDocument::fromJson(body);
            if (!doc.isObject()) {
                qWarning() << "AppCore: invalid group @notify format";
                return;
            }

            QJsonObject obj = doc.object();
            uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));
            uint64_t senderId = static_cast<uint64_t>(obj["senderId"].toInteger(0));
            QString senderNickname = obj["senderNickname"].toString();
            uint64_t msgId = static_cast<uint64_t>(obj["messageId"].toInteger(0));
            Q_UNUSED(msgId)

            // Get group name
            QString groupName;
            if (m_groupModel) {
                for (int i = 0; i < m_groupModel->rowCount(); ++i) {
                    QModelIndex idx = m_groupModel->index(i, 0);
                    if (idx.data(GroupModel::GroupIdRole).toULongLong() == groupId) {
                        groupName = idx.data(GroupModel::GroupNameRole).toString();
                        break;
                    }
                }
            }
            if (groupName.isEmpty()) {
                groupName = QString::number(static_cast<qulonglong>(groupId));
            }

            // Show toast notification
            QString toastMsg = senderNickname + " 在群 \"" + groupName + "\" 中@了你";
            emit showNotification(toastMsg, "info");

            qDebug() << "AppCore: @mention notify from" << senderId << "in group" << groupId;
        });

    // Group message ack (0x67)
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::GroupMessageAck),
        [this](uint8_t flags, uint32_t sequence, const QByteArray& body) {
            Q_UNUSED(flags)
            Q_UNUSED(body)
            // Try to match ACK to specific message by sequence number
            if (!m_chatController->acknowledgeMessage(sequence)) {
                // Fallback: mark last sending message as delivered
                m_chatController->messageModel()->markLastSendingAsDelivered();
            }
        });

    // Group offline message push (0x71)
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::GroupOfflineMsgPush),
        [this](uint8_t flags, uint32_t sequence, const QByteArray& body) {
            Q_UNUSED(flags)
            Q_UNUSED(sequence)

            QJsonDocument doc = QJsonDocument::fromJson(body);
            if (!doc.isObject()) {
                qWarning() << "AppCore: invalid group offline message format";
                return;
            }

            QJsonObject obj = doc.object();
            uint64_t groupId = static_cast<uint64_t>(obj["groupId"].toInteger(0));
            uint64_t senderId = static_cast<uint64_t>(obj["senderId"].toInteger(0));
            QString senderNickname = obj["senderNickname"].toString();
            QString content = obj["content"].toString();
            qint64 timestamp = obj["timestamp"].toInteger(QDateTime::currentSecsSinceEpoch());
            QDateTime msgTime = QDateTime::fromSecsSinceEpoch(timestamp);

            // Save to database
            chatroom::models::ChatMessage msg;
            msg.senderId = senderId;
            msg.groupId = groupId;
            msg.senderNickname = senderNickname;
            msg.content = content;
            msg.timestamp = msgTime;
            msg.isMine = false;
            msg.isRead = (groupId == m_chatController->currentGroupId());
            msg.type = chatroom::models::ChatMessageType::Text;

            qint64 insertedId = m_appManager->databaseManager()->saveGroupMessage(msg);
            if (insertedId > 0) {
                msg.messageId = static_cast<uint64_t>(insertedId);
            }

            // Add to model if currently viewing this group chat
            if (groupId == m_chatController->currentGroupId()) {
                m_chatController->messageModel()->addMessage(msg);
            }

            // Update conversation model
            QString groupName = obj["groupName"].toString();
            QString groupAvatar = obj["groupAvatar"].toString();
            m_conversationModel->updateGroupConversation(groupId, groupName, groupAvatar,
                                                          content, msgTime, 1);

            qDebug() << "AppCore: group offline message received from" << senderId
                     << "in group" << groupId;
        });

    // Group offline message done (0x72)
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::GroupOfflineMsgDone),
        [this](uint8_t flags, uint32_t sequence, const QByteArray& body) {
            Q_UNUSED(flags)
            Q_UNUSED(sequence)
            Q_UNUSED(body)
            qDebug() << "AppCore: all group offline messages received";
        });

    // File upload response
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FileUploadResponse),
        [this](uint8_t, uint32_t, const QByteArray& body) {
            m_chatController->handleFileUploadResponse(body);
        });

    // File download response
    handler->registerHandler(
        static_cast<uint8_t>(chatroom::protocol::MessageType::FileDownloadResponse),
        [this](uint8_t, uint32_t, const QByteArray& body) {
            m_chatController->handleFileDownloadResponse(body);
        });
}

void AppCore::handleChatMessage(uint8_t flags, uint32_t sequence, const QByteArray& body)
{
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        qWarning() << "AppCore: invalid chat message format";
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t senderId = static_cast<uint64_t>(obj["senderId"].toInteger(0));
    QString content = obj["content"].toString();
    qint64 timestamp = obj["timestamp"].toInteger(QDateTime::currentSecsSinceEpoch());
    QDateTime msgTime = QDateTime::fromSecsSinceEpoch(timestamp);

    bool isEncrypted = flags & static_cast<uint8_t>(chatroom::protocol::MessageFlag::Encrypted);

    // Pass to ChatController (it handles decryption internally)
    m_chatController->onChatMessageReceived(senderId, content, msgTime, isEncrypted);

    // Update conversation model with decrypted content for preview
    QString displayContent = content;
    if (isEncrypted) {
        // Try to decrypt for preview
        auto* session = m_appManager->sessionManager();
        if (session->hasSessionKey(senderId)) {
            QByteArray sessionKey = session->getSessionKey(senderId);
            QByteArray encryptedData = QByteArray::fromBase64(content.toUtf8());
            QByteArray decrypted = chatroom::crypto::CryptoUtils::aes256GcmDecrypt(encryptedData, sessionKey);
            if (!decrypted.isEmpty()) {
                displayContent = QString::fromUtf8(decrypted);
            } else {
                displayContent = "[Encrypted message]";
            }
        } else {
            displayContent = "[Encrypted message]";
        }
    }
    m_conversationModel->updateConversation(senderId, displayContent, msgTime, 1);

    // Cache sender info if available
    if (obj.contains("senderUsername")) {
        chatroom::models::UserInfo user;
        user.userId = senderId;
        user.username = obj["senderUsername"].toString();
        user.nickname = obj["senderNickname"].toString();
        m_appManager->databaseManager()->upsertUser(user);
    }

    // Message notification (only if not currently viewing this chat)
    if (senderId != m_chatController->currentChatFriendId()) {
        m_pendingMessageCounts[senderId]++;
        if (!m_notificationTimers.contains(senderId)) {
            QTimer* timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(1000);
            connect(timer, &QTimer::timeout, this, [this, senderId]() {
                triggerNotification(senderId);
            });
            m_notificationTimers[senderId] = timer;
            timer->start();
        }
    }
}

void AppCore::handleChatMessageAck(uint8_t flags, uint32_t sequence, const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(body)
    // Try to match ACK to specific message by sequence number
    if (!m_chatController->acknowledgeMessage(sequence)) {
        // Fallback: mark last sending message as delivered
        m_chatController->messageModel()->markLastSendingAsDelivered();
    }
}

void AppCore::handleOfflineMessagePush(uint8_t flags, uint32_t sequence,
                                        const QByteArray& body)
{
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        qWarning() << "AppCore: invalid offline message format";
        return;
    }

    QJsonObject obj = doc.object();
    uint64_t senderId = static_cast<uint64_t>(obj["senderId"].toInteger(0));
    QString content = obj["content"].toString();
    qint64 timestamp = obj["timestamp"].toInteger(QDateTime::currentSecsSinceEpoch());
    QDateTime msgTime = QDateTime::fromSecsSinceEpoch(timestamp);

    bool isEncrypted = flags & static_cast<uint8_t>(chatroom::protocol::MessageFlag::Encrypted);

    // Try to decrypt for display in conversation list
    QString displayContent = content;
    if (isEncrypted) {
        auto* session = m_appManager->sessionManager();
        if (session->hasSessionKey(senderId)) {
            QByteArray sessionKey = session->getSessionKey(senderId);
            QByteArray encryptedData = QByteArray::fromBase64(content.toUtf8());
            QByteArray decrypted = chatroom::crypto::CryptoUtils::aes256GcmDecrypt(
                encryptedData, sessionKey);
            if (!decrypted.isEmpty()) {
                displayContent = QString::fromUtf8(decrypted);
            } else {
                displayContent = "[Encrypted message]";
            }
        } else {
            displayContent = "[Encrypted message]";
        }
    }

    m_chatController->onOfflineMessageReceived(senderId, content, msgTime, isEncrypted);
    m_conversationModel->updateConversation(senderId, displayContent, msgTime, 1);

    qDebug() << "AppCore: offline message received from" << senderId;
}

void AppCore::handleOfflineMessageDone(uint8_t flags, uint32_t sequence,
                                        const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)
    Q_UNUSED(body)
    qDebug() << "AppCore: all offline messages received";
}

void AppCore::onNeedKeyExchange(quint64 friendId)
{
    // Get local RSA public key
    auto [pubKey, privKey] = m_appManager->databaseManager()->loadKeyPair();
    QByteArray publicKey = pubKey;
    if (publicKey.isEmpty()) {
        // Generate new key pair
        auto keyPair = chatroom::crypto::CryptoUtils::generateRsaKeyPair();
        m_appManager->databaseManager()->saveKeyPair(keyPair.first, keyPair.second);
        m_appManager->sessionManager()->setLocalPublicKey(keyPair.first);
        m_appManager->sessionManager()->setLocalPrivateKey(keyPair.second);
        publicKey = keyPair.first;
    }

    QJsonObject bodyObj;
    bodyObj["publicKey"] = QString(publicKey.toBase64());
    bodyObj["targetUserId"] = static_cast<qint64>(friendId);
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::KeyExchangeRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();
    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
}

void AppCore::handleKeyExchangeRequest(uint8_t flags, uint32_t sequence, const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    quint64 senderId = static_cast<quint64>(doc.object()["fromUserId"].toInteger(0));
    QString peerPubKeyB64 = doc.object()["publicKey"].toString();
    QByteArray peerPublicKey = QByteArray::fromBase64(peerPubKeyB64.toUtf8());

    // Generate AES-256 session key
    QByteArray sessionKey = chatroom::crypto::CryptoUtils::generateAesKey();

    // Encrypt AES key with peer's RSA public key
    QByteArray encryptedKey = chatroom::crypto::CryptoUtils::rsaEncrypt(sessionKey, peerPublicKey);

    // Store session key locally
    m_appManager->sessionManager()->storeSessionKey(senderId, sessionKey);
    m_appManager->databaseManager()->saveSessionKey(senderId, sessionKey);

    // Get local RSA public key
    auto [pubKey, privKey] = m_appManager->databaseManager()->loadKeyPair();
    QByteArray myPublicKey = pubKey;

    // Send response
    QJsonObject respObj;
    respObj["fromUserId"] = static_cast<qint64>(m_appManager->sessionManager()->userId());
    respObj["publicKey"] = QString(myPublicKey.toBase64());
    respObj["encryptedKey"] = QString(encryptedKey.toBase64());
    QByteArray respBody = QJsonDocument(respObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::KeyExchangeResponse);
    uint8_t respFlags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsResponse);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();
    m_appManager->tcpClient()->sendPacket(type, respFlags, seq, respBody);

    qDebug() << "Key exchange completed with user" << senderId;
}

void AppCore::handleKeyExchangeResponse(uint8_t flags, uint32_t sequence,
                                         const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        qWarning() << "AppCore: invalid key exchange response";
        return;
    }

    QJsonObject obj = doc.object();
    quint64 fromUserId = static_cast<quint64>(obj["fromUserId"].toInteger(0));
    QString peerPubKeyB64 = obj["publicKey"].toString();
    QString encryptedKeyB64 = obj["encryptedKey"].toString();

    // Save peer's public key
    QByteArray peerPublicKey = QByteArray::fromBase64(peerPubKeyB64.toUtf8());

    // Decrypt AES key with our private key
    QByteArray myPrivateKey = m_appManager->sessionManager()->getLocalPrivateKey();
    if (myPrivateKey.isEmpty()) {
        auto [pubKey, privKey] = m_appManager->databaseManager()->loadKeyPair();
        myPrivateKey = privKey;
    }
    QByteArray sessionKey = chatroom::crypto::CryptoUtils::rsaDecrypt(
        QByteArray::fromBase64(encryptedKeyB64.toUtf8()), myPrivateKey);

    if (sessionKey.isEmpty()) {
        qWarning() << "Failed to decrypt session key from user" << fromUserId;
        return;
    }

    // Store session key locally
    m_appManager->sessionManager()->storeSessionKey(fromUserId, sessionKey);
    m_appManager->databaseManager()->saveSessionKey(fromUserId, sessionKey);

    qDebug() << "Key exchange completed with user" << fromUserId;
}

void AppCore::triggerNotification(quint64 senderId)
{
    int count = m_pendingMessageCounts.take(senderId);
    QTimer* timer = m_notificationTimers.take(senderId);
    delete timer;

    if (count <= 0) return;

    // Get sender name
    auto user = m_appManager->databaseManager()->getCachedUser(senderId);
    QString name = user.nickname.isEmpty() ? user.username : user.nickname;

    playNotificationSound();

    emit newMessagesReceived(senderId, name, count);
}

void AppCore::playNotificationSound()
{
#ifdef CHATROOM_HAS_MULTIMEDIA
    if (m_messageSound) {
        m_messageSound->play();
    }
#else
    // Fallback: use Windows API
#ifdef Q_OS_WIN
    ::MessageBeep(MB_ICONASTERISK);
#endif
#endif
}



void AppCore::setChatBackground(const QString& colorOrPath) {
    m_chatBackground = colorOrPath;
    m_appManager->databaseManager()->saveSetting("chat_background", colorOrPath);
    emit chatBackgroundChanged();
}

void AppCore::clearChatBackground() {
    m_chatBackground = "";
    m_appManager->databaseManager()->saveSetting("chat_background", "");
    emit chatBackgroundChanged();
}


} // namespace chatroom::client
