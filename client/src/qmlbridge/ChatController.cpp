#include "ChatController.h"
#include "core/AppManager.h"
#include "core/SessionManager.h"
#include "database/DatabaseManager.h"
#include "models/MessageModel.h"
#include "network/TcpClient.h"
#include "protocol/ChatProtocol.h"
#include "protocol/MessageTypes.h"
#include "crypto/CryptoUtils.h"
#include "models/ChatMessage.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QUrl>
#include <algorithm>

using namespace chatroom::models;

namespace chatroom::client {

ChatController::ChatController(AppManager* appManager, QObject* parent)
    : QObject(parent)
    , m_appManager(appManager)
    , m_messageModel(new MessageModel(this))
{
}

MessageModel* ChatController::messageModel() const
{
    return m_messageModel;
}

quint64 ChatController::currentChatFriendId() const
{
    return m_currentChatFriendId;
}

void ChatController::setCurrentChatFriendId(quint64 friendId)
{
    if (m_currentChatFriendId == friendId) {
        return;
    }
    m_currentChatFriendId = friendId;

    // Look up nickname from database cache
    auto user = m_appManager->databaseManager()->getCachedUser(friendId);
    m_currentChatNickname = user.nickname.isEmpty() ? user.username : user.nickname;

    // Mark messages as read in database
    m_appManager->databaseManager()->markAsRead(friendId);

    // Notify that this conversation's unread count should be cleared
    emit unreadCleared(friendId);

    emit currentChatFriendIdChanged();
}

QString ChatController::currentChatNickname() const
{
    return m_currentChatNickname;
}

quint64 ChatController::currentGroupId() const
{
    return m_currentGroupId;
}

void ChatController::setCurrentGroupId(quint64 groupId)
{
    if (m_currentGroupId == groupId) {
        return;
    }
    m_currentGroupId = groupId;

    // Clear friend chat when switching to group chat
    m_currentChatFriendId = 0;

    // Look up group name from database or group model
    // For now, use the group name if available
    if (groupId > 0) {
        // Load group chat history
        loadGroupChatHistory(groupId);
    } else {
        m_currentGroupName.clear();
        m_messageModel->clear();
    }

    emit currentGroupIdChanged();
}

QString ChatController::currentGroupName() const
{
    return m_currentGroupName;
}

bool ChatController::isInGroupChat() const
{
    return m_currentGroupId > 0;
}

void ChatController::sendGroupMessage(const QString& content)
{
    if (content.isEmpty()) {
        return;
    }

    if (m_currentGroupId == 0) {
        qWarning() << "ChatController::sendGroupMessage: no group chat target selected";
        return;
    }

    auto* session = m_appManager->sessionManager();
    auto* tcp = m_appManager->tcpClient();

    // Build GroupMessage JSON body
    QJsonObject bodyObj;
    bodyObj["groupId"] = static_cast<qint64>(m_currentGroupId);
    bodyObj["content"] = content;
    bodyObj["timestamp"] = QDateTime::currentSecsSinceEpoch();
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupMessage);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();
    tcp->sendPacket(type, 0, seq, body);

    // Save to local database and add to model
    chatroom::models::ChatMessage msg;
    msg.senderId = session->userId();
    msg.groupId = m_currentGroupId;
    msg.senderNickname = "";
    msg.content = content;
    msg.timestamp = QDateTime::currentDateTime();
    msg.isMine = true;
    msg.isRead = true;
    msg.type = chatroom::models::ChatMessageType::Text;
    msg.status = chatroom::models::ChatMessage::MessageStatus::Sending;

    qint64 insertedId = m_appManager->databaseManager()->saveGroupMessage(msg);
    if (insertedId > 0) {
        msg.messageId = static_cast<uint64_t>(insertedId);
    }
    m_messageModel->addMessage(msg);

    // Register sequence -> messageId for ACK matching
    registerPendingAck(seq, msg.messageId);

    emit messageSent();
}

