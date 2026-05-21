#include "database/ServerDatabase.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

using chatroom::models::UserInfo;
using chatroom::models::ChatMessage;
using chatroom::models::FriendRequest;
using chatroom::models::RequestStatus;
using chatroom::models::ChatMessageType;

namespace chatroom::server {

ServerDatabase::ServerDatabase() = default;

ServerDatabase::~ServerDatabase() {
    QString connectionName = m_db.connectionName();
    if (m_db.isOpen()) {
        m_db.close();
    }
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

bool ServerDatabase::initialize(const QString& dbPath) {
    m_db = QSqlDatabase::addDatabase("QSQLITE", "chatroom_server");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    // Enable WAL mode for better concurrent read performance
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    if (!query.exec("PRAGMA journal_mode = WAL;")) {
        qWarning() << "Failed to set WAL mode:" << query.lastError().text();
    }

    // Enable foreign keys
    if (!query.exec("PRAGMA foreign_keys = ON;")) {
        qWarning() << "Failed to enable foreign keys:" << query.lastError().text();
    }

    createTables();

    // Check if default group exists, create if not
    QSqlQuery checkDefault(QSqlDatabase::database("chatroom_server"));
    checkDefault.exec("SELECT group_id FROM groups WHERE is_default = 1");
    if (!checkDefault.next()) {
        QSqlQuery ins(QSqlDatabase::database("chatroom_server"));
        ins.prepare("INSERT INTO groups (name, is_default, owner_id, max_members, created_at) VALUES ('局域网大群', 1, 0, 75, ?)");
        ins.addBindValue(static_cast<qint64>(QDateTime::currentSecsSinceEpoch()));
        ins.exec();
    }

    return true;
}

void ServerDatabase::createTables() {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));

    // Users table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            user_id       INTEGER PRIMARY KEY AUTOINCREMENT,
            username      TEXT    NOT NULL UNIQUE,
            password_hash TEXT    NOT NULL,
            nickname      TEXT    NOT NULL DEFAULT '',
            avatar        TEXT    NOT NULL DEFAULT '',
            created_at    INTEGER NOT NULL DEFAULT 0
        );
    )")) {
        qCritical() << "Failed to create users table:" << query.lastError().text();
    }

    // Friendships table (bidirectional storage)
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS friendships (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id     INTEGER NOT NULL,
            friend_id   INTEGER NOT NULL,
            created_at  INTEGER NOT NULL DEFAULT 0,
            UNIQUE(user_id, friend_id),
            FOREIGN KEY (user_id)   REFERENCES users(user_id),
            FOREIGN KEY (friend_id) REFERENCES users(user_id)
        );
    )")) {
        qCritical() << "Failed to create friendships table:" << query.lastError().text();
    }

    // Friend requests table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS friend_requests (
            request_id    INTEGER PRIMARY KEY AUTOINCREMENT,
            from_user_id  INTEGER NOT NULL,
            to_user_id    INTEGER NOT NULL,
            message       TEXT    NOT NULL DEFAULT '',
            status        INTEGER NOT NULL DEFAULT 0,
            created_at    INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY (from_user_id) REFERENCES users(user_id),
            FOREIGN KEY (to_user_id)   REFERENCES users(user_id)
        );
    )")) {
        qCritical() << "Failed to create friend_requests table:" << query.lastError().text();
    }

    // Offline messages table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS offline_messages (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            sender_id   INTEGER NOT NULL,
            receiver_id INTEGER NOT NULL,
            content     TEXT    NOT NULL,
            msg_type    INTEGER NOT NULL DEFAULT 0,
            timestamp   INTEGER NOT NULL,
            created_at  INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY (sender_id)   REFERENCES users(user_id),
            FOREIGN KEY (receiver_id) REFERENCES users(user_id)
        );
    )")) {
        qCritical() << "Failed to create offline_messages table:" << query.lastError().text();
    }

    // Groups table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS groups (
            group_id    INTEGER PRIMARY KEY AUTOINCREMENT,
            name        TEXT    NOT NULL DEFAULT '',
            avatar      TEXT    NOT NULL DEFAULT '',
            owner_id    INTEGER NOT NULL,
            is_default  INTEGER NOT NULL DEFAULT 0,
            max_members INTEGER NOT NULL DEFAULT 100,
            created_at  INTEGER NOT NULL DEFAULT 0,
            updated_at  INTEGER NOT NULL DEFAULT 0
        );
    )")) {
        qCritical() << "Failed to create groups table:" << query.lastError().text();
    }

    // Group members table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS group_members (
            id               INTEGER PRIMARY KEY AUTOINCREMENT,
            group_id         INTEGER NOT NULL,
            user_id          INTEGER NOT NULL,
            role             INTEGER NOT NULL DEFAULT 0,
            joined_at        INTEGER NOT NULL DEFAULT 0,
            last_read_msg_id INTEGER NOT NULL DEFAULT 0,
            UNIQUE(group_id, user_id)
        );
    )")) {
        qCritical() << "Failed to create group_members table:" << query.lastError().text();
    }

    // Group messages table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS group_messages (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            group_id   INTEGER NOT NULL,
            sender_id  INTEGER NOT NULL,
            content    TEXT    NOT NULL,
            msg_type   INTEGER NOT NULL DEFAULT 0,
            timestamp  INTEGER NOT NULL,
            created_at INTEGER NOT NULL DEFAULT 0
        );
    )")) {
        qCritical() << "Failed to create group_messages table:" << query.lastError().text();
    }

    // Index for group_messages queries
    if (!query.exec(R"(
        CREATE INDEX IF NOT EXISTS idx_group_messages_group_id_id ON group_messages(group_id, id);
    )")) {
        qCritical() << "Failed to create group_messages index:" << query.lastError().text();
    }

    // File metadata table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS file_metadata (
            file_id     INTEGER PRIMARY KEY AUTOINCREMENT,
            sender_id   INTEGER NOT NULL,
            receiver_id INTEGER DEFAULT 0,
            group_id    INTEGER DEFAULT 0,
            file_name   TEXT    NOT NULL,
            file_size   INTEGER NOT NULL,
            file_path   TEXT    NOT NULL,
            mime_type   TEXT    DEFAULT '',
            sha256_hash TEXT    DEFAULT '',
            created_at  INTEGER NOT NULL,
            FOREIGN KEY (sender_id) REFERENCES users(user_id)
        );
    )")) {
        qWarning() << "Failed to create file_metadata table:" << query.lastError().text();
    }

    // Group announcements table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS group_announcements (
            announcement_id INTEGER PRIMARY KEY AUTOINCREMENT,
            group_id        INTEGER NOT NULL,
            sender_id       INTEGER NOT NULL,
            content         TEXT    NOT NULL,
            created_at      INTEGER NOT NULL,
            is_pinned       INTEGER DEFAULT 0,
            FOREIGN KEY (group_id) REFERENCES groups(group_id)
        );
    )")) {
        qWarning() << "Failed to create group_announcements table:" << query.lastError().text();
    }

    // Server settings table
    query.exec("CREATE TABLE IF NOT EXISTS server_settings (key TEXT PRIMARY KEY, value TEXT NOT NULL)");

    // Group encryption keys table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS group_encryption_keys (
            id       INTEGER PRIMARY KEY AUTOINCREMENT,
            group_id INTEGER NOT NULL,
            user_id  INTEGER NOT NULL,
            encrypted_key TEXT NOT NULL,
            created_at INTEGER NOT NULL,
            UNIQUE(group_id, user_id)
        );
    )")) {
        qWarning() << "Failed to create group_encryption_keys table:" << query.lastError().text();
    }

    // Group message read records table
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS group_message_reads (
            msg_id   INTEGER NOT NULL,
            group_id INTEGER NOT NULL,
            user_id  INTEGER NOT NULL,
            read_at  INTEGER NOT NULL,
            PRIMARY KEY (msg_id, user_id)
        );
    )")) {
        qWarning() << "Failed to create group_message_reads table:" << query.lastError().text();
    }

    // Persistent private chat messages (survives offline_messages cleanup, used for search)
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS chat_messages (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            sender_id   INTEGER NOT NULL,
            receiver_id INTEGER NOT NULL,
            content     TEXT    NOT NULL,
            msg_type    INTEGER NOT NULL DEFAULT 0,
            timestamp   INTEGER NOT NULL,
            created_at  INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY (sender_id)   REFERENCES users(user_id),
            FOREIGN KEY (receiver_id) REFERENCES users(user_id)
        );
    )")) {
        qCritical() << "Failed to create chat_messages table:" << query.lastError().text();
    }

    // Index for chat_messages search queries (sender_id + receiver_id + timestamp)
    if (!query.exec(R"(
        CREATE INDEX IF NOT EXISTS idx_chat_messages_sender_receiver
        ON chat_messages(sender_id, receiver_id, timestamp);
    )")) {
        qWarning() << "Failed to create chat_messages index:" << query.lastError().text();
    }

    // Try to create FTS indexes for full-text search (may fail if FTS5 not available)
    query.exec(R"(
        CREATE VIRTUAL TABLE IF NOT EXISTS offline_msgs_fts USING fts5(
            content,
            content=offline_messages,
            content_rowid=id,
            tokenize='unicode61'
        );
    )");
    query.exec(R"(
        CREATE VIRTUAL TABLE IF NOT EXISTS group_messages_fts USING fts5(
            content,
            content=group_messages,
            content_rowid=id,
            tokenize='unicode61'
        );
    )");
    // FTS triggers for offline_messages table
    query.exec(R"(
        CREATE TRIGGER IF NOT EXISTS offline_msgs_fts_insert AFTER INSERT ON offline_messages
        BEGIN
            INSERT INTO offline_msgs_fts(rowid, content) VALUES (new.id, new.content);
        END;
    )");
    // FTS triggers for group_messages table
    query.exec(R"(
        CREATE TRIGGER IF NOT EXISTS group_messages_fts_insert AFTER INSERT ON group_messages
        BEGIN
            INSERT INTO group_messages_fts(rowid, content) VALUES (new.id, new.content);
        END;
    )");

    // FTS virtual table + triggers for chat_messages
    query.exec(R"(
        CREATE VIRTUAL TABLE IF NOT EXISTS chat_messages_fts USING fts5(
            content,
            content=chat_messages,
            content_rowid=id,
            tokenize='unicode61'
        );
    )");
    query.exec(R"(
        CREATE TRIGGER IF NOT EXISTS chat_messages_fts_insert AFTER INSERT ON chat_messages
        BEGIN
            INSERT INTO chat_messages_fts(rowid, content) VALUES (new.id, new.content);
        END;
    )");
}

