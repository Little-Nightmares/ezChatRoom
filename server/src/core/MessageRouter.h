#pragma once

#include <QObject>
#include <cstdint>
#include <QByteArray>
#include "protocol/MessageTypes.h"

using chatroom::protocol::MessageType;

namespace chatroom::server {

class ClientSession;
class ServerDatabase;
class TcpServer;
class BotService;

class MessageRouter : public QObject {
    Q_OBJECT
public:
    explicit MessageRouter(ServerDatabase* db, TcpServer* server, QObject* parent = nullptr);

    void registerSession(ClientSession* session);
    void setBotService(BotService* service) { m_botService = service; }
    void setAdminPort(quint16 port) { m_adminPort = port; }

public slots:
    void onPacketReceived(MessageType type, uint8_t flags, uint32_t sequence,
                          const QByteArray& body, ClientSession* session);

private:
    // Message type handlers
    void handleLogin(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleRegister(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleLogout(ClientSession* session, uint32_t seq);
    void handleHeartbeat(ClientSession* session, uint32_t seq);
    void handleChatMessage(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleFriendSearch(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleFriendAdd(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleFriendAccept(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleFriendReject(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleFriendDelete(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleFriendList(ClientSession* session, uint32_t seq);
    void handleFriendRequestList(ClientSession* session, uint32_t seq);
    void handleKeyExchange(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleMessageRecall(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleAvatarUpload(ClientSession* session, uint32_t seq, const QByteArray& body);

    // File transfer handlers
    void handleFileUpload(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleFileDownload(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleFileChunk(ClientSession* session, uint32_t seq, const QByteArray& body);

    // Message search
    void handleMessageSearch(ClientSession* session, uint32_t seq, const QByteArray& body);

    // Message reply
    void handleMessageReply(ClientSession* session, uint32_t seq, const QByteArray& body);

    // Group enhancement handlers
    void handleGroupAnnouncement(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleGroupMessageRead(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleGroupKeyRequest(ClientSession* session, uint32_t seq, const QByteArray& body);

    // Group chat handlers
    void handleGroupCreate(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleGroupList(ClientSession* session, uint32_t seq);
    void handleGroupInfo(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleGroupMessage(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleGroupMemberAdd(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleGroupMemberRemove(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleGroupUpdate(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleGroupLeave(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleGroupDissolve(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleGroupAvatarUpload(ClientSession* session, uint32_t seq, const QByteArray& body);
    void handleGroupTransferOwner(ClientSession* session, uint32_t seq, const QByteArray& body);

    // Helper: send response packet
    void sendResponse(ClientSession* session, MessageType respType,
                      uint32_t seq, const QByteArray& body);

    // Helper: send error response packet
    void sendError(ClientSession* session, uint32_t seq, const QString& message);

    ServerDatabase* m_db = nullptr;
    TcpServer* m_server = nullptr;
    BotService* m_botService = nullptr;
    quint16 m_adminPort = 0;

    // Convert relative avatar path to full HTTP URL
    QString avatarUrl(const QString& relativePath) const;
};

} // namespace chatroom::server

