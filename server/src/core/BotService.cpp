#include "BotService.h"
#include "database/ServerDatabase.h"
#include "core/TcpServer.h"
#include "core/ClientSession.h"
#include "protocol/ChatProtocol.h"
#include "protocol/MessageTypes.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QDebug>

namespace chatroom::server {

BotService::BotService(ServerDatabase* db, TcpServer* server, QObject* parent)
    : QObject(parent), m_db(db), m_server(server) {
    m_nam = new QNetworkAccessManager(this);
    connect(m_nam, &QNetworkAccessManager::finished, this, &BotService::onReplyFinished);
    m_botUserId = m_db->getOrCreateBotUser();
    qDebug() << "BotService: bot userId =" << m_botUserId;
}

void BotService::handleBotMention(uint64_t groupId, uint64_t senderId, const QString& question) {
    QString senderName = "User";
    auto userInfo = m_db->getUserInfo(senderId);
    if (!userInfo.nickname.isEmpty()) senderName = userInfo.nickname;
    else if (!userInfo.username.isEmpty()) senderName = userInfo.username;

    QString apiUrl = m_db->loadSetting("astrbot_url", m_astrBotUrl);
    if (apiUrl.isEmpty()) return;

    QUrl burl(apiUrl);
    QNetworkRequest req(burl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["type"] = "group_message";
    body["sender_name"] = senderName;
    body["message"] = question;
    body["is_group"] = true;

    QNetworkReply* reply = m_nam->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    uint64_t reqId = m_nextReqId++;
    m_pendingGroups[reply] = groupId;
    m_pendingSenders[reqId] = senderId;
    reply->setProperty("reqId", static_cast<qulonglong>(reqId));
}

void BotService::onReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    uint64_t groupId = m_pendingGroups.take(reply);
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Bot: AstrBot error:" << reply->errorString();
        return;
    }

    QJsonDocument resp = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = resp.object();
    QString answer;

    if (obj.contains("reply")) answer = obj["reply"].toString();
    else if (obj.contains("response")) answer = obj["response"].toString();
    else if (obj.contains("content")) answer = obj["content"].toString();
    else if (obj.contains("message")) answer = obj["message"].toString();
    else answer = QString::fromUtf8("Bot: ") + QString::fromUtf8(QJsonDocument(resp).toJson(QJsonDocument::Compact));

    if (answer.isEmpty() || groupId == 0) return;

    qint64 now = QDateTime::currentSecsSinceEpoch();
    m_db->storeGroupMessage(groupId, m_botUserId, answer, 0, now);

    QJsonObject push;
    push["groupId"] = static_cast<qint64>(groupId);
    push["senderId"] = static_cast<qint64>(m_botUserId);
    push["senderNickname"] = "Bot";
    push["content"] = answer;
    push["timestamp"] = now;
    push["type"] = 3;

    QByteArray data = QJsonDocument(push).toJson(QJsonDocument::Compact);
    for (auto uid : m_db->getGroupMemberIds(groupId)) {
        auto* s = m_server->getSession(uid);
        if (s) s->sendPacket(chatroom::protocol::MessageType::GroupMessageNotify, 0,
                             chatroom::protocol::ChatProtocol::nextSequence(), data);
    }
}

}