int ServerDatabase::registerUser(const QString& username, const QString& passwordHash,
                                  const QString& nickname) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("INSERT INTO users (username, password_hash, nickname, created_at) "
                  "VALUES (:username, :password_hash, :nickname, :created_at)");
    query.bindValue(":username", username);
    query.bindValue(":password_hash", passwordHash);
    query.bindValue(":nickname", nickname);
    query.bindValue(":created_at", static_cast<qint64>(QDateTime::currentDateTime().toSecsSinceEpoch()));

    if (!query.exec()) {
        QSqlError err = query.lastError();
        // SQLite native error code 2067 = SQLITE_CONSTRAINT_UNIQUE
        // Qt maps it to QSqlError::ConstraintViolation or we check the native code
        if (err.nativeErrorCode() == "2067" ||
            err.text().contains("UNIQUE constraint failed")) {
            qWarning() << "Username already exists:" << username;
            return 1;
        }
        qWarning() << "Failed to register user:" << err.text();
        return 2;
    }

    // Auto-join the new user to the default group (LAN group)
    uint64_t newUserId = static_cast<uint64_t>(query.lastInsertId().toULongLong());
    if (newUserId > 0) {
        GroupInfo defaultGroup = getDefaultGroup();
        if (defaultGroup.groupId > 0 && !isGroupMember(defaultGroup.groupId, newUserId)) {
            addGroupMember(defaultGroup.groupId, newUserId);
            qDebug() << "Default group: auto-joined user id" << newUserId;
        }
    }

    return 0;
}

uint64_t ServerDatabase::authenticateUser(const QString& username, const QString& passwordHash) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT user_id, password_hash FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec() || !query.next()) {
        return 0;
    }

    uint64_t userId = query.value("user_id").toULongLong();
    QString storedHash = query.value("password_hash").toString();

    if (storedHash == passwordHash) {
        return userId;
    }
    return 0;
}

UserInfo ServerDatabase::getUserInfo(uint64_t userId) {
    UserInfo info;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT user_id, username, nickname, avatar FROM users WHERE user_id = :user_id");
    query.bindValue(":user_id", static_cast<qulonglong>(userId));

    if (!query.exec() || !query.next()) {
        return info;
    }

    info.userId = query.value("user_id").toULongLong();
    info.username = query.value("username").toString();
    info.nickname = query.value("nickname").toString();
    info.avatar = query.value("avatar").toString();

    return info;
}

UserInfo ServerDatabase::getUserByUsername(const QString& username) {
    UserInfo info;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT user_id, username, nickname, avatar FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec() || !query.next()) {
        return info;
    }

    info.userId = query.value("user_id").toULongLong();
    info.username = query.value("username").toString();
    info.nickname = query.value("nickname").toString();
    info.avatar = query.value("avatar").toString();

    return info;
}

QVector<UserInfo> ServerDatabase::searchUsers(const QString& keyword, int limit) {
    QVector<UserInfo> users;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT user_id, username, nickname, avatar FROM users "
                   "WHERE username LIKE ? OR nickname LIKE ? LIMIT ?");
    QString pattern = "%" + keyword + "%";
    query.bindValue(0, pattern);
    query.bindValue(1, pattern);
    query.bindValue(2, limit);
    if (query.exec()) {
        while (query.next()) {
            UserInfo u;
            u.userId = query.value(0).toULongLong();
            u.username = query.value(1).toString();
            u.nickname = query.value(2).toString();
            u.avatar = query.value(3).toString();
            users.append(u);
        }
    }
    return users;
}

uint64_t ServerDatabase::sendFriendRequest(uint64_t fromUserId, uint64_t toUserId,
                                       const QString& message) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("INSERT INTO friend_requests (from_user_id, to_user_id, message, created_at) "
                  "VALUES (:from_user_id, :to_user_id, :message, :created_at)");
    query.bindValue(":from_user_id", static_cast<qulonglong>(fromUserId));
    query.bindValue(":to_user_id", static_cast<qulonglong>(toUserId));
    query.bindValue(":message", message);
    query.bindValue(":created_at", static_cast<qint64>(QDateTime::currentDateTime().toSecsSinceEpoch()));

    if (!query.exec()) {
        qWarning() << "Failed to send friend request:" << query.lastError().text();
        return 0;
    }
    return query.lastInsertId().toULongLong();
}

