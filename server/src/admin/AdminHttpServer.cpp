#include "admin/AdminHttpServer.h"
#include "core/TcpServer.h"
#include "core/ClientSession.h"
#include "database/ServerDatabase.h"
#include "protocol/ChatProtocol.h"
#include "protocol/MessageTypes.h"

#include <QCoreApplication>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrlQuery>
#include <QDateTime>
#include <QFile>
#include <QRandomGenerator>
#include <QTimer>
#include <QDebug>

using chatroom::protocol::MessageType;
using chatroom::protocol::ChatProtocol;

namespace chatroom::server {

AdminHttpServer::AdminHttpServer(ServerDatabase* db, TcpServer* tcpServer, QObject* parent)
    : QObject(parent)
    , m_db(db)
    , m_tcpServer(tcpServer)
{
    m_statsCacheTimer.start();
    m_startTime = QDateTime::currentDateTime();
}

AdminHttpServer::~AdminHttpServer() {
    stop();
}

bool AdminHttpServer::start(quint16 port, const QString& adminToken) {
    m_adminToken = adminToken;

    if (m_httpServer) {
        stop();
    }

    m_httpServer = new QTcpServer(this);
    if (!m_httpServer->listen(QHostAddress::Any, port)) {
        qCritical() << "Admin HTTP server failed to start on port" << port
                    << ":" << m_httpServer->errorString();
        delete m_httpServer;
        m_httpServer = nullptr;
        return false;
    }

    connect(m_httpServer, &QTcpServer::newConnection, this, &AdminHttpServer::onHttpConnection);

    // SSE heartbeat timer: send keepalive comment every 30 seconds
    m_sseHeartbeatTimer = new QTimer(this);
    m_sseHeartbeatTimer->setInterval(30000);
    connect(m_sseHeartbeatTimer, &QTimer::timeout, this, [this]() {
        QByteArray heartbeat = ": heartbeat\n\n";
        for (QTcpSocket* sock : m_sseClients) {
            if (sock->state() == QAbstractSocket::ConnectedState) {
                sock->write(heartbeat);
                sock->flush();
            }
        }
    });
    m_sseHeartbeatTimer->start();

    qDebug() << "Admin HTTP server started on port" << port;
    return true;
}

void AdminHttpServer::stop() {
    if (m_sseHeartbeatTimer) {
        m_sseHeartbeatTimer->stop();
        delete m_sseHeartbeatTimer;
        m_sseHeartbeatTimer = nullptr;
    }

    for (QTcpSocket* sock : m_sseClients) {
        sock->disconnectFromHost();
    }
    m_sseClients.clear();

    if (m_httpServer) {
        m_httpServer->close();
        delete m_httpServer;
        m_httpServer = nullptr;
    }
}

void AdminHttpServer::broadcastSSEEvent(const QString& event, const QJsonObject& data) {
    QJsonObject payload;
    payload["event"] = event;
    payload["data"] = data;
    QByteArray msg = "event: " + event.toUtf8() + "\ndata: "
                   + QJsonDocument(payload).toJson(QJsonDocument::Compact) + "\n\n";

    for (QTcpSocket* sock : m_sseClients) {
        if (sock->state() == QAbstractSocket::ConnectedState) {
            sock->write(msg);
            sock->flush();
        }
    }
}

void AdminHttpServer::invalidateStatsCache() {
    m_statsCacheTimer.restart();
    m_cachedStats = QJsonObject();
}

void AdminHttpServer::onHttpConnection() {
    while (m_httpServer->hasPendingConnections()) {
        QTcpSocket* socket = m_httpServer->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &AdminHttpServer::onHttpRequest);
        connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
            m_sseClients.remove(socket);
            m_requestBuffers.remove(socket);
            socket->deleteLater();
        });
    }
}

