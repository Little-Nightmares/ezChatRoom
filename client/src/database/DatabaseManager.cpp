#include "DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QDateTime>

namespace chatroom::client {

DatabaseManager::DatabaseManager()
{
    m_connectionName = "chatroom_client_" + QString::number(reinterpret_cast<quintptr>(this));
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    // Must clear the member reference before removing the connection,
    // otherwise QSqlDatabase's copy-on-write may re-open the connection.
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool DatabaseManager::initialize(const QString& dbPath)
{
    // Ensure parent directory exists
    QFileInfo fi(dbPath);
    QDir dir = fi.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qCritical() << "DatabaseManager: failed to open database:" << m_db.lastError().text();
        return false;
    }

    // Enable WAL mode for better concurrent read performance
    QSqlQuery query(m_db);
    if (!query.exec("PRAGMA journal_mode = WAL;")) {
        qWarning() << "DatabaseManager: failed to set WAL mode:" << query.lastError().text();
    }
    if (!query.exec("PRAGMA synchronous = NORMAL;")) {
        qWarning() << "DatabaseManager: failed to set synchronous:" << query.lastError().text();
    }

    createTables();
    return true;
}

void DatabaseManager::createTables()
{
    QSqlQuery query(m_db);

    // Messages table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS messages (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            sender_id       INTEGER NOT NULL,
            receiver_id     INTEGER NOT NULL,
            content         TEXT    NOT NULL,
            msg_type        INTEGER NOT NULL DEFAULT 0,
            timestamp       INTEGER NOT NULL,
            is_read         INTEGER NOT NULL DEFAULT 0,
            is_mine         INTEGER NOT NULL DEFAULT 0,
            group_id        INTEGER NOT NULL DEFAULT 0,
            sender_nickname TEXT    NOT NULL DEFAULT ''
        );
    )");

    // Index for conversation queries
    query.exec(R"(
        CREATE INDEX IF NOT EXISTS idx_messages_conv ON messages(
            CASE WHEN is_mine = 1 THEN receiver_id ELSE sender_id END,
            timestamp DESC
        );
    )");

    // Migration: add group_id and sender_nickname columns if missing (existing databases)
    query.exec("ALTER TABLE messages ADD COLUMN group_id INTEGER NOT NULL DEFAULT 0");
    query.exec("ALTER TABLE messages ADD COLUMN sender_nickname TEXT NOT NULL DEFAULT ''");

    // Contacts table (local cache)
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS contacts (
            user_id     INTEGER PRIMARY KEY,
            username    TEXT    NOT NULL,
            nickname    TEXT    NOT NULL DEFAULT '',
            avatar      TEXT    NOT NULL DEFAULT ''
        );
    )");

    // Key pairs table (single row for local RSA key pair)
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS key_pairs (
            id          INTEGER PRIMARY KEY CHECK (id = 1),
            public_key  BLOB    NOT NULL,
            private_key BLOB    NOT NULL
        );
    )");

    // Session keys table (per-friend AES keys)
    // NOTE: DEFAULT 0 instead of DEFAULT (unixepoch()) to avoid SQLite driver issues
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS session_keys (
            friend_id  INTEGER PRIMARY KEY,
            aes_key    BLOB    NOT NULL,
            created_at INTEGER NOT NULL DEFAULT 0
        );
    )");

    // Groups cache table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS groups_cache (
            group_id        INTEGER PRIMARY KEY,
            name            TEXT    NOT NULL DEFAULT '',
            avatar          TEXT    NOT NULL DEFAULT '',
            owner_id        INTEGER NOT NULL DEFAULT 0,
            member_count    INTEGER NOT NULL DEFAULT 0,
            is_default      INTEGER NOT NULL DEFAULT 0,
            last_read_msg_id INTEGER NOT NULL DEFAULT 0
        );
    )");

    // Group members cache table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS group_members_cache (
            group_id  INTEGER NOT NULL,
            user_id   INTEGER NOT NULL,
            nickname  TEXT    NOT NULL DEFAULT '',
            avatar    TEXT    NOT NULL DEFAULT '',
            role      INTEGER NOT NULL DEFAULT 0,
            PRIMARY KEY (group_id, user_id)
        );
    )");

    // Migration: add group_id column to messages table if missing
    QSqlQuery checkCol(m_db);
    checkCol.exec("PRAGMA table_info(messages)");
    bool hasGroupId = false;
    while (checkCol.next()) {
        if (checkCol.value(1).toString() == "group_id") {
            hasGroupId = true;
            break;
        }
    }
    if (!hasGroupId) {
        m_db.exec("ALTER TABLE messages ADD COLUMN group_id INTEGER NOT NULL DEFAULT 0");
    }

    // Migration: add sender_nickname column to messages table if missing
    bool hasSenderNick = false;
    checkCol.exec("PRAGMA table_info(messages)");
    while (checkCol.next()) {
        if (checkCol.value(1).toString() == "sender_nickname") {
            hasSenderNick = true;
            break;
        }
    }
    if (!hasSenderNick) {
        m_db.exec("ALTER TABLE messages ADD COLUMN sender_nickname TEXT NOT NULL DEFAULT ''");
    }


    qDebug() << "DatabaseManager: tables created/verified";
}