bool ServerDatabase::acceptFriendRequest(uint64_t requestId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));

    // First, get the request details
    query.prepare(
        "SELECT from_user_id, to_user_id FROM friend_requests "
        "WHERE request_id = :request_id AND status = 0");
    query.bindValue(":request_id", static_cast<qulonglong>(requestId));

    if (!query.exec() || !query.next()) {
        qWarning() << "Friend request not found or already processed:" << requestId;
        return false;
    }

    uint64_t fromUserId = query.value("from_user_id").toULongLong();
    uint64_t toUserId = query.value("to_user_id").toULongLong();

    // Update request status to accepted
    query.prepare("UPDATE friend_requests SET status = 1 WHERE request_id = :request_id");
    query.bindValue(":request_id", static_cast<qulonglong>(requestId));

    if (!query.exec()) {
        qWarning() << "Failed to update friend request status:" << query.lastError().text();
        return false;
    }

    // Insert bidirectional friendship: from -> to
    query.prepare("INSERT OR IGNORE INTO friendships (user_id, friend_id, created_at) "
                  "VALUES (:user_id, :friend_id, :created_at)");
    query.bindValue(":user_id", static_cast<qulonglong>(fromUserId));
    query.bindValue(":friend_id", static_cast<qulonglong>(toUserId));
    query.bindValue(":created_at", static_cast<qint64>(QDateTime::currentDateTime().toSecsSinceEpoch()));

    if (!query.exec()) {
        qWarning() << "Failed to insert friendship (from->to):" << query.lastError().text();
        return false;
    }

    // Insert bidirectional friendship: to -> from
    query.prepare("INSERT OR IGNORE INTO friendships (user_id, friend_id, created_at) "
                  "VALUES (:user_id, :friend_id, :created_at)");
    query.bindValue(":user_id", static_cast<qulonglong>(toUserId));
    query.bindValue(":friend_id", static_cast<qulonglong>(fromUserId));
    query.bindValue(":created_at", static_cast<qint64>(QDateTime::currentDateTime().toSecsSinceEpoch()));

    if (!query.exec()) {
        qWarning() << "Failed to insert friendship (to->from):" << query.lastError().text();
        return false;
    }

    return true;
}

bool ServerDatabase::rejectFriendRequest(uint64_t requestId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "UPDATE friend_requests SET status = 2 WHERE request_id = :request_id AND status = 0");
    query.bindValue(":request_id", static_cast<qulonglong>(requestId));

    if (!query.exec()) {
        qWarning() << "Failed to reject friend request:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool ServerDatabase::deleteFriend(uint64_t userId, uint64_t friendId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));

    // Delete friendship in both directions
    query.prepare(
        "DELETE FROM friendships "
        "WHERE (user_id = :user_id1 AND friend_id = :friend_id1) "
           "OR (user_id = :user_id2 AND friend_id = :friend_id2)");
    query.bindValue(":user_id1", static_cast<qulonglong>(userId));
    query.bindValue(":friend_id1", static_cast<qulonglong>(friendId));
    query.bindValue(":user_id2", static_cast<qulonglong>(friendId));
    query.bindValue(":friend_id2", static_cast<qulonglong>(userId));

    if (!query.exec()) {
        qWarning() << "Failed to delete friendship:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QVector<UserInfo> ServerDatabase::getFriendList(uint64_t userId) {
    QVector<UserInfo> friends;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "SELECT u.user_id, u.username, u.nickname, u.avatar "
        "FROM friendships f "
        "JOIN users u ON u.user_id = f.friend_id "
        "WHERE f.user_id = :user_id");
    query.bindValue(":user_id", static_cast<qulonglong>(userId));

    if (!query.exec()) {
        qWarning() << "Failed to get friend list:" << query.lastError().text();
        return friends;
    }

    while (query.next()) {
        UserInfo info;
        info.userId = query.value("user_id").toULongLong();
        info.username = query.value("username").toString();
        info.nickname = query.value("nickname").toString();
        info.avatar = query.value("avatar").toString();
        friends.append(info);
    }

    return friends;
}

QVector<FriendRequest> ServerDatabase::getPendingFriendRequests(uint64_t userId) {
    QVector<FriendRequest> requests;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "SELECT fr.request_id, fr.from_user_id, fr.to_user_id, fr.message, fr.status, "
               "u.username AS from_username "
        "FROM friend_requests fr "
        "JOIN users u ON u.user_id = fr.from_user_id "
        "WHERE fr.to_user_id = :user_id AND fr.status = 0 "
        "ORDER BY fr.created_at ASC");
    query.bindValue(":user_id", static_cast<qulonglong>(userId));

    if (!query.exec()) {
        qWarning() << "Failed to get pending friend requests:" << query.lastError().text();
        return requests;
    }

    while (query.next()) {
        FriendRequest req;
        req.requestId = query.value("request_id").toULongLong();
        req.fromUserId = query.value("from_user_id").toULongLong();
        req.toUserId = query.value("to_user_id").toULongLong();
        req.fromUsername = query.value("from_username").toString();
        req.message = query.value("message").toString();
        int status = query.value("status").toInt();
        req.status = static_cast<RequestStatus>(status);
        requests.append(req);
    }

    return requests;
}

bool ServerDatabase::isFriend(uint64_t userId, uint64_t friendId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "SELECT COUNT(*) FROM friendships "
        "WHERE user_id = :user_id AND friend_id = :friend_id");
    query.bindValue(":user_id", static_cast<qulonglong>(userId));
    query.bindValue(":friend_id", static_cast<qulonglong>(friendId));

    if (!query.exec() || !query.next()) {
        return false;
    }

    return query.value(0).toInt() > 0;
}

bool ServerDatabase::storeOfflineMessage(const ChatMessage& msg) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "INSERT INTO offline_messages (sender_id, receiver_id, content, msg_type, timestamp, created_at) "
        "VALUES (:sender_id, :receiver_id, :content, :msg_type, :timestamp, :created_at)");
    query.bindValue(":sender_id", static_cast<qulonglong>(msg.senderId));
    query.bindValue(":receiver_id", static_cast<qulonglong>(msg.receiverId));
    query.bindValue(":content", msg.content);
    query.bindValue(":msg_type", static_cast<int>(msg.type));
    query.bindValue(":timestamp", msg.timestamp.toSecsSinceEpoch());
    query.bindValue(":created_at", static_cast<qint64>(QDateTime::currentDateTime().toSecsSinceEpoch()));

    if (!query.exec()) {
        qWarning() << "Failed to store offline message:" << query.lastError().text();
        return false;
    }
    return true;
}

QVector<ChatMessage> ServerDatabase::getOfflineMessages(uint64_t userId) {
    QVector<ChatMessage> messages;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "SELECT id, sender_id, receiver_id, content, msg_type, timestamp "
        "FROM offline_messages "
        "WHERE receiver_id = :receiver_id "
        "ORDER BY timestamp ASC");
    query.bindValue(":receiver_id", static_cast<qulonglong>(userId));

    if (!query.exec()) {
        qWarning() << "Failed to get offline messages:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        ChatMessage msg;
        msg.messageId = query.value("id").toULongLong();
        msg.senderId = query.value("sender_id").toULongLong();
        msg.receiverId = query.value("receiver_id").toULongLong();
        msg.content = query.value("content").toString();
        msg.type = static_cast<ChatMessageType>(query.value("msg_type").toInt());
        msg.timestamp = QDateTime::fromSecsSinceEpoch(query.value("timestamp").toLongLong());
        messages.append(msg);
    }

    return messages;
}

bool ServerDatabase::clearOfflineMessages(uint64_t userId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("DELETE FROM offline_messages WHERE receiver_id = :receiver_id");
    query.bindValue(":receiver_id", static_cast<qulonglong>(userId));

    if (!query.exec()) {
        qWarning() << "Failed to clear offline messages:" << query.lastError().text();
        return false;
    }
    return true;
}

bool ServerDatabase::storeChatMessage(uint64_t senderId, uint64_t receiverId,
                                      const QString& content, int msgType, qint64 timestamp) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "INSERT INTO chat_messages (sender_id, receiver_id, content, msg_type, timestamp, created_at) "
        "VALUES (:sender_id, :receiver_id, :content, :msg_type, :timestamp, :created_at)");
    query.bindValue(":sender_id", static_cast<qulonglong>(senderId));
    query.bindValue(":receiver_id", static_cast<qulonglong>(receiverId));
    query.bindValue(":content", content);
    query.bindValue(":msg_type", msgType);
    query.bindValue(":timestamp", timestamp);
    query.bindValue(":created_at", static_cast<qint64>(QDateTime::currentDateTime().toSecsSinceEpoch()));

    if (!query.exec()) {
        qWarning() << "Failed to store chat message:" << query.lastError().text();
        return false;
    }
    return true;
}