void ChatController::loadGroupChatHistory(quint64 groupId)
{
    QVector<chatroom::models::ChatMessage> messages =
        m_appManager->databaseManager()->getGroupMessages(groupId, 100, 0);
    m_messageModel->loadMessages(messages);
}

void ChatController::sendMessage(const QString& content)
{
    if (content.isEmpty()) {
        return;
    }

    if (m_currentChatFriendId == 0) {
        qWarning() << "ChatController::sendMessage: no chat target selected";
        return;
    }

    auto* session = m_appManager->sessionManager();
    auto* tcp = m_appManager->tcpClient();

    QByteArray contentBytes = content.toUtf8();
    uint8_t flags = 0;

    // Encrypt if session key exists
    if (session->hasSessionKey(m_currentChatFriendId)) {
        QByteArray sessionKey = session->getSessionKey(m_currentChatFriendId);
        QByteArray iv = chatroom::crypto::CryptoUtils::generateIv();
        QByteArray encrypted = chatroom::crypto::CryptoUtils::aes256GcmEncrypt(
            contentBytes, sessionKey, iv);
        contentBytes = encrypted;
        flags |= static_cast<uint8_t>(chatroom::protocol::MessageFlag::Encrypted);
    } else {
        // No session key, trigger key exchange
        emit needKeyExchange(m_currentChatFriendId);
    }

    // Build JSON body
    QJsonObject bodyObj;
    bodyObj["receiverId"] = static_cast<qint64>(m_currentChatFriendId);
    // Only base64-encode encrypted content; send plaintext as-is
    if (flags & static_cast<uint8_t>(chatroom::protocol::MessageFlag::Encrypted)) {
        bodyObj["content"] = QString(contentBytes.toBase64());
    } else {
        bodyObj["content"] = content;
    }
    bodyObj["timestamp"] = QDateTime::currentSecsSinceEpoch();
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::ChatMessage);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();
    tcp->sendPacket(type, flags, seq, body);

    // Save to local database and add to model
    chatroom::models::ChatMessage msg;
    msg.senderId = session->userId();
    msg.receiverId = m_currentChatFriendId;
    msg.content = content;
    msg.timestamp = QDateTime::currentDateTime();
    msg.isMine = true;
    msg.isRead = true;
    msg.type = chatroom::models::ChatMessageType::Text;
    msg.status = chatroom::models::ChatMessage::MessageStatus::Sending;

    qint64 insertedId = m_appManager->databaseManager()->saveMessage(msg);
    if (insertedId > 0) {
        msg.messageId = static_cast<uint64_t>(insertedId);
    }
    m_messageModel->addMessage(msg);

    // Register sequence -> messageId for ACK matching
    registerPendingAck(seq, msg.messageId);

    emit messageSent();
}

void ChatController::loadChatHistory(quint64 friendId)
{
    QVector<chatroom::models::ChatMessage> messages =
        m_appManager->databaseManager()->getMessages(friendId, 100, 0);
    m_messageModel->loadMessages(messages);
}

void ChatController::clearCurrentChat()
{
    m_currentChatFriendId = 0;
    m_currentChatNickname.clear();
    m_messageModel->clear();
    emit currentChatFriendIdChanged();
}

void ChatController::clearAllHistory()
{
    m_appManager->databaseManager()->clearAllMessages();
    m_messageModel->clear();
    m_currentChatFriendId = 0;
    m_currentChatNickname.clear();
    emit currentChatFriendIdChanged();
}

void ChatController::searchMessages(const QString& keyword)
{
    if (keyword.isEmpty() || m_currentChatFriendId == 0) {
        // Clear search, restore original messages
        loadChatHistory(m_currentChatFriendId);
        return;
    }
    auto messages = m_appManager->databaseManager()->searchMessages(m_currentChatFriendId, keyword);
    // Reverse to chronological order (search results are in reverse chronological order)
    std::reverse(messages.begin(), messages.end());
    m_messageModel->loadMessages(messages);
    emit currentChatFriendIdChanged();
}

