#pragma once

#include <QObject>
#include <QTcpServer>
#include <QMap>
#include <QSet>
#include <QElapsedTimer>
#include <QTimer>
#include <QJsonObject>
#include <QDateTime>

namespace chatroom::server {

class ServerDatabase;
class TcpServer;

class AdminHttpServer : public QObject {
    Q_OBJECT
public:
    explicit AdminHttpServer(ServerDatabase* db, TcpServer* tcpServer, QObject* parent = nullptr);
    ~AdminHttpServer();

    bool start(quint16 port, const QString& adminToken);
    void stop();
    QString adminToken() const { return m_adminToken; }

    // Called from main thread when user connects/disconnects
    void broadcastSSEEvent(const QString& event, const QJsonObject& data);
    void invalidateStatsCache();

private slots:
    void onHttpConnection();
    void onHttpRequest();

private:
    void handleHttpRequest(QTcpSocket* socket, const QByteArray& requestData);
    QByteArray handleApiRequest(const QString& method, const QString& path, const QByteArray& body, const QString& queryString = QString());
    bool checkAuth(const QMap<QString, QString>& headers);

    QTcpServer* m_httpServer = nullptr;
    ServerDatabase* m_db;
    TcpServer* m_tcpServer;
    QString m_adminToken;

    // SSE clients (main thread only)
    QSet<QTcpSocket*> m_sseClients;
    QTimer* m_sseHeartbeatTimer = nullptr;

    // Stats cache
    QJsonObject m_cachedStats;
    QElapsedTimer m_statsCacheTimer;
    static constexpr int STATS_CACHE_TTL_MS = 5000;

    // Note: m_mutex removed. All operations run on the main thread only.
    // If multi-threading is needed in the future, re-add QMutex protection.

    // HTTP request buffering
    QMap<QTcpSocket*, QByteArray> m_requestBuffers;

    // Server info
    QDateTime m_startTime;
    QString m_serverVersion = "v2.1";
};

} // namespace chatroom::server