QVector<UserInfo> ServerDatabase::getAllUsers(int offset, int limit) {
    QVector<UserInfo> users;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT user_id, username, nickname, avatar, created_at FROM users ORDER BY user_id LIMIT :limit OFFSET :offset");
    query.bindValue(":limit", limit);
    query.bindValue(":offset", offset);
    if (query.exec()) {
        while (query.next()) {
            UserInfo u;
            u.userId = query.value(0).toULongLong();
            u.username = query.value(1).toString();
            u.nickname = query.value(2).toString();
            u.avatar = query.value(3).toString();
            users.append(u);
        }
    }
    return users;
}

int ServerDatabase::getUserCount() {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    if (query.exec("SELECT COUNT(*) FROM users") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

ServerDatabase::ServerStats ServerDatabase::getStatistics() {
    ServerStats stats;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));

    if (query.exec("SELECT COUNT(*) FROM users") && query.next())
        stats.totalUsers = query.value(0).toInt();
    if (query.exec("SELECT COUNT(*) FROM friendships") && query.next())
        stats.totalFriendships = query.value(0).toInt() / 2; // 双向存储
    if (query.exec("SELECT COUNT(*) FROM offline_messages") && query.next())
        stats.totalOfflineMessages = query.value(0).toInt();
    if (query.exec("SELECT COUNT(*) FROM friend_requests WHERE status = 0") && query.next())
        stats.pendingFriendRequests = query.value(0).toInt();
    // Note: totalMessages is set to the same value as totalOfflineMessages because
    // there is no separate message history table. All stored messages are offline messages.
    stats.totalMessages = stats.totalOfflineMessages;

    if (query.exec("SELECT COUNT(*) FROM groups") && query.next())
        stats.totalGroups = query.value(0).toInt();
    if (query.exec("SELECT COUNT(*) FROM group_messages") && query.next())
        stats.totalGroupMessages = query.value(0).toInt();

    return stats;
}

bool ServerDatabase::deleteUser(uint64_t userId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));

    // Delete all related data in order due to foreign keys
    // 0. Remove user from all groups
    if (!removeUserFromAllGroups(userId)) {
        qWarning() << "Failed to remove user from all groups:" << query.lastError().text();
        return false;
    }

    // 1. Delete friendships
    query.prepare("DELETE FROM friendships WHERE user_id = :user_id OR friend_id = :friend_id");
    query.bindValue(":user_id", static_cast<qulonglong>(userId));
    query.bindValue(":friend_id", static_cast<qulonglong>(userId));
    if (!query.exec()) {
        qWarning() << "Failed to delete friendships for user:" << query.lastError().text();
        return false;
    }

    // 2. Delete friend requests (both sent and received)
    query.prepare("DELETE FROM friend_requests WHERE from_user_id = :from_id OR to_user_id = :to_id");
    query.bindValue(":from_id", static_cast<qulonglong>(userId));
    query.bindValue(":to_id", static_cast<qulonglong>(userId));
    if (!query.exec()) {
        qWarning() << "Failed to delete friend requests for user:" << query.lastError().text();
        return false;
    }

    // 3. Delete offline messages (both sent and received)
    query.prepare("DELETE FROM offline_messages WHERE sender_id = :sender_id OR receiver_id = :receiver_id");
    query.bindValue(":sender_id", static_cast<qulonglong>(userId));
    query.bindValue(":receiver_id", static_cast<qulonglong>(userId));
    if (!query.exec()) {
        qWarning() << "Failed to delete offline messages for user:" << query.lastError().text();
        return false;
    }

    // 4. Delete the user
    query.prepare("DELETE FROM users WHERE user_id = :user_id");
    query.bindValue(":user_id", static_cast<qulonglong>(userId));
    if (!query.exec()) {
        qWarning() << "Failed to delete user:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool ServerDatabase::resetUserPassword(uint64_t userId, const QString& newPasswordHash) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("UPDATE users SET password_hash = :password_hash WHERE user_id = :user_id");
    query.bindValue(":password_hash", newPasswordHash);
    query.bindValue(":user_id", static_cast<qulonglong>(userId));

    if (!query.exec()) {
        qWarning() << "Failed to reset user password:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QVector<FriendRequest> ServerDatabase::getAllFriendRequests(int offset, int limit) {
    QVector<FriendRequest> requests;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "SELECT fr.request_id, fr.from_user_id, fr.to_user_id, fr.message, fr.status, fr.created_at, "
               "u1.username AS from_username, u2.username AS to_username "
        "FROM friend_requests fr "
        "LEFT JOIN users u1 ON u1.user_id = fr.from_user_id "
        "LEFT JOIN users u2 ON u2.user_id = fr.to_user_id "
        "ORDER BY fr.created_at DESC LIMIT :limit OFFSET :offset");
    query.bindValue(":limit", limit);
    query.bindValue(":offset", offset);

    if (!query.exec()) {
        qWarning() << "Failed to get all friend requests:" << query.lastError().text();
        return requests;
    }

    while (query.next()) {
        FriendRequest req;
        req.requestId = query.value("request_id").toULongLong();
        req.fromUserId = query.value("from_user_id").toULongLong();
        req.toUserId = query.value("to_user_id").toULongLong();
        req.fromUsername = query.value("from_username").toString();
        req.message = query.value("message").toString();
        int status = query.value("status").toInt();
        req.status = static_cast<RequestStatus>(status);
        req.createdAt = QDateTime::fromSecsSinceEpoch(query.value("created_at").toLongLong());
        requests.append(req);
    }

    return requests;
}

int ServerDatabase::getMessageCount() {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    if (query.exec("SELECT COUNT(*) FROM offline_messages") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QVector<ChatMessage> ServerDatabase::getRecentMessages(int limit) {
    QVector<ChatMessage> messages;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "SELECT id, sender_id, receiver_id, content, msg_type, timestamp "
        "FROM offline_messages "
        "ORDER BY timestamp DESC LIMIT :limit");
    query.bindValue(":limit", limit);

    if (!query.exec()) {
        qWarning() << "Failed to get recent messages:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        ChatMessage msg;
        msg.messageId = query.value("id").toULongLong();
        msg.senderId = query.value("sender_id").toULongLong();
        msg.receiverId = query.value("receiver_id").toULongLong();
        msg.content = query.value("content").toString();
        msg.type = static_cast<ChatMessageType>(query.value("msg_type").toInt());
        msg.timestamp = QDateTime::fromSecsSinceEpoch(query.value("timestamp").toLongLong());
        messages.append(msg);
    }

    return messages;
}

bool ServerDatabase::clearAllOfflineMessages() {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    if (!query.exec("DELETE FROM offline_messages")) {
        qWarning() << "Failed to clear all offline messages:" << query.lastError().text();
        return false;
    }
    return true;
}

bool ServerDatabase::clearAllFriendRequests() {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    if (!query.exec("DELETE FROM friend_requests")) {
        qWarning() << "Failed to clear all friend requests:" << query.lastError().text();
        return false;
    }
    return true;
}

bool ServerDatabase::clearAllMessages() {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    if (!query.exec("DELETE FROM offline_messages")) {
        qWarning() << "Failed to clear all messages:" << query.lastError().text();
        return false;
    }
    return true;
}

bool ServerDatabase::clearDatabase() {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));

    // Delete in order respecting foreign key constraints
    if (!query.exec("DELETE FROM group_messages")) {
        qWarning() << "Failed to clear group_messages:" << query.lastError().text();
        return false;
    }
    if (!query.exec("DELETE FROM chat_messages")) {
        qWarning() << "Failed to clear chat_messages:" << query.lastError().text();
        return false;
    }
    if (!query.exec("DELETE FROM group_members")) {
        qWarning() << "Failed to clear group_members:" << query.lastError().text();
        return false;
    }
    if (!query.exec("DELETE FROM groups WHERE is_default = 0")) {
        qWarning() << "Failed to clear groups (non-default):" << query.lastError().text();
        return false;
    }
    if (!query.exec("DELETE FROM offline_messages")) {
        qWarning() << "Failed to clear offline_messages:" << query.lastError().text();
        return false;
    }
    if (!query.exec("DELETE FROM friend_requests")) {
        qWarning() << "Failed to clear friend_requests:" << query.lastError().text();
        return false;
    }
    if (!query.exec("DELETE FROM friendships")) {
        qWarning() << "Failed to clear friendships:" << query.lastError().text();
        return false;
    }
    if (!query.exec("DELETE FROM users")) {
        qWarning() << "Failed to clear users:" << query.lastError().text();
        return false;
    }

    return true;
}