void ChatController::clearSearch()
{
    if (m_currentChatFriendId != 0) {
        loadChatHistory(m_currentChatFriendId);
    }
}

void ChatController::recallMessage(qint64 messageId)
{
    QJsonObject bodyObj;
    bodyObj["messageId"] = messageId;
    bodyObj["receiverId"] = static_cast<qint64>(m_currentChatFriendId);
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::MessageRecallRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();
    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);

    // Update locally
    m_messageModel->updateMessageContent(messageId, "[你撤回了一条消息]");
    m_appManager->databaseManager()->updateMessageContent(messageId, "[你撤回了一条消息]");
}

void ChatController::uploadAvatar(const QString& imagePath)
{
    QFile file(imagePath);
    if (!file.open(QIODevice::ReadOnly)) return;
    QByteArray imageData = file.readAll();
    file.close();

    // Limit to 64KB
    if (imageData.size() > 64 * 1024) {
        qWarning() << "Avatar too large, max 64KB";
        return;
    }

    // Save a copy locally for display
    QString localAvatarDir = QCoreApplication::applicationDirPath() + "/avatars";
    QDir().mkpath(localAvatarDir);
    QString localAvatarPath = localAvatarDir + "/" +
        QString::number(m_appManager->sessionManager()->userId()) + ".png";
    QFile localFile(localAvatarPath);
    if (localFile.open(QIODevice::WriteOnly)) {
        localFile.write(imageData);
        localFile.close();
        // Update session avatar to local file URL
        QString fileUrl = QUrl::fromLocalFile(localAvatarPath).toString();
        m_appManager->sessionManager()->setAvatar(fileUrl);
        qDebug() << "Avatar saved locally:" << fileUrl;
    }

    QJsonObject bodyObj;
    bodyObj["avatarData"] = QString(imageData.toBase64());
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::AvatarUploadRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();
    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
}

void ChatController::onChatMessageReceived(uint64_t senderId, const QString& content,
                                            const QDateTime& timestamp, bool encrypted)
{
    auto* session = m_appManager->sessionManager();
    QString decryptedContent = content;

    // Decrypt if needed
    if (encrypted && session->hasSessionKey(senderId)) {
        QByteArray sessionKey = session->getSessionKey(senderId);
        QByteArray encryptedData = QByteArray::fromBase64(content.toUtf8());
        QByteArray decrypted = chatroom::crypto::CryptoUtils::aes256GcmDecrypt(
            encryptedData, sessionKey);
        if (!decrypted.isEmpty()) {
            decryptedContent = QString::fromUtf8(decrypted);
        } else {
            qWarning() << "ChatController: failed to decrypt message from" << senderId;
            decryptedContent = "[Decryption failed]";
        }
    } else if (encrypted) {
        decryptedContent = "[Encrypted - key not available]";
    }

    // Save to database
    chatroom::models::ChatMessage msg;
    msg.senderId = senderId;
    msg.receiverId = session->userId();
    msg.content = decryptedContent;
    msg.timestamp = timestamp;
    msg.isMine = false;
    msg.isRead = (senderId == m_currentChatFriendId);
    msg.type = chatroom::models::ChatMessageType::Text;

    qint64 insertedId = m_appManager->databaseManager()->saveMessage(msg);
    if (insertedId > 0) {
        msg.messageId = static_cast<uint64_t>(insertedId);
    }

    // Add to model if currently viewing this chat
    if (senderId == m_currentChatFriendId) {
        m_messageModel->addMessage(msg);
    }
}

void ChatController::onOfflineMessageReceived(uint64_t senderId, const QString& content,
                                               const QDateTime& timestamp, bool encrypted)
{
    // Offline messages are handled the same way as real-time messages
    onChatMessageReceived(senderId, content, timestamp, encrypted);
}

void ChatController::registerPendingAck(uint32_t sequence, uint64_t messageId)
{
    m_pendingAcks.insert(sequence, messageId);
    // Clean up old entries (keep max 500)
    while (m_pendingAcks.size() > 500) {
        m_pendingAcks.erase(m_pendingAcks.begin());
    }
}

