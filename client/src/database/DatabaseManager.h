#pragma once

#include <QSqlDatabase>
#include <QVector>
#include <QString>
#include <cstdint>
#include <utility>
#include "models/ChatMessage.h"
#include "models/UserInfo.h"

namespace chatroom::client {

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool initialize(const QString& dbPath = "chatroom_client.db");

    // Message storage
    qint64 saveMessage(const chatroom::models::ChatMessage& msg);
    QVector<chatroom::models::ChatMessage> getMessages(uint64_t friendId,
                                                       int limit = 50,
                                                       int offset = 0);
    int getUnreadCount(uint64_t friendId);
    bool markAsRead(uint64_t friendId);
    bool clearHistory(uint64_t friendId);
    bool clearAllMessages();
    bool updateMessageContent(qint64 messageId, const QString& newContent);
    QVector<chatroom::models::ChatMessage> searchMessages(quint64 friendId, const QString& keyword, int limit = 50);

    // Conversation list
    struct ConversationInfo {
        uint64_t    friendId;
        QString     nickname;
        QString     avatar;
        QString     lastMessage;
        QDateTime   lastTime;
        int         unreadCount;
    };
    QVector<ConversationInfo> getConversationList();

    // User/contact cache
    bool upsertUser(const chatroom::models::UserInfo& user);
    chatroom::models::UserInfo getCachedUser(uint64_t userId);

    // Key persistence
    bool saveKeyPair(const QByteArray& publicKey, const QByteArray& privateKey);
    std::pair<QByteArray, QByteArray> loadKeyPair();
    bool saveSessionKey(uint64_t friendId, const QByteArray& key);
    QByteArray loadSessionKey(uint64_t friendId);

    // Group message storage
    qint64 saveGroupMessage(const chatroom::models::ChatMessage& msg);
    QVector<chatroom::models::ChatMessage> getGroupMessages(uint64_t groupId, int limit = 100, int offset = 0);
    int getGroupUnreadCount(uint64_t groupId);
    bool markGroupAsRead(uint64_t groupId);

    // Group cache
    bool upsertGroupCache(uint64_t groupId, const QString& name, const QString& avatar,
                          uint64_t ownerId, int memberCount, bool isDefault, uint64_t lastReadMsgId = 0);
    bool removeGroupCache(uint64_t groupId);

    // Group members cache
    bool upsertGroupMembersCache(uint64_t groupId, const QVector<QVariantMap>& members);
    QVector<QVariantMap> getCachedGroupMembers(uint64_t groupId);
    bool removeGroupMembersCache(uint64_t groupId);


  public:
    void saveSetting(const QString& key, const QString& value);
    QString loadSetting(const QString& key, const QString& defaultValue = QString());

private:
    void createTables();

    QSqlDatabase m_db;
    QString m_connectionName;
};

} // namespace chatroom::client