uint64_t ServerDatabase::getOrCreateBotUser() {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));

    // Check if bot user already exists
    query.prepare("SELECT user_id FROM users WHERE username = '_bot_system_'");
    if (query.exec() && query.next()) {
        return query.value(0).toULongLong();
    }

    // Create bot user
    query.prepare("INSERT INTO users (username, password_hash, nickname, created_at) "
                  "VALUES ('_bot_system_', '', 'Bot', :now)");
    query.bindValue(":now", static_cast<qint64>(QDateTime::currentDateTime().toSecsSinceEpoch()));
    if (query.exec()) {
        return query.lastInsertId().toULongLong();
    }

    qWarning() << "Failed to create bot user:" << query.lastError().text();
    return 0;
}

// ==================== Group Management ====================

uint64_t ServerDatabase::createGroup(const QString& name, uint64_t ownerId, bool isDefault, int maxMembers) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("INSERT INTO groups (name, owner_id, is_default, max_members, created_at, updated_at) "
                  "VALUES (:name, :owner_id, :is_default, :max_members, :created_at, :updated_at)");
    query.bindValue(":name", name);
    query.bindValue(":owner_id", static_cast<qulonglong>(ownerId));
    query.bindValue(":is_default", isDefault ? 1 : 0);
    query.bindValue(":max_members", maxMembers);
    qint64 now = QDateTime::currentSecsSinceEpoch();
    query.bindValue(":created_at", now);
    query.bindValue(":updated_at", now);

    if (!query.exec()) {
        qWarning() << "Failed to create group:" << query.lastError().text();
        return 0;
    }
    return query.lastInsertId().toULongLong();
}

bool ServerDatabase::updateGroup(uint64_t groupId, const QString& name, const QString& avatar, int maxMembers) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));

    if (maxMembers >= 0) {
        query.prepare("UPDATE groups SET name = :name, avatar = :avatar, max_members = :max_members, "
                      "updated_at = :updated_at WHERE group_id = :group_id");
        query.bindValue(":max_members", maxMembers);
    } else {
        query.prepare("UPDATE groups SET name = :name, avatar = :avatar, "
                      "updated_at = :updated_at WHERE group_id = :group_id");
    }
    query.bindValue(":name", name);
    query.bindValue(":avatar", avatar);
    query.bindValue(":updated_at", static_cast<qint64>(QDateTime::currentSecsSinceEpoch()));
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));

    if (!query.exec()) {
        qWarning() << "Failed to update group:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool ServerDatabase::dissolveGroup(uint64_t groupId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));

    // Delete group messages
    query.prepare("DELETE FROM group_messages WHERE group_id = :group_id");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));
    if (!query.exec()) {
        qWarning() << "Failed to delete group messages:" << query.lastError().text();
        return false;
    }

    // Delete group members
    query.prepare("DELETE FROM group_members WHERE group_id = :group_id");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));
    if (!query.exec()) {
        qWarning() << "Failed to delete group members:" << query.lastError().text();
        return false;
    }

    // Delete the group itself (only non-default groups can be dissolved)
    query.prepare("DELETE FROM groups WHERE group_id = :group_id AND is_default = 0");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));
    if (!query.exec()) {
        qWarning() << "Failed to dissolve group:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

ServerDatabase::GroupInfo ServerDatabase::getGroupInfo(uint64_t groupId) {
    GroupInfo info;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT group_id, name, avatar, owner_id, is_default, max_members, created_at FROM groups WHERE group_id = :group_id");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));

    if (!query.exec() || !query.next()) {
        return info;
    }

    info.groupId = query.value("group_id").toULongLong();
    info.name = query.value("name").toString();
    info.avatar = query.value("avatar").toString();
    info.ownerId = query.value("owner_id").toULongLong();
    info.isDefault = query.value("is_default").toInt() != 0;
    info.maxMembers = query.value("max_members").toInt();
    info.createdAt = query.value("created_at").toLongLong();

    // Get member count
    QSqlQuery countQuery(QSqlDatabase::database("chatroom_server"));
    countQuery.prepare("SELECT COUNT(*) FROM group_members WHERE group_id = :group_id");
    countQuery.bindValue(":group_id", static_cast<qulonglong>(groupId));
    if (countQuery.exec() && countQuery.next()) {
        info.memberCount = countQuery.value(0).toInt();
    }

    return info;
}

ServerDatabase::GroupInfo ServerDatabase::getDefaultGroup() {
    GroupInfo info;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.exec("SELECT group_id, name, avatar, owner_id, is_default, max_members, created_at FROM groups WHERE is_default = 1 LIMIT 1");

    if (!query.next()) {
        return info;
    }

    info.groupId = query.value("group_id").toULongLong();
    info.name = query.value("name").toString();
    info.avatar = query.value("avatar").toString();
    info.ownerId = query.value("owner_id").toULongLong();
    info.isDefault = query.value("is_default").toInt() != 0;
    info.maxMembers = query.value("max_members").toInt();
    info.createdAt = query.value("created_at").toLongLong();

    // Get member count
    QSqlQuery countQuery(QSqlDatabase::database("chatroom_server"));
    countQuery.prepare("SELECT COUNT(*) FROM group_members WHERE group_id = :group_id");
    countQuery.bindValue(":group_id", static_cast<qulonglong>(info.groupId));
    if (countQuery.exec() && countQuery.next()) {
        info.memberCount = countQuery.value(0).toInt();
    }

    return info;
}

QVector<ServerDatabase::GroupInfo> ServerDatabase::getUserGroups(uint64_t userId) {
    QVector<GroupInfo> groups;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "SELECT g.group_id, g.name, g.avatar, g.owner_id, g.is_default, g.max_members, g.created_at "
        "FROM groups g "
        "JOIN group_members gm ON gm.group_id = g.group_id "
        "WHERE gm.user_id = :user_id "
        "ORDER BY g.group_id");
    query.bindValue(":user_id", static_cast<qulonglong>(userId));

    if (!query.exec()) {
        qWarning() << "Failed to get user groups:" << query.lastError().text();
        return groups;
    }

    while (query.next()) {
        GroupInfo info;
        info.groupId = query.value("group_id").toULongLong();
        info.name = query.value("name").toString();
        info.avatar = query.value("avatar").toString();
        info.ownerId = query.value("owner_id").toULongLong();
        info.isDefault = query.value("is_default").toInt() != 0;
        info.maxMembers = query.value("max_members").toInt();
        info.createdAt = query.value("created_at").toLongLong();
        groups.append(info);
    }

    return groups;
}

