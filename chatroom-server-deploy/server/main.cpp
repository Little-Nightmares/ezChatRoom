#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include "core/TcpServer.h"
#include "core/ClientSession.h"
#include "core/MessageRouter.h"
#include "database/ServerDatabase.h"
#include "protocol/ChatProtocol.h"
#include "protocol/MessageTypes.h"
#include "core/BotService.h"
#include "models/UserInfo.h"
#include "admin/AdminHttpServer.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

using namespace chatroom::server;

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("ChatRoomServer");
    app.setApplicationVersion("1.0.0");

    // Parse command line arguments for port
    quint16 port = 6667;
    quint16 adminPort = 19527;
    QString adminToken;
    for (int i = 1; i < argc; ++i) {
        if (QString(argv[i]) == "--port" && i + 1 < argc) {
            bool ok = false;
            int p = QString(argv[i + 1]).toInt(&ok);
            if (ok && p > 0 && p < 65536) {
                port = static_cast<quint16>(p);
            }
        }
        if (QString(argv[i]) == "--admin-port" && i + 1 < argc) {
            bool ok = false;
            int p = QString(argv[i + 1]).toInt(&ok);
            if (ok && p > 0 && p < 65536) {
                adminPort = static_cast<quint16>(p);
            }
        }
        if (QString(argv[i]) == "--admin-token" && i + 1 < argc) {
            adminToken = QString(argv[i + 1]);
        }
        if (QString(argv[i]) == "--help" || QString(argv[i]) == "-h") {
            qDebug() << "Usage: ChatRoomServer [--port PORT] [--admin-port PORT] [--admin-token TOKEN]";
            qDebug() << "  --port PORT         Main server port (default: 6667)";
            qDebug() << "  --admin-port PORT   Admin HTTP server port (default: 19527)";
            qDebug() << "  --admin-token TOKEN Admin authentication token";
            return 0;
        }
    }

    // Clean up stale WAL/SHM files if the main DB doesn't exist
    QString dbPath = "chatroom_server.db";
    if (!QFile::exists(dbPath)) {
        QFile::remove(dbPath + "-wal");
        QFile::remove(dbPath + "-shm");
    }

    // Initialize the server database
    ServerDatabase db;
    if (!db.initialize(dbPath)) {
        qCritical() << "Failed to initialize database";
        return 1;
    }
    qDebug() << "Database initialized successfully";

    // Create upload directories for file sharing
    QDir().mkpath("uploads/images");
    QDir().mkpath("uploads/files");
    QDir().mkpath("uploads/avatars");
    qDebug() << "Upload directories created";

    // Create the TCP server
    TcpServer server;

    // Create the message router with database and server references
    MessageRouter router(&db, &server);

    // Set the router on the server so new sessions get registered
    server.setRouter(&router);

    // Notify friends when user comes online
    QObject::connect(&server, &TcpServer::clientConnected,
                     [&db, &server](uint64_t userId) {
        auto friends = db.getFriendList(userId);
        UserInfo userInfo = db.getUserInfo(userId);
        for (const auto& friendInfo : friends) {
            ClientSession* session = server.getSession(friendInfo.userId);
            if (session) {
                QJsonObject notifyObj;
                notifyObj["userId"] = static_cast<qint64>(userId);
                notifyObj["username"] = userInfo.username;
                notifyObj["nickname"] = userInfo.nickname;
                session->sendPacket(
                    chatroom::protocol::MessageType::FriendOnlineNotify, 0,
                    chatroom::protocol::ChatProtocol::nextSequence(),
                    QJsonDocument(notifyObj).toJson(QJsonDocument::Compact));
            }
        }
        qDebug() << "User" << userId << "online, notified" << friends.size() << "friends";
    });

    // Notify friends when user goes offline
    QObject::connect(&server, &TcpServer::clientDisconnected,
                     [&db, &server](uint64_t userId) {
        auto friends = db.getFriendList(userId);
        for (const auto& friendInfo : friends) {
            ClientSession* session = server.getSession(friendInfo.userId);
            if (session) {
                QJsonObject notifyObj;
                notifyObj["userId"] = static_cast<qint64>(userId);
                session->sendPacket(
                    chatroom::protocol::MessageType::FriendOfflineNotify, 0,
                    chatroom::protocol::ChatProtocol::nextSequence(),
                    QJsonDocument(notifyObj).toJson(QJsonDocument::Compact));
            }
        }
        qDebug() << "User" << userId << "offline, notified" << friends.size() << "friends";
    });

    // Start listening on the specified port
    if (!server.start(port)) {
        qCritical() << "Failed to start server on port" << port
                    << "-" << server.errorString();
        qCritical() << "Possible reasons:";
        qCritical() << "  1. Another instance is already running on port" << port;
        qCritical() << "  2. The port is in use by another application";
        qCritical() << "  3. Insufficient permissions (ports < 1024 require root)";
        qCritical() << "";
        qCritical() << "Try: kill any existing ChatRoomServer process, or use --port PORT";
        return 1;
    }

    qDebug() << "ChatRoom server started on port" << port;

    // --- Admin HTTP Server ---
    // Token priority: command line > admin.token file > random generate
    if (adminToken.isEmpty()) {
        QFile tokenFile("admin.token");
        if (tokenFile.exists() && tokenFile.open(QIODevice::ReadOnly)) {
            adminToken = QString::fromUtf8(tokenFile.readAll()).trimmed();
            tokenFile.close();
        }
    }
    if (adminToken.isEmpty()) {
        // Generate a random 32-byte hex token
        QByteArray randomBytes(32, 0);
        for (int i = 0; i < 32; ++i) {
            randomBytes[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
        }
        adminToken = QString::fromUtf8(randomBytes.toHex());
        QFile tokenFile("admin.token");
        if (tokenFile.open(QIODevice::WriteOnly)) {
            tokenFile.write(adminToken.toUtf8());
            tokenFile.close();
            qDebug() << "Admin token generated and saved to admin.token";
        }
    }

    AdminHttpServer adminHttpServer(&db, &server);
    if (!adminHttpServer.start(adminPort, adminToken)) {
        qWarning() << "Failed to start admin HTTP server on port" << adminPort;
    } else {
        qDebug() << "Admin HTTP server started on port" << adminPort;
        qDebug() << "Admin token:" << adminToken;
    }

    // Tell the message router about the admin port for avatar URL generation
    router.setAdminPort(adminPort);

    BotService botService(&db, &server);
    router.setBotService(&botService);
    qDebug() << "BotService initialized";

    // Connect user connect/disconnect signals to SSE broadcasting
    QObject::connect(&server, &TcpServer::clientConnected,
                     &adminHttpServer, [&adminHttpServer](uint64_t userId) {
        QJsonObject data;
        data["userId"] = static_cast<qint64>(userId);
        adminHttpServer.broadcastSSEEvent("user_connected", data);
        adminHttpServer.invalidateStatsCache();
    });

    QObject::connect(&server, &TcpServer::clientDisconnected,
                     &adminHttpServer, [&adminHttpServer](uint64_t userId) {
        QJsonObject data;
        data["userId"] = static_cast<qint64>(userId);
        adminHttpServer.broadcastSSEEvent("user_disconnected", data);
        adminHttpServer.invalidateStatsCache();
    });

    return app.exec();
}