qint64 DatabaseManager::saveMessage(const chatroom::models::ChatMessage& msg)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "INSERT INTO messages (sender_id, receiver_id, content, msg_type, timestamp, is_read, is_mine) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(static_cast<qint64>(msg.senderId));
    query.addBindValue(static_cast<qint64>(msg.receiverId));
    query.addBindValue(msg.content);
    query.addBindValue(static_cast<int>(msg.type));
    query.addBindValue(msg.timestamp.toSecsSinceEpoch());
    query.addBindValue(msg.isRead ? 1 : 0);
    query.addBindValue(msg.isMine ? 1 : 0);

    if (!query.exec()) {
        qWarning() << "DatabaseManager::saveMessage failed:" << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toLongLong();
}

QVector<chatroom::models::ChatMessage> DatabaseManager::getMessages(uint64_t friendId,
                                                                     int limit, int offset)
{
    QVector<chatroom::models::ChatMessage> messages;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT id, sender_id, receiver_id, content, msg_type, timestamp, is_read, is_mine "
        "FROM messages "
        "WHERE (sender_id = ? OR receiver_id = ?) "
        "ORDER BY timestamp ASC "
        "LIMIT ? OFFSET ?");
    query.addBindValue(static_cast<qint64>(friendId));
    query.addBindValue(static_cast<qint64>(friendId));
    query.addBindValue(limit);
    query.addBindValue(offset);

    if (!query.exec()) {
        qWarning() << "DatabaseManager::getMessages failed:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        chatroom::models::ChatMessage msg;
        msg.messageId = static_cast<uint64_t>(query.value(0).toULongLong());
        msg.senderId = static_cast<uint64_t>(query.value(1).toLongLong());
        msg.receiverId = static_cast<uint64_t>(query.value(2).toLongLong());
        msg.content = query.value(3).toString();
        msg.type = static_cast<chatroom::models::ChatMessageType>(query.value(4).toInt());
        msg.timestamp = QDateTime::fromSecsSinceEpoch(query.value(5).toLongLong());
        msg.isRead = query.value(6).toBool();
        msg.isMine = query.value(7).toBool();
        messages.append(msg);
    }

    return messages;
}

int DatabaseManager::getUnreadCount(uint64_t friendId)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT COUNT(*) FROM messages "
        "WHERE sender_id = ? AND is_read = 0 AND is_mine = 0");
    query.addBindValue(static_cast<qint64>(friendId));

    if (!query.exec() || !query.next()) {
        qWarning() << "DatabaseManager::getUnreadCount failed:" << query.lastError().text();
        return 0;
    }

    return query.value(0).toInt();
}

