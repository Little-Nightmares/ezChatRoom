#pragma once

#include <QTcpServer>
#include <QMap>
#include <QSet>
#include <cstdint>

namespace chatroom::server {

class ClientSession;
class MessageRouter;

class TcpServer : public QTcpServer {
    Q_OBJECT
public:
    explicit TcpServer(QObject* parent = nullptr);
    ~TcpServer() override;

    bool start(quint16 port = 6667);
    void stop();

    ClientSession* getSession(uint64_t userId) const;
    void removeSession(uint64_t userId);
    void setRouter(MessageRouter* router);
    QMap<uint64_t, ClientSession*> getAuthenticatedSessions() const;
    const QSet<uint64_t>& onlineUserIds() const { return m_onlineUserIds; }

    // Move a session from pending (socket descriptor key) to authenticated (userId key)
    void moveToAuthenticated(qintptr socketDesc, uint64_t userId);

signals:
    void clientConnected(uint64_t userId);
    void clientDisconnected(uint64_t userId);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QMap<qintptr, ClientSession*> m_pendingSessions;
    QMap<uint64_t, ClientSession*> m_authenticatedSessions;
    QSet<uint64_t> m_onlineUserIds;
    MessageRouter* m_router = nullptr;
};

} // namespace chatroom::server
