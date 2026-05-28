#pragma once

#include <QObject>
#include <memory>
#include <QTimer>
#include <QtQml/qqml.h>

namespace chatroom::client {

class TcpClient;
class MessageHandler;
class HeartbeatManager;
class SessionManager;
class DatabaseManager;

class AppManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool isReconnecting READ isReconnecting NOTIFY reconnectingChanged)
    Q_PROPERTY(int reconnectAttempt READ reconnectAttempt NOTIFY reconnectingChanged)

public:
    explicit AppManager(QObject* parent = nullptr);
    ~AppManager();

    // Server info for avatar URL construction
    Q_INVOKABLE QString serverHost() const { return m_lastHost; }
    void setAdminPort(quint16 port) { m_adminPort = port; }
    quint16 adminPort() const { return m_adminPort; }
    QString toAvatarUrl(const QString& relativePath) const;

    void initialize();
    void connectToServer(const QString& host, quint16 port);

    TcpClient* tcpClient() const;
    SessionManager* sessionManager() const;
    DatabaseManager* databaseManager() const;
    MessageHandler* messageHandler() const;
    HeartbeatManager* heartbeatManager() const;

    void attemptReconnect();
    void setReconnecting(bool reconnecting);
    bool isReconnecting() const { return m_reconnecting; }
    int reconnectAttempt() const { return m_reconnectAttempt; }

signals:
    void reconnectingChanged();
    void reconnectFailed();
    void reconnectSuccess();

private:
    std::unique_ptr<TcpClient> m_tcpClient;
    std::unique_ptr<MessageHandler> m_messageHandler;
    std::unique_ptr<HeartbeatManager> m_heartbeatManager;
    std::unique_ptr<SessionManager> m_sessionManager;
    std::unique_ptr<DatabaseManager> m_databaseManager;

    QTimer* m_reconnectTimer = nullptr;
    int m_reconnectAttempt = 0;
    static constexpr int MAX_RECONNECT_ATTEMPTS = 5;
    bool m_reconnecting = false;
    quint16 m_adminPort = 0;
    QString m_lastHost;
    quint16 m_lastPort = 0;
};

} // namespace chatroom::client