bool DatabaseManager::markAsRead(uint64_t friendId)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "UPDATE messages SET is_read = 1 "
        "WHERE sender_id = ? AND is_read = 0 AND is_mine = 0");
    query.addBindValue(static_cast<qint64>(friendId));

    if (!query.exec()) {
        qWarning() << "DatabaseManager::markAsRead failed:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::clearHistory(uint64_t friendId)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "DELETE FROM messages "
        "WHERE sender_id = ? OR receiver_id = ?");
    query.addBindValue(static_cast<qint64>(friendId));
    query.addBindValue(static_cast<qint64>(friendId));

    if (!query.exec()) {
        qWarning() << "DatabaseManager::clearHistory failed:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::clearAllMessages()
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec("DELETE FROM messages")) {
        qWarning() << "DatabaseManager::clearAllMessages failed:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateMessageContent(qint64 messageId, const QString& newContent)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare("UPDATE messages SET content = ? WHERE id = ?");
    query.addBindValue(newContent);
    query.addBindValue(messageId);
    return query.exec();
}

QVector<chatroom::models::ChatMessage> DatabaseManager::searchMessages(quint64 friendId, const QString& keyword, int limit)
{
    QVector<chatroom::models::ChatMessage> messages;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare("SELECT id, sender_id, receiver_id, content, timestamp, is_read, is_mine, msg_type "
                   "FROM messages WHERE (sender_id = ? OR receiver_id = ?) AND content LIKE ? "
                   "ORDER BY timestamp DESC LIMIT ?");
    query.addBindValue(static_cast<qint64>(friendId));
    query.addBindValue(static_cast<qint64>(friendId));
    query.addBindValue("%" + keyword + "%");
    query.addBindValue(limit);
    if (query.exec()) {
        while (query.next()) {
            chatroom::models::ChatMessage msg;
            msg.messageId = query.value(0).toULongLong();
            msg.senderId = query.value(1).toULongLong();
            msg.receiverId = query.value(2).toULongLong();
            msg.content = query.value(3).toString();
            msg.timestamp = QDateTime::fromSecsSinceEpoch(query.value(4).toLongLong());
            msg.isRead = query.value(5).toBool();
            msg.isMine = query.value(6).toBool();
            msg.type = static_cast<chatroom::models::ChatMessageType>(query.value(7).toInt());
            messages.append(msg);
        }
    }
    return messages;
}

QVector<DatabaseManager::ConversationInfo> DatabaseManager::getConversationList()
{
    QVector<ConversationInfo> conversations;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    // Get the latest message for each conversation partner using subquery
    QString sql = R"(
        SELECT
            m.id,
            CASE WHEN m.is_mine = 1 THEN m.receiver_id ELSE m.sender_id END AS friend_id,
            m.content AS last_message,
            m.timestamp AS last_time,
            (SELECT COUNT(*) FROM messages m2
             WHERE m2.sender_id = CASE WHEN m.is_mine = 1 THEN m.receiver_id ELSE m.sender_id END
             AND m2.is_read = 0 AND m2.is_mine = 0) AS unread_count
        FROM messages m
        INNER JOIN (
            SELECT MAX(id) as max_id FROM messages
            GROUP BY CASE WHEN is_mine = 1 THEN receiver_id ELSE sender_id END
        ) latest ON m.id = latest.max_id
        ORDER BY m.timestamp DESC
    )";

    if (!query.exec(sql)) {
        qWarning() << "DatabaseManager::getConversationList failed:" << query.lastError().text();
        return conversations;
    }

    while (query.next()) {
        ConversationInfo info;
        info.friendId = static_cast<uint64_t>(query.value(1).toLongLong());
        info.lastMessage = query.value(2).toString();
        info.lastTime = QDateTime::fromSecsSinceEpoch(query.value(3).toLongLong());
        info.unreadCount = query.value(4).toInt();

        // Look up nickname and avatar from contacts cache
        chatroom::models::UserInfo user = getCachedUser(info.friendId);
        info.nickname = user.nickname.isEmpty() ? user.username : user.nickname;
        info.avatar = user.avatar;

        conversations.append(info);
    }

    return conversations;
}