int ServerDatabase::getGroupCount() {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    if (query.exec("SELECT COUNT(*) FROM groups") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

// ==================== Group Member Management ====================

bool ServerDatabase::addGroupMember(uint64_t groupId, uint64_t userId, int role) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("INSERT OR IGNORE INTO group_members (group_id, user_id, role, joined_at) "
                  "VALUES (:group_id, :user_id, :role, :joined_at)");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));
    query.bindValue(":user_id", static_cast<qulonglong>(userId));
    query.bindValue(":role", role);
    query.bindValue(":joined_at", static_cast<qint64>(QDateTime::currentSecsSinceEpoch()));

    if (!query.exec()) {
        qWarning() << "Failed to add group member:" << query.lastError().text();
        return false;
    }
    return true;
}

bool ServerDatabase::addGroupMembers(uint64_t groupId, const QVector<quint64>& userIds) {
    QSqlDatabase::database("chatroom_server").transaction();

    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("INSERT OR IGNORE INTO group_members (group_id, user_id, role, joined_at) "
                  "VALUES (:group_id, :user_id, 0, :joined_at)");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));
    query.bindValue(":joined_at", static_cast<qint64>(QDateTime::currentSecsSinceEpoch()));

    for (uint64_t userId : userIds) {
        query.bindValue(":user_id", static_cast<qulonglong>(userId));
        if (!query.exec()) {
            qWarning() << "Failed to add group member:" << query.lastError().text();
            QSqlDatabase::database("chatroom_server").rollback();
            return false;
        }
    }

    return QSqlDatabase::database("chatroom_server").commit();
}

bool ServerDatabase::removeGroupMember(uint64_t groupId, uint64_t userId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("DELETE FROM group_members WHERE group_id = :group_id AND user_id = :user_id");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));
    query.bindValue(":user_id", static_cast<qulonglong>(userId));

    if (!query.exec()) {
        qWarning() << "Failed to remove group member:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool ServerDatabase::isGroupMember(uint64_t groupId, uint64_t userId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT COUNT(*) FROM group_members WHERE group_id = :group_id AND user_id = :user_id");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));
    query.bindValue(":user_id", static_cast<qulonglong>(userId));

    if (!query.exec() || !query.next()) {
        return false;
    }
    return query.value(0).toInt() > 0;
}

bool ServerDatabase::isGroupOwner(uint64_t groupId, uint64_t userId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT owner_id FROM groups WHERE group_id = :group_id");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));

    if (!query.exec() || !query.next()) {
        return false;
    }
    return query.value("owner_id").toULongLong() == userId;
}

QVector<ServerDatabase::GroupMemberInfo> ServerDatabase::getGroupMembers(uint64_t groupId) {
    QVector<GroupMemberInfo> members;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "SELECT gm.user_id, u.username, u.nickname, u.avatar, gm.role "
        "FROM group_members gm "
        "JOIN users u ON u.user_id = gm.user_id "
        "WHERE gm.group_id = :group_id "
        "ORDER BY gm.role DESC, gm.joined_at ASC");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));

    if (!query.exec()) {
        qWarning() << "Failed to get group members:" << query.lastError().text();
        return members;
    }

    while (query.next()) {
        GroupMemberInfo m;
        m.userId = query.value("user_id").toULongLong();
        m.username = query.value("username").toString();
        m.nickname = query.value("nickname").toString();
        m.avatar = query.value("avatar").toString();
        m.role = query.value("role").toInt();
        members.append(m);
    }

    return members;
}

int ServerDatabase::getGroupMemberCount(uint64_t groupId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT COUNT(*) FROM group_members WHERE group_id = :group_id");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));

    if (!query.exec() || !query.next()) {
        return 0;
    }
    return query.value(0).toInt();
}

QVector<quint64> ServerDatabase::getGroupMemberIds(uint64_t groupId) {
    QVector<quint64> ids;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT user_id FROM group_members WHERE group_id = :group_id");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));

    if (!query.exec()) {
        qWarning() << "Failed to get group member ids:" << query.lastError().text();
        return ids;
    }

    while (query.next()) {
        ids.append(query.value("user_id").toULongLong());
    }

    return ids;
}

// ==================== Group Owner Transfer ====================

bool ServerDatabase::transferGroupOwner(uint64_t groupId, uint64_t newOwnerId) {
    QSqlDatabase::database("chatroom_server").transaction();

    // Get current owner
    QSqlQuery getOwner(QSqlDatabase::database("chatroom_server"));
    getOwner.prepare("SELECT owner_id FROM groups WHERE group_id = :group_id");
    getOwner.bindValue(":group_id", static_cast<qulonglong>(groupId));
    if (!getOwner.exec() || !getOwner.next()) {
        qWarning() << "Failed to get current group owner:" << getOwner.lastError().text();
        QSqlDatabase::database("chatroom_server").rollback();
        return false;
    }
    uint64_t oldOwnerId = getOwner.value("owner_id").toULongLong();

    // Demote old owner to member
    QSqlQuery demote(QSqlDatabase::database("chatroom_server"));
    demote.prepare("UPDATE group_members SET role = 0 WHERE group_id = :group_id AND user_id = :user_id");
    demote.bindValue(":group_id", static_cast<qulonglong>(groupId));
    demote.bindValue(":user_id", static_cast<qulonglong>(oldOwnerId));
    if (!demote.exec()) {
        qWarning() << "Failed to demote old owner:" << demote.lastError().text();
        QSqlDatabase::database("chatroom_server").rollback();
        return false;
    }

    // Promote new owner
    QSqlQuery promote(QSqlDatabase::database("chatroom_server"));
    promote.prepare("UPDATE group_members SET role = 1 WHERE group_id = :group_id AND user_id = :user_id");
    promote.bindValue(":group_id", static_cast<qulonglong>(groupId));
    promote.bindValue(":user_id", static_cast<qulonglong>(newOwnerId));
    if (!promote.exec()) {
        qWarning() << "Failed to promote new owner:" << promote.lastError().text();
        QSqlDatabase::database("chatroom_server").rollback();
        return false;
    }

    // Update group owner
    QSqlQuery updateGroup(QSqlDatabase::database("chatroom_server"));
    updateGroup.prepare("UPDATE groups SET owner_id = :owner_id, updated_at = :updated_at WHERE group_id = :group_id");
    updateGroup.bindValue(":owner_id", static_cast<qulonglong>(newOwnerId));
    updateGroup.bindValue(":updated_at", static_cast<qint64>(QDateTime::currentSecsSinceEpoch()));
    updateGroup.bindValue(":group_id", static_cast<qulonglong>(groupId));
    if (!updateGroup.exec()) {
        qWarning() << "Failed to update group owner:" << updateGroup.lastError().text();
        QSqlDatabase::database("chatroom_server").rollback();
        return false;
    }

    return QSqlDatabase::database("chatroom_server").commit();
}

// ==================== Group Messages ====================

uint64_t ServerDatabase::storeGroupMessage(uint64_t groupId, uint64_t senderId, const QString& content, int msgType, qint64 timestamp) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("INSERT INTO group_messages (group_id, sender_id, content, msg_type, timestamp, created_at) "
                  "VALUES (:group_id, :sender_id, :content, :msg_type, :timestamp, :created_at)");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));
    query.bindValue(":sender_id", static_cast<qulonglong>(senderId));
    query.bindValue(":content", content);
    query.bindValue(":msg_type", msgType);
    query.bindValue(":timestamp", timestamp);
    query.bindValue(":created_at", static_cast<qint64>(QDateTime::currentSecsSinceEpoch()));

    if (!query.exec()) {
        qWarning() << "Failed to store group message:" << query.lastError().text();
        return 0;
    }
    return query.lastInsertId().toULongLong();
}