void AdminHttpServer::onHttpRequest() {
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    // Accumulate data for this socket
    QByteArray& buffer = m_requestBuffers[socket];
    buffer.append(socket->readAll());

    // Check if we have the full request (look for \r\n\r\n)
    int headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd < 0) {
        headerEnd = buffer.indexOf("\n\n");
        if (headerEnd < 0) return; // Header not complete yet
    }

    // Parse Content-Length to determine if body is complete
    int bodyStart = buffer.indexOf("\r\n\r\n");
    if (bodyStart < 0) bodyStart = buffer.indexOf("\n\n");
    int sepLen = (buffer.indexOf("\r\n\r\n") >= 0) ? 4 : 2;

    int contentLength = 0;
    QByteArray headerPart = buffer.left(bodyStart);
    QList<QByteArray> headerLines = headerPart.split('\n');
    for (const auto& line : headerLines) {
        if (line.toLower().startsWith("content-length:")) {
            contentLength = line.mid(15).trimmed().toInt();
            break;
        }
    }

    int receivedBodySize = buffer.size() - bodyStart - sepLen;
    if (receivedBodySize < contentLength) {
        return; // Body not complete yet, wait for more data
    }

    // Full request received, process it
    QByteArray requestData = buffer;
    m_requestBuffers.remove(socket);
    handleHttpRequest(socket, requestData);
}