bool DatabaseManager::upsertUser(const chatroom::models::UserInfo& user)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "INSERT INTO contacts (user_id, username, nickname, avatar) "
        "VALUES (?, ?, ?, ?) "
        "ON CONFLICT(user_id) DO UPDATE SET "
        "username = excluded.username, "
        "nickname = excluded.nickname, "
        "avatar = excluded.avatar");
    query.addBindValue(static_cast<qint64>(user.userId));
    query.addBindValue(user.username);
    query.addBindValue(user.nickname);
    query.addBindValue(user.avatar);

    if (!query.exec()) {
        qWarning() << "DatabaseManager::upsertUser failed:" << query.lastError().text();
        return false;
    }
    return true;
}

chatroom::models::UserInfo DatabaseManager::getCachedUser(uint64_t userId)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare("SELECT user_id, username, nickname, avatar FROM contacts WHERE user_id = ?");
    query.addBindValue(static_cast<qint64>(userId));

    chatroom::models::UserInfo user;
    if (query.exec() && query.next()) {
        user.userId = static_cast<uint64_t>(query.value(0).toLongLong());
        user.username = query.value(1).toString();
        user.nickname = query.value(2).toString();
        user.avatar = query.value(3).toString();
    }
    return user;
}

bool DatabaseManager::saveKeyPair(const QByteArray& publicKey, const QByteArray& privateKey)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "INSERT INTO key_pairs (id, public_key, private_key) "
        "VALUES (1, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "public_key = excluded.public_key, "
        "private_key = excluded.private_key");
    query.addBindValue(publicKey);
    query.addBindValue(privateKey);

    if (!query.exec()) {
        qWarning() << "DatabaseManager::saveKeyPair failed:" << query.lastError().text();
        return false;
    }
    return true;
}

std::pair<QByteArray, QByteArray> DatabaseManager::loadKeyPair()
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.exec("SELECT public_key, private_key FROM key_pairs WHERE id = 1");

    if (query.next()) {
        return {query.value(0).toByteArray(), query.value(1).toByteArray()};
    }
    return {QByteArray(), QByteArray()};
}

bool DatabaseManager::saveSessionKey(uint64_t friendId, const QByteArray& key)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    qint64 now = QDateTime::currentDateTime().toSecsSinceEpoch();
    query.prepare(
        "INSERT INTO session_keys (friend_id, aes_key, created_at) "
        "VALUES (?, ?, ?) "
        "ON CONFLICT(friend_id) DO UPDATE SET aes_key = excluded.aes_key");
    query.addBindValue(static_cast<qint64>(friendId));
    query.addBindValue(key);
    query.addBindValue(now);

    if (!query.exec()) {
        qWarning() << "DatabaseManager::saveSessionKey failed:" << query.lastError().text();
        return false;
    }
    return true;
}

QByteArray DatabaseManager::loadSessionKey(uint64_t friendId)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare("SELECT aes_key FROM session_keys WHERE friend_id = ?");
    query.addBindValue(static_cast<qint64>(friendId));

    if (query.exec() && query.next()) {
        return query.value(0).toByteArray();
    }
    return QByteArray();
}

// ==================== Group message storage ====================