QVector<ChatMessage> ServerDatabase::getGroupOfflineMessages(uint64_t groupId, uint64_t lastReadMsgId) {
    QVector<ChatMessage> messages;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "SELECT id, group_id, sender_id, content, msg_type, timestamp "
        "FROM group_messages "
        "WHERE group_id = :group_id AND id > :last_read_msg_id "
        "ORDER BY id ASC LIMIT 500");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));
    query.bindValue(":last_read_msg_id", static_cast<qulonglong>(lastReadMsgId));

    if (!query.exec()) {
        qWarning() << "Failed to get group offline messages:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        ChatMessage msg;
        msg.messageId = query.value("id").toULongLong();
        msg.groupId = query.value("group_id").toULongLong();
        msg.senderId = query.value("sender_id").toULongLong();
        msg.content = query.value("content").toString();
        msg.type = static_cast<ChatMessageType>(query.value("msg_type").toInt());
        msg.timestamp = QDateTime::fromSecsSinceEpoch(query.value("timestamp").toLongLong());
        messages.append(msg);
    }

    return messages;
}

bool ServerDatabase::updateLastReadMsgId(uint64_t groupId, uint64_t userId, uint64_t lastReadMsgId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("UPDATE group_members SET last_read_msg_id = :last_read_msg_id "
                  "WHERE group_id = :group_id AND user_id = :user_id");
    query.bindValue(":last_read_msg_id", static_cast<qulonglong>(lastReadMsgId));
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));
    query.bindValue(":user_id", static_cast<qulonglong>(userId));

    if (!query.exec()) {
        qWarning() << "Failed to update last read msg id:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

QVector<ChatMessage> ServerDatabase::getRecentGroupMessages(int limit) {
    QVector<ChatMessage> messages;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "SELECT gm.id, gm.group_id, gm.sender_id, gm.content, gm.msg_type, gm.timestamp, u.nickname "
        "FROM group_messages gm "
        "LEFT JOIN users u ON u.user_id = gm.sender_id "
        "ORDER BY gm.id DESC LIMIT :limit");
    query.bindValue(":limit", limit);

    if (!query.exec()) {
        qWarning() << "Failed to get recent group messages:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        ChatMessage msg;
        msg.messageId = query.value("gm.id").toULongLong();
        msg.groupId = query.value("gm.group_id").toULongLong();
        msg.senderId = query.value("gm.sender_id").toULongLong();
        msg.content = query.value("gm.content").toString();
        msg.type = static_cast<ChatMessageType>(query.value("gm.msg_type").toInt());
        msg.timestamp = QDateTime::fromSecsSinceEpoch(query.value("gm.timestamp").toLongLong());
        msg.senderNickname = query.value("u.nickname").toString();
        messages.append(msg);
    }

    return messages;
}

int ServerDatabase::getGroupMessageCount() {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    if (query.exec("SELECT COUNT(*) FROM group_messages") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

// ==================== Maintenance ====================

bool ServerDatabase::trimOldGroupMessages(uint64_t groupId, int maxCount) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "DELETE FROM group_messages "
        "WHERE group_id = :group_id AND id NOT IN ("
        "  SELECT id FROM group_messages WHERE group_id = :group_id2 ORDER BY id DESC LIMIT :limit"
        ")");
    query.bindValue(":group_id", static_cast<qulonglong>(groupId));
    query.bindValue(":group_id2", static_cast<qulonglong>(groupId));
    query.bindValue(":limit", maxCount);

    if (!query.exec()) {
        qWarning() << "Failed to trim old group messages:" << query.lastError().text();
        return false;
    }
    return true;
}

bool ServerDatabase::removeUserFromAllGroups(uint64_t userId) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("DELETE FROM group_members WHERE user_id = :user_id");
    query.bindValue(":user_id", static_cast<qulonglong>(userId));

    if (!query.exec()) {
        qWarning() << "Failed to remove user from all groups:" << query.lastError().text();
        return false;
    }
    return true;
}

QVector<ServerDatabase::GroupInfo> ServerDatabase::getAllGroups(int offset, int limit) {
    QVector<GroupInfo> groups;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(
        "SELECT g.group_id, g.name, g.avatar, g.owner_id, g.is_default, g.max_members, g.created_at, "
        "       (SELECT COUNT(*) FROM group_members gm WHERE gm.group_id = g.group_id) AS member_count "
        "FROM groups g "
        "ORDER BY g.group_id LIMIT :limit OFFSET :offset");
    query.bindValue(":limit", limit);
    query.bindValue(":offset", offset);

    if (!query.exec()) {
        qWarning() << "Failed to get all groups:" << query.lastError().text();
        return groups;
    }

    while (query.next()) {
        GroupInfo info;
        info.groupId = query.value("group_id").toULongLong();
        info.name = query.value("name").toString();
        info.avatar = query.value("avatar").toString();
        info.ownerId = query.value("owner_id").toULongLong();
        info.isDefault = query.value("is_default").toInt() != 0;
        info.maxMembers = query.value("max_members").toInt();
        info.createdAt = query.value("created_at").toLongLong();
        info.memberCount = query.value("member_count").toInt();
        groups.append(info);
    }

    return groups;
}

// ---------------------------------------------------------------------------
// File metadata
// ---------------------------------------------------------------------------
uint64_t ServerDatabase::saveFileMeta(uint64_t senderId, uint64_t receiverId,
                                       uint64_t groupId, const QString& fileName,
                                       qint64 fileSize, const QString& filePath,
                                       const QString& mimeType, const QString& sha256Hash)
{
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(R"(
        INSERT INTO file_metadata (sender_id, receiver_id, group_id, file_name, file_size,
                                   file_path, mime_type, sha256_hash, created_at)
        VALUES (:sid, :rid, :gid, :fname, :fsize, :fpath, :mime, :hash, :cat)
    )");
    query.bindValue(":sid", static_cast<qulonglong>(senderId));
    query.bindValue(":rid", static_cast<qulonglong>(receiverId));
    query.bindValue(":gid", static_cast<qulonglong>(groupId));
    query.bindValue(":fname", fileName);
    query.bindValue(":fsize", fileSize);
    query.bindValue(":fpath", filePath);
    query.bindValue(":mime", mimeType);
    query.bindValue(":hash", sha256Hash);
    query.bindValue(":cat", static_cast<qint64>(QDateTime::currentSecsSinceEpoch()));

    if (!query.exec()) {
        qWarning() << "Failed to save file meta:" << query.lastError().text();
        return 0;
    }
    return static_cast<uint64_t>(query.lastInsertId().toULongLong());
}

ServerDatabase::FileMeta ServerDatabase::getFileMeta(uint64_t fileId)
{
    FileMeta meta;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT * FROM file_metadata WHERE file_id = :fid");
    query.bindValue(":fid", static_cast<qulonglong>(fileId));

    if (query.exec() && query.next()) {
        meta.fileId = query.value("file_id").toULongLong();
        meta.senderId = query.value("sender_id").toULongLong();
        meta.receiverId = query.value("receiver_id").toULongLong();
        meta.groupId = query.value("group_id").toULongLong();
        meta.fileName = query.value("file_name").toString();
        meta.fileSize = query.value("file_size").toLongLong();
        meta.filePath = query.value("file_path").toString();
        meta.mimeType = query.value("mime_type").toString();
        meta.sha256Hash = query.value("sha256_hash").toString();
        meta.createdAt = query.value("created_at").toLongLong();
    }
    return meta;
}

// ---------------------------------------------------------------------------
// Group encryption keys
// ---------------------------------------------------------------------------
bool ServerDatabase::saveGroupEncryptedKey(uint64_t groupId, uint64_t userId,
                                            const QString& encryptedKey)
{
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(R"(
        INSERT OR REPLACE INTO group_encryption_keys (group_id, user_id, encrypted_key, created_at)
        VALUES (:gid, :uid, :key, :cat)
    )");
    query.bindValue(":gid", static_cast<qulonglong>(groupId));
    query.bindValue(":uid", static_cast<qulonglong>(userId));
    query.bindValue(":key", encryptedKey);
    query.bindValue(":cat", static_cast<qint64>(QDateTime::currentSecsSinceEpoch()));
    return query.exec();
}