void AdminHttpServer::handleHttpRequest(QTcpSocket* socket, const QByteArray& requestData) {
    // Parse HTTP request
    QList<QByteArray> lines = requestData.split('\n');
    if (lines.isEmpty()) {
        socket->disconnectFromHost();
        return;
    }

    // Parse request line
    QByteArray requestLine = lines[0].trimmed();
    QList<QByteArray> parts = requestLine.split(' ');
    if (parts.size() < 3) {
        socket->disconnectFromHost();
        return;
    }

    QString method = QString::fromUtf8(parts[0]);
    QString fullPath = QString::fromUtf8(parts[1]);

    // Split path and query string
    QString path = fullPath;
    QString queryString;
    int queryPos = fullPath.indexOf('?');
    if (queryPos >= 0) {
        path = fullPath.left(queryPos);
        queryString = fullPath.mid(queryPos + 1);
    }

    // Parse headers
    QMap<QString, QString> headers;
    int headerEnd = -1;
    for (int i = 1; i < lines.size(); ++i) {
        QByteArray line = lines[i].trimmed();
        if (line.isEmpty()) {
            headerEnd = i;
            break;
        }
        int colonPos = line.indexOf(':');
        if (colonPos > 0) {
            QString key = QString::fromUtf8(line.left(colonPos).trimmed()).toLower();
            QString value = QString::fromUtf8(line.mid(colonPos + 1).trimmed());
            headers[key] = value;
        }
    }

    // Extract body (everything after the empty line)
    QByteArray body;
    int sepPos = requestData.indexOf("\r\n\r\n");
    if (sepPos >= 0) {
        body = requestData.mid(sepPos + 4);
    } else {
        sepPos = requestData.indexOf("\n\n");
        if (sepPos >= 0) {
            body = requestData.mid(sepPos + 2);
        }
    }

    // Check if this is an SSE endpoint
    if (path == "/api/events" && method == "GET") {
        if (!checkAuth(headers)) {
            QByteArray response = "HTTP/1.1 401 Unauthorized\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Connection: close\r\n"
                                  "\r\nUnauthorized";
            socket->write(response);
            socket->disconnectFromHost();
            return;
        }

        // SSE response
        QByteArray response = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/event-stream\r\n"
                              "Cache-Control: no-cache\r\n"
                              "Connection: keep-alive\r\n"
                              "Access-Control-Allow-Origin: *\r\n"
                              "\r\n";
        socket->write(response);
        socket->flush();
        m_sseClients.insert(socket);
        return;
    }

    // Handle CORS preflight
    if (method == "OPTIONS") {
        QByteArray response = "HTTP/1.1 204 No Content\r\n"
                              "Access-Control-Allow-Origin: *\r\n"
                              "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n"
                              "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
                              "Access-Control-Max-Age: 86400\r\n"
                              "\r\n";
        socket->write(response);
        socket->disconnectFromHost();
        return;
    }

    // Serve index.html for root path
    if (path == "/" || path == "/index.html") {
        // Try qrc resource first, then local file relative to exe directory
        QFile file(":/admin/index.html");
        if (!file.exists()) {
            QString appDir = QCoreApplication::applicationDirPath();
            file.setFileName(appDir + "/../admin/index.html");
        }
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray content = file.readAll();
            QByteArray response = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/html; charset=utf-8\r\n"
                                  "Content-Length: " + QByteArray::number(content.size()) + "\r\n"
                                  "Connection: close\r\n"
                                  "\r\n" + content;
            socket->write(response);
        } else {
            QByteArray response = "HTTP/1.1 404 Not Found\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Connection: close\r\n"
                                  "\r\nNot Found";
            socket->write(response);
        }
        socket->disconnectFromHost();
        return;
    }

    // Serve avatar static files (no auth required for avatars)
    if (path.startsWith("/avatars/")) {
        QString avatarDir = QCoreApplication::applicationDirPath() + "/avatars";
        QString filename = path.mid(9); // remove "/avatars/"
        // Security: prevent directory traversal
        if (filename.contains("..") || filename.contains('/') || filename.contains('\\')) {
            QByteArray response = "HTTP/1.1 403 Forbidden\r\n"
                                  "Connection: close\r\n\r\n";
            socket->write(response);
            socket->disconnectFromHost();
            return;
        }
        QString filePath = avatarDir + "/" + filename;
        QFile avatarFile(filePath);
        if (avatarFile.open(QIODevice::ReadOnly)) {
            QByteArray content = avatarFile.readAll();
            QByteArray response = "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: image/png\r\n"
                                  "Content-Length: " + QByteArray::number(content.size()) + "\r\n"
                                  "Cache-Control: public, max-age=86400\r\n"
                                  "Connection: close\r\n"
                                  "\r\n" + content;
            socket->write(response);
        } else {
            QByteArray response = "HTTP/1.1 404 Not Found\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Connection: close\r\n"
                                  "\r\nNot Found";
            socket->write(response);
        }
        socket->disconnectFromHost();
        return;
    }

    // Handle API requests
    if (path.startsWith("/api/")) {
        // Auth check for all API endpoints except /api/auth
        if (path != "/api/auth" && !checkAuth(headers)) {
            QJsonObject errObj;
            errObj["code"] = 401;
            errObj["message"] = "Unauthorized";
            QByteArray errBody = QJsonDocument(errObj).toJson(QJsonDocument::Compact);
            QByteArray response = "HTTP/1.1 401 Unauthorized\r\n"
                                  "Content-Type: application/json; charset=utf-8\r\n"
                                  "Content-Length: " + QByteArray::number(errBody.size()) + "\r\n"
                                  "Access-Control-Allow-Origin: *\r\n"
                                  "Connection: close\r\n"
                                  "\r\n" + errBody;
            socket->write(response);
            socket->disconnectFromHost();
            return;
        }

        QByteArray result = handleApiRequest(method, path, body, queryString);

        QByteArray response = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: application/json; charset=utf-8\r\n"
                              "Content-Length: " + QByteArray::number(result.size()) + "\r\n"
                              "Access-Control-Allow-Origin: *\r\n"
                              "Connection: close\r\n"
                              "\r\n" + result;
        socket->write(response);
        socket->disconnectFromHost();
        return;
    }

    // 404
    QByteArray response = "HTTP/1.1 404 Not Found\r\n"
                          "Content-Type: text/plain\r\n"
                          "Connection: close\r\n"
                          "\r\nNot Found";
    socket->write(response);
    socket->disconnectFromHost();
}

bool AdminHttpServer::checkAuth(const QMap<QString, QString>& headers) {
    QString authHeader = headers.value("authorization", "");
    if (authHeader.startsWith("Bearer ")) {
        QString token = authHeader.mid(7);
        return token == m_adminToken;
    }
    return false;
}

