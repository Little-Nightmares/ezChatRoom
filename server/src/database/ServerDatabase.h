#pragma once

#include <QSqlDatabase>
#include <QString>
#include <QVector>
#include <cstdint>
#include "models/UserInfo.h"
#include "models/ChatMessage.h"
#include "models/FriendRequest.h"

using chatroom::models::UserInfo;
using chatroom::models::ChatMessage;
using chatroom::models::FriendRequest;

namespace chatroom::server {

class ServerDatabase {
public:
    ServerDatabase();
    ~ServerDatabase();

    struct ServerStats {
        int totalUsers = 0;
        int onlineUsers = 0;
        int totalFriendships = 0;
        int totalOfflineMessages = 0;
        int pendingFriendRequests = 0;
        int totalMessages = 0;
        int totalGroups = 0;
        int totalGroupMessages = 0;
    };

    // Group info struct
    struct GroupInfo {
        uint64_t groupId = 0;
        QString name;
        QString avatar;
        uint64_t ownerId = 0;
        int memberCount = 0;
        bool isDefault = false;
        int maxMembers = 100;
        qint64 createdAt = 0;
    };

    struct GroupMemberInfo {
        uint64_t userId = 0;
        QString username;
        QString nickname;
        QString avatar;
        int role = 0;  // 0=member, 1=owner
    };

    bool initialize(const QString& dbPath);

    // User management
    // Returns 0 on success, 1 if username already exists (UNIQUE constraint), 2 on other errors
    int registerUser(const QString& username, const QString& passwordHash,
                     const QString& nickname);
    uint64_t authenticateUser(const QString& username, const QString& passwordHash);
    UserInfo getUserInfo(uint64_t userId);
    UserInfo getUserByUsername(const QString& username);

    // User search (fuzzy match by username or nickname)
    QVector<UserInfo> searchUsers(const QString& keyword, int limit = 20);

    // Admin queries
    QVector<UserInfo> getAllUsers(int offset = 0, int limit = 100);
    int getUserCount();
    ServerStats getStatistics();

    // Friend management
    uint64_t sendFriendRequest(uint64_t fromUserId, uint64_t toUserId,
                               const QString& message);
    bool acceptFriendRequest(uint64_t requestId);
    bool rejectFriendRequest(uint64_t requestId);
    bool deleteFriend(uint64_t userId, uint64_t friendId);
    QVector<UserInfo> getFriendList(uint64_t userId);
    QVector<FriendRequest> getPendingFriendRequests(uint64_t userId);
    bool isFriend(uint64_t userId, uint64_t friendId);

    // Offline messages
    bool storeOfflineMessage(const ChatMessage& msg);
    QVector<ChatMessage> getOfflineMessages(uint64_t userId);
    bool clearOfflineMessages(uint64_t userId);

    // Persistent private chat messages (for search)
    bool storeChatMessage(uint64_t senderId, uint64_t receiverId, const QString& content,
                          int msgType, qint64 timestamp);

    // Group management methods
    uint64_t createGroup(const QString& name, uint64_t ownerId, bool isDefault = false, int maxMembers = 100);
    bool updateGroup(uint64_t groupId, const QString& name, const QString& avatar, int maxMembers = -1);
    bool dissolveGroup(uint64_t groupId);
    GroupInfo getGroupInfo(uint64_t groupId);
    GroupInfo getDefaultGroup();
    bool saveSetting(const QString& key, const QString& value);
    QString loadSetting(const QString& key, const QString& defaultValue = QString());
    QVector<GroupInfo> getUserGroups(uint64_t userId);
    int getGroupCount();

    // Group member management
    bool addGroupMember(uint64_t groupId, uint64_t userId, int role = 0);
    bool addGroupMembers(uint64_t groupId, const QVector<quint64>& userIds);
    bool removeGroupMember(uint64_t groupId, uint64_t userId);
    bool isGroupMember(uint64_t groupId, uint64_t userId);
    bool isGroupOwner(uint64_t groupId, uint64_t userId);
    QVector<GroupMemberInfo> getGroupMembers(uint64_t groupId);
    int getGroupMemberCount(uint64_t groupId);
    QVector<quint64> getGroupMemberIds(uint64_t groupId);

    // Group owner transfer (transaction)
    bool transferGroupOwner(uint64_t groupId, uint64_t newOwnerId);

    // Group messages
    uint64_t storeGroupMessage(uint64_t groupId, uint64_t senderId, const QString& content, int msgType, qint64 timestamp);
    QVector<chatroom::models::ChatMessage> getGroupOfflineMessages(uint64_t groupId, uint64_t lastReadMsgId);
    bool updateLastReadMsgId(uint64_t groupId, uint64_t userId, uint64_t lastReadMsgId);
    QVector<chatroom::models::ChatMessage> getRecentGroupMessages(int limit = 50);
    int getGroupMessageCount();

    // File metadata
    struct FileMeta {
        uint64_t fileId = 0;
        uint64_t senderId = 0;
        uint64_t receiverId = 0;
        uint64_t groupId = 0;
        QString fileName;
        qint64 fileSize = 0;
        QString filePath;
        QString mimeType;
        QString sha256Hash;
        qint64 createdAt = 0;
    };
    uint64_t saveFileMeta(uint64_t senderId, uint64_t receiverId, uint64_t groupId,
                          const QString& fileName, qint64 fileSize,
                          const QString& filePath, const QString& mimeType,
                          const QString& sha256Hash);
    FileMeta getFileMeta(uint64_t fileId);

    // Group encryption keys
    bool saveGroupEncryptedKey(uint64_t groupId, uint64_t userId, const QString& encryptedKey);
    QString getGroupEncryptedKey(uint64_t groupId, uint64_t userId);
    bool removeGroupEncryptedKey(uint64_t groupId, uint64_t userId);

    // Group announcements
    bool setAnnouncement(uint64_t groupId, uint64_t senderId, const QString& content);
    QVector<QPair<QString, qint64>> getAnnouncements(uint64_t groupId, int limit = 5);

    // Group message read status
    bool markGroupMessageRead(uint64_t msgId, uint64_t groupId, uint64_t userId);
    int getGroupMessageReadCount(uint64_t msgId, uint64_t groupId);

    // Message search
    struct SearchResult {
        uint64_t messageId = 0;
        uint64_t senderId = 0;
        QString senderName;
        QString content;
        QString snippet;
        qint64 timestamp = 0;
        bool isGroup = false;
        uint64_t groupId = 0;
        QString groupName;
    };
    QVector<SearchResult> searchMessages(uint64_t userId, const QString& keyword,
                                         int limit = 50, int offset = 0);

    // Maintenance
    bool trimOldGroupMessages(uint64_t groupId, int maxCount);
    bool removeUserFromAllGroups(uint64_t userId);
    QVector<GroupInfo> getAllGroups(int offset = 0, int limit = 100);

    // Admin management
    bool deleteUser(uint64_t userId);
    bool resetUserPassword(uint64_t userId, const QString& newPasswordHash);
    QVector<FriendRequest> getAllFriendRequests(int offset = 0, int limit = 100);
    int getMessageCount();
    QVector<ChatMessage> getRecentMessages(int limit = 50);
    bool clearAllOfflineMessages();
    bool clearAllFriendRequests();
    bool clearAllMessages();
    bool clearDatabase();

    // Bot user management
    uint64_t getOrCreateBotUser();

private:
    void createTables();
    QSqlDatabase m_db;
};

} // namespace chatroom::server