qint64 DatabaseManager::saveGroupMessage(const chatroom::models::ChatMessage& msg)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "INSERT INTO messages (sender_id, receiver_id, content, msg_type, timestamp, "
        "is_read, is_mine, group_id, sender_nickname) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(static_cast<qint64>(msg.senderId));
    query.addBindValue(static_cast<qint64>(msg.receiverId));
    query.addBindValue(msg.content);
    query.addBindValue(static_cast<int>(msg.type));
    query.addBindValue(msg.timestamp.toSecsSinceEpoch());
    query.addBindValue(msg.isRead ? 1 : 0);
    query.addBindValue(msg.isMine ? 1 : 0);
    query.addBindValue(static_cast<qint64>(msg.groupId));
    query.addBindValue(msg.senderNickname);

    if (!query.exec()) {
        qWarning() << "DatabaseManager::saveGroupMessage failed:" << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toLongLong();
}

QVector<chatroom::models::ChatMessage> DatabaseManager::getGroupMessages(uint64_t groupId,
                                                                          int limit, int offset)
{
    QVector<chatroom::models::ChatMessage> messages;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT id, sender_id, receiver_id, content, msg_type, timestamp, "
        "is_read, is_mine, group_id, sender_nickname "
        "FROM messages "
        "WHERE group_id = ? "
        "ORDER BY timestamp ASC "
        "LIMIT ? OFFSET ?");
    query.addBindValue(static_cast<qint64>(groupId));
    query.addBindValue(limit);
    query.addBindValue(offset);

    if (!query.exec()) {
        qWarning() << "DatabaseManager::getGroupMessages failed:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        chatroom::models::ChatMessage msg;
        msg.messageId      = static_cast<uint64_t>(query.value(0).toULongLong());
        msg.senderId       = static_cast<uint64_t>(query.value(1).toLongLong());
        msg.receiverId     = static_cast<uint64_t>(query.value(2).toLongLong());
        msg.content        = query.value(3).toString();
        msg.type           = static_cast<chatroom::models::ChatMessageType>(query.value(4).toInt());
        msg.timestamp      = QDateTime::fromSecsSinceEpoch(query.value(5).toLongLong());
        msg.isRead         = query.value(6).toBool();
        msg.isMine         = query.value(7).toBool();
        msg.groupId        = static_cast<uint64_t>(query.value(8).toLongLong());
        msg.senderNickname = query.value(9).toString();
        messages.append(msg);
    }

    return messages;
}

int DatabaseManager::getGroupUnreadCount(uint64_t groupId)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT COUNT(*) FROM messages "
        "WHERE group_id = ? AND is_read = 0 AND is_mine = 0");
    query.addBindValue(static_cast<qint64>(groupId));

    if (!query.exec() || !query.next()) {
        qWarning() << "DatabaseManager::getGroupUnreadCount failed:" << query.lastError().text();
        return 0;
    }

    return query.value(0).toInt();
}

bool DatabaseManager::markGroupAsRead(uint64_t groupId)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.transaction();

    // Mark all unread non-self messages in this group as read
    QSqlQuery updateMsg(db);
    updateMsg.prepare(
        "UPDATE messages SET is_read = 1 "
        "WHERE group_id = ? AND is_read = 0 AND is_mine = 0");
    updateMsg.addBindValue(static_cast<qint64>(groupId));

    if (!updateMsg.exec()) {
        qWarning() << "DatabaseManager::markGroupAsRead (messages) failed:" << updateMsg.lastError().text();
        db.rollback();
        return false;
    }

    // Update last_read_msg_id in groups_cache to the latest message id in this group
    QSqlQuery updateCache(db);
    updateCache.prepare(
        "UPDATE groups_cache SET last_read_msg_id = "
        "(SELECT MAX(id) FROM messages WHERE group_id = ?) "
        "WHERE group_id = ?");
    updateCache.addBindValue(static_cast<qint64>(groupId));
    updateCache.addBindValue(static_cast<qint64>(groupId));

    if (!updateCache.exec()) {
        qWarning() << "DatabaseManager::markGroupAsRead (cache) failed:" << updateCache.lastError().text();
        db.rollback();
        return false;
    }

    db.commit();
    return true;
}