QByteArray AdminHttpServer::handleApiRequest(const QString& method, const QString& path, const QByteArray& body, const QString& queryString) {
    auto makeError = [](int code, const QString& message) -> QByteArray {
        QJsonObject obj;
        obj["code"] = code;
        obj["message"] = message;
        return QJsonDocument(obj).toJson(QJsonDocument::Compact);
    };

    auto makeSuccess = [](const QJsonObject& data) -> QByteArray {
        QJsonObject obj;
        obj["code"] = 0;
        obj["data"] = data;
        return QJsonDocument(obj).toJson(QJsonDocument::Compact);
    };

    // Parse headers from body context (we already extracted body separately)
    // For auth check on API requests, we need to check from the stored request
    // Actually, auth is checked before calling handleApiRequest for SSE,
    // but for other API calls we need to check here.
    // We'll rely on the caller to have checked auth, or check via a different mechanism.
    // Since we don't have headers here, let's adjust: we pass headers through.
    // For simplicity, API calls (non-SSE) will be checked in handleHttpRequest.

    // POST /api/auth
    if (path == "/api/auth" && method == "POST") {
        QJsonDocument doc = QJsonDocument::fromJson(body);
        if (!doc.isObject()) {
            return makeError(400, "Invalid request body");
        }
        QString token = doc.object().value("token").toString();
        if (token == m_adminToken) {
            QJsonObject data;
            data["authenticated"] = true;
            return QJsonDocument(data).toJson(QJsonDocument::Compact);
        }
        return makeError(401, "Invalid token");
    }

    // All other API endpoints require auth - checked in handleHttpRequest

    // GET /api/stats
    if (path == "/api/stats" && method == "GET") {
        // Use cache with TTL
        if (!m_cachedStats.isEmpty() && m_statsCacheTimer.elapsed() < STATS_CACHE_TTL_MS) {
            return QJsonDocument(m_cachedStats).toJson(QJsonDocument::Compact);
        }

        auto stats = m_db->getStatistics();
        stats.onlineUsers = m_tcpServer->getAuthenticatedSessions().size();

        QJsonObject data;
        data["totalUsers"] = stats.totalUsers;
        data["onlineUsers"] = stats.onlineUsers;
        data["totalFriendships"] = stats.totalFriendships;
        data["totalOfflineMessages"] = stats.totalOfflineMessages;
        data["pendingFriendRequests"] = stats.pendingFriendRequests;
        data["totalMessages"] = stats.totalMessages;
        data["totalGroups"] = stats.totalGroups;
        data["totalGroupMessages"] = stats.totalGroupMessages;

        m_cachedStats = data;
        m_statsCacheTimer.restart();

        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // GET /api/users
    if (path == "/api/users" && method == "GET") {
        // Parse offset and limit from query string
        int offset = 0, limit = 100;
        if (!queryString.isEmpty()) {
            QStringList params = queryString.split("&");
            for (const QString& param : params) {
                QStringList kv = param.split("=");
                if (kv.size() == 2) {
                    if (kv[0] == "offset") offset = kv[1].toInt();
                    if (kv[0] == "limit") limit = kv[1].toInt();
                }
            }
        }

        auto users = m_db->getAllUsers(offset, limit);
        int total = m_db->getUserCount();

        QJsonArray userArray;
        for (const auto& u : users) {
            QJsonObject obj;
            obj["userId"] = static_cast<qint64>(u.userId);
            obj["username"] = u.username;
            obj["nickname"] = u.nickname;
            obj["avatar"] = u.avatar;
            userArray.append(obj);
        }

        QJsonObject data;
        data["users"] = userArray;
        data["total"] = total;
        data["offset"] = offset;
        data["limit"] = limit;

        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // GET /api/online-users
    if (path == "/api/online-users" && method == "GET") {
        auto sessions = m_tcpServer->getAuthenticatedSessions();

        QJsonArray userArray;
        for (auto it = sessions.constBegin(); it != sessions.constEnd(); ++it) {
            ClientSession* session = it.value();
            if (!session) continue;

            UserInfo info = m_db->getUserInfo(it.key());
            QJsonObject obj;
            obj["userId"] = static_cast<qint64>(it.key());
            obj["username"] = info.username;
            obj["nickname"] = info.nickname;
            obj["avatar"] = info.avatar;
            userArray.append(obj);
        }

        QJsonObject data;
        data["users"] = userArray;
        data["count"] = userArray.size();

        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // GET /api/friendships
    if (path == "/api/friendships" && method == "GET") {
        // Get all users and their friend lists
        auto users = m_db->getAllUsers(0, 10000);
        QJsonArray friendshipsArray;

        for (const auto& u : users) {
            auto friends = m_db->getFriendList(u.userId);
            for (const auto& f : friends) {
                // Only include pairs where userId < friendId to avoid duplicates
                if (u.userId < f.userId) {
                    QJsonObject obj;
                    obj["userId"] = static_cast<qint64>(u.userId);
                    obj["username"] = u.username;
                    obj["friendId"] = static_cast<qint64>(f.userId);
                    obj["friendUsername"] = f.username;
                    friendshipsArray.append(obj);
                }
            }
        }

        QJsonObject data;
        data["friendships"] = friendshipsArray;
        data["total"] = friendshipsArray.size();

        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // GET /api/groups — List all groups (paginated)
    if (path == "/api/groups" && method == "GET") {
        int offset = 0, limit = 100;
        if (!queryString.isEmpty()) {
            QStringList params = queryString.split("&");
            for (const QString& param : params) {
                QStringList kv = param.split("=");
                if (kv.size() == 2) {
                    if (kv[0] == "offset") offset = kv[1].toInt();
                    if (kv[0] == "limit") limit = kv[1].toInt();
                }
            }
        }

        auto groups = m_db->getAllGroups(offset, limit);
        int total = m_db->getGroupCount();

        QJsonArray groupArray;
        for (const auto& g : groups) {
            QJsonObject obj;
            obj["groupId"] = static_cast<qint64>(g.groupId);
            obj["name"] = g.name;
            obj["avatar"] = g.avatar;
            obj["ownerId"] = static_cast<qint64>(g.ownerId);
            obj["memberCount"] = g.memberCount;
            obj["isDefault"] = g.isDefault;
            obj["maxMembers"] = g.maxMembers;
            obj["createdAt"] = g.createdAt;
            groupArray.append(obj);
        }

        QJsonObject data;
        data["groups"] = groupArray;
        data["total"] = total;
        data["offset"] = offset;
        data["limit"] = limit;

        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // GET /api/groups/{id}/messages — Get group message history
    if (path.contains("/messages") && path.startsWith("/api/groups/") && method == "GET") {
        // Extract group ID from path: /api/groups/{id}/messages
        QString idStr = path.mid(QString("/api/groups/").length());
        idStr = idStr.left(idStr.indexOf("/messages"));
        bool ok = false;
        uint64_t groupId = idStr.toULongLong(&ok);
        if (!ok) {
            return makeError(400, "Invalid group ID");
        }

        int limit = 50;
        if (!queryString.isEmpty()) {
            QStringList params = queryString.split("&");
            for (const QString& param : params) {
                QStringList kv = param.split("=");
                if (kv.size() == 2 && kv[0] == "limit") {
                    limit = kv[1].toInt();
                }
            }
        }

        auto messages = m_db->getRecentGroupMessages(limit);

        // Filter messages for this specific group
        QJsonArray msgArray;
        for (const auto& msg : messages) {
            if (msg.groupId == groupId) {
                QJsonObject obj;
                obj["messageId"] = static_cast<qint64>(msg.messageId);
                obj["groupId"] = static_cast<qint64>(msg.groupId);
                obj["senderId"] = static_cast<qint64>(msg.senderId);
                obj["senderNickname"] = msg.senderNickname;
                obj["content"] = msg.content;
                obj["type"] = static_cast<int>(msg.type);
                obj["timestamp"] = msg.timestamp.toSecsSinceEpoch();
                msgArray.append(obj);
            }
        }

        QJsonObject data;
        data["messages"] = msgArray;
        data["total"] = msgArray.size();

        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // GET /api/groups/{id} — Get group details with members
    if (path.startsWith("/api/groups/") && method == "GET") {
        QString idStr = path.mid(QString("/api/groups/").length());
        bool ok = false;
        uint64_t groupId = idStr.toULongLong(&ok);
        if (!ok) {
            return makeError(400, "Invalid group ID");
        }

        auto groupInfo = m_db->getGroupInfo(groupId);
        if (groupInfo.groupId == 0) {
            return makeError(404, "Group not found");
        }

        auto members = m_db->getGroupMembers(groupId);

        QJsonObject groupObj;
        groupObj["groupId"] = static_cast<qint64>(groupInfo.groupId);
        groupObj["name"] = groupInfo.name;
        groupObj["avatar"] = groupInfo.avatar;
        groupObj["ownerId"] = static_cast<qint64>(groupInfo.ownerId);
        groupObj["memberCount"] = groupInfo.memberCount;
        groupObj["isDefault"] = groupInfo.isDefault;
        groupObj["maxMembers"] = groupInfo.maxMembers;
        groupObj["createdAt"] = groupInfo.createdAt;

        QJsonArray memberArray;
        for (const auto& m : members) {
            QJsonObject obj;
            obj["userId"] = static_cast<qint64>(m.userId);
            obj["username"] = m.username;
            obj["nickname"] = m.nickname;
            obj["avatar"] = m.avatar;
            obj["role"] = m.role;
            memberArray.append(obj);
        }

        QJsonObject data;
        data["group"] = groupObj;
        data["members"] = memberArray;

        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // DELETE /api/groups/{id} — Delete a group (not default group)
    if (path.startsWith("/api/groups/") && method == "DELETE") {
        QString idStr = path.mid(QString("/api/groups/").length());
        bool ok = false;
        uint64_t groupId = idStr.toULongLong(&ok);
        if (!ok) {
            return makeError(400, "Invalid group ID");
        }

        auto groupInfo = m_db->getGroupInfo(groupId);
        if (groupInfo.groupId == 0) {
            return makeError(404, "Group not found");
        }

        if (groupInfo.isDefault) {
            return makeError(403, "Cannot delete default group");
        }

        if (!m_db->dissolveGroup(groupId)) {
            return makeError(500, "Failed to delete group");
        }

        // Broadcast SSE event
        QJsonObject eventData;
        eventData["groupId"] = static_cast<qint64>(groupId);
        eventData["groupName"] = groupInfo.name;
        broadcastSSEEvent("group_deleted", eventData);

        invalidateStatsCache();

        QJsonObject data;
        data["deleted"] = true;
        data["groupId"] = static_cast<qint64>(groupId);
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // POST /api/users/<id>/kick
    if (path.startsWith("/api/users/") && path.endsWith("/kick") && method == "POST") {
        // Extract user ID from path
        QString idStr = path.mid(QString("/api/users/").length());
        idStr = idStr.left(idStr.length() - QString("/kick").length());
        bool ok = false;
        uint64_t userId = idStr.toULongLong(&ok);
        if (!ok) {
            return makeError(400, "Invalid user ID");
        }

        ClientSession* session = m_tcpServer->getSession(userId);
        if (!session) {
            return makeError(404, "User not online");
        }

        // Parse kick reason from body
        QString reason = "管理员踢出";
        QJsonDocument doc = QJsonDocument::fromJson(body);
        if (doc.isObject()) {
            QString customReason = doc.object().value("reason").toString();
            if (!customReason.isEmpty()) {
                reason = customReason;
            }
        }

        // Send ServerKickNotify
        QJsonObject kickBody;
        kickBody["reason"] = reason;
        uint32_t seq = ChatProtocol::nextSequence();
        session->sendPacket(
            MessageType::ServerKickNotify, 0, seq,
            QJsonDocument(kickBody).toJson(QJsonDocument::Compact));

        // Delayed disconnect
        QTimer::singleShot(100, this, [this, userId]() {
            m_tcpServer->removeSession(userId);
        });

        // Broadcast SSE event
        QJsonObject eventData;
        eventData["userId"] = static_cast<qint64>(userId);
        eventData["reason"] = reason;
        broadcastSSEEvent("user_kicked", eventData);

        invalidateStatsCache();

        QJsonObject data;
        data["kicked"] = true;
        data["userId"] = static_cast<qint64>(userId);
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // DELETE /api/users/<id> — Delete user
    if (path.startsWith("/api/users/") && method == "DELETE") {
        QString idStr = path.mid(QString("/api/users/").length());
        bool ok = false;
        uint64_t userId = idStr.toULongLong(&ok);
        if (!ok) {
            return makeError(400, "Invalid user ID");
        }

        // Get user info before deletion for SSE broadcast
        UserInfo userInfo = m_db->getUserInfo(userId);
        if (userInfo.userId == 0) {
            return makeError(404, "User not found");
        }

        // Kick user if online
        ClientSession* session = m_tcpServer->getSession(userId);
        if (session) {
            QJsonObject kickBody;
            kickBody["reason"] = "用户已被删除";
            uint32_t seq = ChatProtocol::nextSequence();
            session->sendPacket(
                MessageType::ServerKickNotify, 0, seq,
                QJsonDocument(kickBody).toJson(QJsonDocument::Compact));
            QTimer::singleShot(100, this, [this, userId]() {
                m_tcpServer->removeSession(userId);
            });
        }

        if (!m_db->deleteUser(userId)) {
            return makeError(500, "Failed to delete user");
        }

        // Broadcast SSE event
        QJsonObject eventData;
        eventData["userId"] = static_cast<qint64>(userId);
        eventData["username"] = userInfo.username;
        broadcastSSEEvent("user_deleted", eventData);

        invalidateStatsCache();

        QJsonObject data;
        data["deleted"] = true;
        data["userId"] = static_cast<qint64>(userId);
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // POST /api/users/<id>/reset-pwd — Reset user password
    if (path.startsWith("/api/users/") && path.endsWith("/reset-pwd") && method == "POST") {
        QString idStr = path.mid(QString("/api/users/").length());
        idStr = idStr.left(idStr.length() - QString("/reset-pwd").length());
        bool ok = false;
        uint64_t userId = idStr.toULongLong(&ok);
        if (!ok) {
            return makeError(400, "Invalid user ID");
        }

        QJsonDocument doc = QJsonDocument::fromJson(body);
        if (!doc.isObject()) {
            return makeError(400, "Invalid request body");
        }
        QString newPasswordHash = doc.object().value("password").toString();
        if (newPasswordHash.isEmpty()) {
            return makeError(400, "Password hash is required");
        }

        if (!m_db->resetUserPassword(userId, newPasswordHash)) {
            return makeError(500, "Failed to reset password");
        }

        QJsonObject data;
        data["reset"] = true;
        data["userId"] = static_cast<qint64>(userId);
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // GET /api/friend-requests — Get all friend requests
    if (path == "/api/friend-requests" && method == "GET") {
        int offset = 0, limit = 100;
        if (!queryString.isEmpty()) {
            QStringList params = queryString.split("&");
            for (const QString& param : params) {
                QStringList kv = param.split("=");
                if (kv.size() == 2) {
                    if (kv[0] == "offset") offset = kv[1].toInt();
                    if (kv[0] == "limit") limit = kv[1].toInt();
                }
            }
        }

        auto requests = m_db->getAllFriendRequests(offset, limit);

        QJsonArray reqArray;
        for (const auto& req : requests) {
            QJsonObject obj;
            obj["requestId"] = static_cast<qint64>(req.requestId);
            obj["fromUserId"] = static_cast<qint64>(req.fromUserId);
            obj["toUserId"] = static_cast<qint64>(req.toUserId);
            obj["fromUsername"] = req.fromUsername;
            obj["message"] = req.message;
            obj["status"] = static_cast<int>(req.status);
            obj["createdAt"] = req.createdAt.toSecsSinceEpoch();
            reqArray.append(obj);
        }

        QJsonObject data;
        data["requests"] = reqArray;
        data["total"] = reqArray.size();

        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // DELETE /api/friend-requests — Clear all friend requests
    if (path == "/api/friend-requests" && method == "DELETE") {
        if (!m_db->clearAllFriendRequests()) {
            return makeError(500, "Failed to clear friend requests");
        }

        invalidateStatsCache();

        QJsonObject data;
        data["cleared"] = true;
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // GET /api/recent-messages — Get recent messages
    if (path == "/api/recent-messages" && method == "GET") {
        int limit = 50;
        if (!queryString.isEmpty()) {
            QStringList params = queryString.split("&");
            for (const QString& param : params) {
                QStringList kv = param.split("=");
                if (kv.size() == 2 && kv[0] == "limit") {
                    limit = kv[1].toInt();
                }
            }
        }

        auto messages = m_db->getRecentMessages(limit);

        QJsonArray msgArray;
        for (const auto& msg : messages) {
            QJsonObject obj;
            obj["messageId"] = static_cast<qint64>(msg.messageId);
            obj["senderId"] = static_cast<qint64>(msg.senderId);
            obj["receiverId"] = static_cast<qint64>(msg.receiverId);
            obj["content"] = msg.content;
            obj["type"] = static_cast<int>(msg.type);
            obj["timestamp"] = msg.timestamp.toSecsSinceEpoch();
            msgArray.append(obj);
        }

        QJsonObject data;
        data["messages"] = msgArray;
        data["total"] = msgArray.size();

        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // DELETE /api/messages — Clear all messages
    if (path == "/api/messages" && method == "DELETE") {
        if (!m_db->clearAllMessages()) {
            return makeError(500, "Failed to clear messages");
        }

        invalidateStatsCache();

        QJsonObject data;
        data["cleared"] = true;
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // DELETE /api/offline-messages — Clear all offline messages
    if (path == "/api/offline-messages" && method == "DELETE") {
        if (!m_db->clearAllOfflineMessages()) {
            return makeError(500, "Failed to clear offline messages");
        }

        invalidateStatsCache();

        QJsonObject data;
        data["cleared"] = true;
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // POST /api/database/reset — Reset database (clear all data, keep table structure)
    if (path == "/api/database/reset" && method == "POST") {
        if (!m_db->clearDatabase()) {
            return makeError(500, "Failed to reset database");
        }

        // Broadcast SSE event
        QJsonObject eventData;
        broadcastSSEEvent("database_reset", eventData);

        invalidateStatsCache();

        QJsonObject data;
        data["reset"] = true;
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // GET /api/server/info — Get server info
    if (path == "/api/server/info" && method == "GET") {
        qint64 uptimeSeconds = m_startTime.secsTo(QDateTime::currentDateTime());
        qint64 uptimeDays = uptimeSeconds / 86400;
        qint64 uptimeHours = (uptimeSeconds % 86400) / 3600;
        qint64 uptimeMinutes = (uptimeSeconds % 3600) / 60;
        qint64 uptimeSecs = uptimeSeconds % 60;

        QString uptimeStr = QString("%1天 %2小时 %3分钟 %4秒")
            .arg(uptimeDays)
            .arg(uptimeHours)
            .arg(uptimeMinutes)
            .arg(uptimeSecs);

        QJsonObject data;
        data["version"] = m_serverVersion;
        data["uptime"] = uptimeStr;
        data["uptimeSeconds"] = uptimeSeconds;
        data["startTime"] = m_startTime.toString(Qt::ISODate);

        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

        // GET /api/messages - Get recent messages
    if (path == "/api/messages" && method == "GET") {
        int limit = 50;
        if (!queryString.isEmpty()) {
            QStringList params = queryString.split("&");
            for (const QString& p : params) {
                QStringList kv = p.split("=");
                if (kv.size() == 2 && kv[0] == "limit") limit = kv[1].toInt();
            }
        }
        auto msgs = m_db->getRecentMessages(limit);
        QJsonArray arr;
        for (const auto& m : msgs) {
            QJsonObject o;
            o["messageId"] = static_cast<qint64>(m.messageId);
            o["senderId"] = static_cast<qint64>(m.senderId);
            o["senderNickname"] = m.senderNickname;
            o["content"] = m.content.left(100);
            o["timestamp"] = m.timestamp.toSecsSinceEpoch();
            arr.append(o);
        }
        QJsonObject data; data["messages"] = arr;
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // GET /api/settings/astrbot-url
    if (path == "/api/settings/astrbot-url" && method == "GET") {
        QJsonObject data; data["key"] = m_db->loadSetting("astrbot_url", "");
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }

    // POST /api/settings/astrbot-url
    if (path == "/api/settings/astrbot-url" && method == "POST") {
        QJsonDocument doc = QJsonDocument::fromJson(body);
        QString key = doc.object()["key"].toString();
        m_db->saveSetting("astrbot_url", key);
        QJsonObject data; data["success"] = true;
        return QJsonDocument(data).toJson(QJsonDocument::Compact);
    }
    return makeError(404, "API endpoint not found");
}

} // namespace chatroom::server
