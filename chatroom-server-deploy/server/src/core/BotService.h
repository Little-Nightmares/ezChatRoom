#pragma once
#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QMap>
#include <cstdint>

namespace chatroom::server {
class ServerDatabase; class TcpServer;

class BotService : public QObject {
    Q_OBJECT
public:
    explicit BotService(ServerDatabase* db, TcpServer* server, QObject* parent = nullptr);
    void handleBotMention(uint64_t groupId, uint64_t senderId, const QString& question);

private slots:
    void onReplyFinished();

private:
    ServerDatabase* m_db;
    TcpServer* m_server;
    QNetworkAccessManager* m_nam = nullptr;
    QMap<QNetworkReply*, uint64_t> m_pendingGroups;
    QMap<uint64_t, uint64_t> m_pendingSenders;
    uint64_t m_nextReqId = 0;
    QString m_astrBotUrl = "http://127.0.0.1:6185/api/chat";
    uint64_t m_botUserId = 0;
};

}