// ==================== Group cache ====================

bool DatabaseManager::upsertGroupCache(uint64_t groupId, const QString& name, const QString& avatar,
                                        uint64_t ownerId, int memberCount, bool isDefault,
                                        uint64_t lastReadMsgId)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "INSERT OR REPLACE INTO groups_cache "
        "(group_id, name, avatar, owner_id, member_count, is_default, last_read_msg_id) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(static_cast<qint64>(groupId));
    query.addBindValue(name);
    query.addBindValue(avatar);
    query.addBindValue(static_cast<qint64>(ownerId));
    query.addBindValue(memberCount);
    query.addBindValue(isDefault ? 1 : 0);
    query.addBindValue(static_cast<qint64>(lastReadMsgId));

    if (!query.exec()) {
        qWarning() << "DatabaseManager::upsertGroupCache failed:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::removeGroupCache(uint64_t groupId)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare("DELETE FROM groups_cache WHERE group_id = ?");
    query.addBindValue(static_cast<qint64>(groupId));

    if (!query.exec()) {
        qWarning() << "DatabaseManager::removeGroupCache failed:" << query.lastError().text();
        return false;
    }
    return true;
}

// ==================== Group members cache ====================

bool DatabaseManager::upsertGroupMembersCache(uint64_t groupId, const QVector<QVariantMap>& members)
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.transaction();

    QSqlQuery query(db);
    query.prepare(
        "INSERT OR REPLACE INTO group_members_cache "
        "(group_id, user_id, nickname, avatar, role) "
        "VALUES (?, ?, ?, ?, ?)");

    for (const auto& member : members) {
        query.addBindValue(static_cast<qint64>(groupId));
        query.addBindValue(static_cast<qint64>(member.value("user_id").toULongLong()));
        query.addBindValue(member.value("nickname").toString());
        query.addBindValue(member.value("avatar").toString());
        query.addBindValue(member.value("role").toInt());

        if (!query.exec()) {
            qWarning() << "DatabaseManager::upsertGroupMembersCache failed:" << query.lastError().text();
            db.rollback();
            return false;
        }
    }

    db.commit();
    return true;
}

QVector<QVariantMap> DatabaseManager::getCachedGroupMembers(uint64_t groupId)
{
    QVector<QVariantMap> members;

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(
        "SELECT user_id, nickname, avatar, role "
        "FROM group_members_cache "
        "WHERE group_id = ?");
    query.addBindValue(static_cast<qint64>(groupId));

    if (!query.exec()) {
        qWarning() << "DatabaseManager::getCachedGroupMembers failed:" << query.lastError().text();
        return members;
    }

    while (query.next()) {
        QVariantMap member;
        member["user_id"] = static_cast<quint64>(query.value(0).toULongLong());
        member["nickname"] = query.value(1).toString();
        member["avatar"]   = query.value(2).toString();
        member["role"]     = query.value(3).toInt();
        members.append(member);
    }

    return members;
}

bool DatabaseManager::removeGroupMembersCache(uint64_t groupId)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare("DELETE FROM group_members_cache WHERE group_id = ?");
    query.addBindValue(static_cast<qint64>(groupId));

    if (!query.exec()) {
        qWarning() << "DatabaseManager::removeGroupMembersCache failed:" << query.lastError().text();
        return false;
    }
    return true;
}

void DatabaseManager::saveSetting(const QString& key, const QString& value) {
    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO user_settings (key, value) VALUES (:k, :v)");
    query.bindValue(":k", key); query.bindValue(":v", value);
    query.exec();
}
QString DatabaseManager::loadSetting(const QString& key, const QString& defaultValue) {
    QSqlQuery query(m_db);
    query.prepare("SELECT value FROM user_settings WHERE key = :k");
    query.bindValue(":k", key);
    if (query.exec() && query.next()) return query.value(0).toString();
    return defaultValue;
}

} // namespace chatroom::client