QString ServerDatabase::getGroupEncryptedKey(uint64_t groupId, uint64_t userId)
{
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT encrypted_key FROM group_encryption_keys "
                  "WHERE group_id = :gid AND user_id = :uid");
    query.bindValue(":gid", static_cast<qulonglong>(groupId));
    query.bindValue(":uid", static_cast<qulonglong>(userId));
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return {};
}

bool ServerDatabase::removeGroupEncryptedKey(uint64_t groupId, uint64_t userId)
{
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("DELETE FROM group_encryption_keys WHERE group_id = :gid AND user_id = :uid");
    query.bindValue(":gid", static_cast<qulonglong>(groupId));
    query.bindValue(":uid", static_cast<qulonglong>(userId));
    return query.exec();
}

// ---------------------------------------------------------------------------
// Group announcements
// ---------------------------------------------------------------------------
bool ServerDatabase::setAnnouncement(uint64_t groupId, uint64_t senderId, const QString& content)
{
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(R"(
        INSERT INTO group_announcements (group_id, sender_id, content, created_at)
        VALUES (:gid, :sid, :content, :cat)
    )");
    query.bindValue(":gid", static_cast<qulonglong>(groupId));
    query.bindValue(":sid", static_cast<qulonglong>(senderId));
    query.bindValue(":content", content);
    query.bindValue(":cat", static_cast<qint64>(QDateTime::currentSecsSinceEpoch()));

    if (!query.exec()) {
        qWarning() << "Failed to set announcement:" << query.lastError().text();
        return false;
    }
    return true;
}

QVector<QPair<QString, qint64>> ServerDatabase::getAnnouncements(uint64_t groupId, int limit)
{
    QVector<QPair<QString, qint64>> result;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT content, created_at FROM group_announcements "
                  "WHERE group_id = :gid ORDER BY created_at DESC LIMIT :lim");
    query.bindValue(":gid", static_cast<qulonglong>(groupId));
    query.bindValue(":lim", limit);

    if (query.exec()) {
        while (query.next()) {
            result.append({query.value(0).toString(), query.value(1).toLongLong()});
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Group message read status
// ---------------------------------------------------------------------------
bool ServerDatabase::markGroupMessageRead(uint64_t msgId, uint64_t groupId, uint64_t userId)
{
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare(R"(
        INSERT OR IGNORE INTO group_message_reads (msg_id, group_id, user_id, read_at)
        VALUES (:mid, :gid, :uid, :rat)
    )");
    query.bindValue(":mid", static_cast<qulonglong>(msgId));
    query.bindValue(":gid", static_cast<qulonglong>(groupId));
    query.bindValue(":uid", static_cast<qulonglong>(userId));
    query.bindValue(":rat", static_cast<qint64>(QDateTime::currentSecsSinceEpoch()));

    return query.exec();
}

int ServerDatabase::getGroupMessageReadCount(uint64_t msgId, uint64_t groupId)
{
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT COUNT(*) FROM group_message_reads WHERE msg_id = :mid AND group_id = :gid");
    query.bindValue(":mid", static_cast<qulonglong>(msgId));
    query.bindValue(":gid", static_cast<qulonglong>(groupId));

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Message search (FTS5 with LIKE fallback)
// ---------------------------------------------------------------------------
QVector<ServerDatabase::SearchResult> ServerDatabase::searchMessages(
    uint64_t userId, const QString& keyword, int limit, int offset)
{
    QVector<SearchResult> results;
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    QString likePattern = "%" + keyword + "%";

    // Private messages: search chat_messages (persistent) + offline_messages (fallback)
    query.prepare(R"(
        SELECT m.id, m.sender_id, u.nickname, m.content, m.timestamp
        FROM chat_messages m
        JOIN users u ON u.user_id = m.sender_id
        WHERE (m.sender_id = :uid OR m.receiver_id = :uid2)
          AND m.content LIKE :kw
        ORDER BY m.timestamp DESC
        LIMIT :lim OFFSET :off
    )");
    query.bindValue(":uid", static_cast<qulonglong>(userId));
    query.bindValue(":uid2", static_cast<qulonglong>(userId));
    query.bindValue(":kw", likePattern);
    query.bindValue(":lim", limit);
    query.bindValue(":off", offset);

    if (query.exec()) {
        while (query.next()) {
            SearchResult r;
            r.messageId = query.value(0).toULongLong();
            r.senderId = query.value(1).toULongLong();
            r.senderName = query.value(2).toString();
            r.content = query.value(3).toString();
            r.timestamp = query.value(4).toLongLong();
            r.isGroup = false;
            // Build snippet: extract 40 chars around match
            int idx = r.content.indexOf(keyword, 0, Qt::CaseInsensitive);
            if (idx >= 0) {
                int start = qMax(0, idx - 20);
                int len = qMin(keyword.length() + 40, r.content.length() - start);
                r.snippet = (start > 0 ? "..." : "") +
                            r.content.mid(start, len) +
                            (start + len < r.content.length() ? "..." : "");
            } else {
                r.snippet = r.content.left(60);
            }
            results.append(r);
        }
    }

    // Group messages
    QSqlQuery gQuery(QSqlDatabase::database("chatroom_server"));
    gQuery.prepare(R"(
        SELECT gm.id, gm.sender_id, u.nickname, gm.content, gm.timestamp, g.group_id, g.name
        FROM group_messages gm
        JOIN users u ON u.user_id = gm.sender_id
        JOIN group_members gmem ON gmem.group_id = gm.group_id AND gmem.user_id = :uid
        JOIN groups g ON g.group_id = gm.group_id
        WHERE gm.content LIKE :kw
        ORDER BY gm.timestamp DESC
        LIMIT :lim OFFSET :off
    )");
    gQuery.bindValue(":uid", static_cast<qulonglong>(userId));
    gQuery.bindValue(":kw", likePattern);
    gQuery.bindValue(":lim", limit);
    gQuery.bindValue(":off", offset);

    if (gQuery.exec()) {
        while (gQuery.next()) {
            SearchResult r;
            r.messageId = gQuery.value(0).toULongLong();
            r.senderId = gQuery.value(1).toULongLong();
            r.senderName = gQuery.value(2).toString();
            r.content = gQuery.value(3).toString();
            r.timestamp = gQuery.value(4).toLongLong();
            r.isGroup = true;
            r.groupId = gQuery.value(5).toULongLong();
            r.groupName = gQuery.value(6).toString();
            int idx = r.content.indexOf(keyword, 0, Qt::CaseInsensitive);
            if (idx >= 0) {
                int start = qMax(0, idx - 20);
                int len = qMin(keyword.length() + 40, r.content.length() - start);
                r.snippet = (start > 0 ? "..." : "") +
                            r.content.mid(start, len) +
                            (start + len < r.content.length() ? "..." : "");
            } else {
                r.snippet = r.content.left(60);
            }
            results.append(r);
        }
    }

    // Sort combined results by timestamp descending
    std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
        return a.timestamp > b.timestamp;
    });

    return results;
}

bool ServerDatabase::saveSetting(const QString& key, const QString& value) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("INSERT OR REPLACE INTO server_settings (key, value) VALUES (:k, :v)");
    query.bindValue(":k", key); query.bindValue(":v", value);
    return query.exec();
}

QString ServerDatabase::loadSetting(const QString& key, const QString& defaultValue) {
    QSqlQuery query(QSqlDatabase::database("chatroom_server"));
    query.prepare("SELECT value FROM server_settings WHERE key = :k");
    query.bindValue(":k", key);
    if (query.exec() && query.next()) return query.value(0).toString();
    return defaultValue;
}

} // namespace chatroom::server