bool ChatController::acknowledgeMessage(uint32_t sequence)
{
    auto it = m_pendingAcks.find(sequence);
    if (it != m_pendingAcks.end()) {
        uint64_t messageId = it.value();
        m_pendingAcks.erase(it);
        // Update the specific message status in the model
        for (int i = 0; i < m_messageModel->rowCount(); ++i) {
            QModelIndex idx = m_messageModel->index(i, 0);
            QVariant idVar = m_messageModel->data(idx, MessageModel::MessageIdRole);
            if (idVar.toULongLong() == messageId) {
                m_messageModel->updateMessageStatus(i,
                    chatroom::models::ChatMessage::MessageStatus::Delivered);
                return true;
            }
        }
    }
    return false;
}

// ===================================================================
// File transfer
// ===================================================================
void ChatController::sendFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ChatController: cannot open file" << filePath;
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    const qint64 MAX_SIZE = 50 * 1024 * 1024; // 50MB
    if (data.size() > MAX_SIZE) {
        qWarning() << "ChatController: file too large" << data.size();
        return;
    }

    QFileInfo fi(filePath);
    QJsonObject obj;
    obj["fileName"] = fi.fileName();
    obj["fileSize"] = static_cast<qint64>(data.size());
    obj["mimeType"] = "application/octet-stream";
    obj["fileData"] = QString::fromUtf8(data.toBase64());
    obj["targetUserId"] = static_cast<qint64>(m_currentChatFriendId);
    obj["targetGroupId"] = static_cast<qint64>(m_currentGroupId);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::FileUploadRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();
    m_appManager->tcpClient()->sendPacket(
        type, 0, seq,
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    qDebug() << "ChatController: file sent" << fi.fileName() << data.size() << "bytes";
}

void ChatController::downloadFile(quint64 fileId)
{
    QJsonObject obj;
    obj["fileId"] = static_cast<qint64>(fileId);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::FileDownloadRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();
    m_appManager->tcpClient()->sendPacket(
        type, 0, seq,
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    qDebug() << "ChatController: file download requested, fileId:" << fileId;
}

void ChatController::handleFileUploadResponse(const QByteArray& body)
{
    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    if (obj["success"].toBool()) {
        // File uploaded successfully, fileId available
        quint64 fileId = static_cast<quint64>(obj["fileId"].toInteger(0));
        QString fileName = obj["fileName"].toString();

        // Add file message to local model
        chatroom::models::ChatMessage msg;
        msg.messageId = fileId;
        msg.content = fileName;
        msg.isMine = true;
        msg.type = ChatMessageType::File;
        msg.timestamp = QDateTime::currentDateTime();
        msg.status = ChatMessage::MessageStatus::Sent;
        m_messageModel->addMessage(msg);

        qDebug() << "ChatController: file uploaded" << fileName << "fileId:" << fileId;
    }
}

void ChatController::handleFileDownloadResponse(const QByteArray& body)
{
    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    if (obj["success"].toBool()) {
        QString fileName = obj["fileName"].toString();
        QString fileDataB64 = obj["fileData"].toString();
        QByteArray data = QByteArray::fromBase64(fileDataB64.toUtf8());

        // Save to Downloads directory
        QString savePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
                           + "/" + fileName;

        // Handle name conflicts
        if (QFile::exists(savePath)) {
            int dot = fileName.lastIndexOf('.');
            QString base = (dot > 0) ? fileName.left(dot) : fileName;
            QString ext  = (dot > 0) ? fileName.mid(dot) : "";
            for (int i = 1; i < 100; ++i) {
                savePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
                           + "/" + base + "(" + QString::number(i) + ")" + ext;
                if (!QFile::exists(savePath)) break;
            }
        }

        QFile outFile(savePath);
        if (outFile.open(QIODevice::WriteOnly)) {
            outFile.write(data);
            outFile.close();
            qDebug() << "ChatController: file saved to" << savePath;
        }
    }
}

} // namespace chatroom::client
