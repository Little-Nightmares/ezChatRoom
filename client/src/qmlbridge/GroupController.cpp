#include "GroupController.h"
#include "core/AppManager.h"
#include "core/SessionManager.h"
#include "network/TcpClient.h"
#include "network/MessageHandler.h"
#include "protocol/ChatProtocol.h"
#include "protocol/MessageTypes.h"
#include "models/GroupModel.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QFile>
#include <QDebug>
#include <QTimer>

namespace chatroom::client {

GroupController::GroupController(AppManager* appManager, GroupModel* groupModel, QObject* parent)
    : QObject(parent)
    , m_appManager(appManager)
    , m_groupModel(groupModel)
{
    auto* handler = m_appManager->messageHandler();

    uint8_t createRespType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::GroupCreateResponse);
    handler->registerHandler(createRespType,
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleGroupCreateResponse(flags, seq, body);
        });

    uint8_t listRespType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::GroupListResponse);
    handler->registerHandler(listRespType,
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleGroupListResponse(flags, seq, body);
        });

    uint8_t infoRespType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::GroupInfoResponse);
    handler->registerHandler(infoRespType,
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleGroupInfoResponse(flags, seq, body);
        });

    uint8_t memberAddNotifyType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::GroupMemberAddNotify);
    handler->registerHandler(memberAddNotifyType,
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleGroupMemberAddNotify(flags, seq, body);
        });

    uint8_t memberRemoveNotifyType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::GroupMemberRemoveNotify);
    handler->registerHandler(memberRemoveNotifyType,
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleGroupMemberRemoveNotify(flags, seq, body);
        });

    uint8_t updateNotifyType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::GroupUpdateNotify);
    handler->registerHandler(updateNotifyType,
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleGroupUpdateNotify(flags, seq, body);
        });

    uint8_t leaveNotifyType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::GroupLeaveNotify);
    handler->registerHandler(leaveNotifyType,
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleGroupLeaveNotify(flags, seq, body);
        });

    uint8_t dissolveNotifyType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::GroupDissolveNotify);
    handler->registerHandler(dissolveNotifyType,
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleGroupDissolveNotify(flags, seq, body);
        });

    uint8_t transferNotifyType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::GroupTransferOwnerNotify);
    handler->registerHandler(transferNotifyType,
        [this](uint8_t flags, uint32_t seq, const QByteArray& body) {
            handleGroupTransferOwnerNotify(flags, seq, body);
        });

    uint8_t announceType = static_cast<uint8_t>(
        chatroom::protocol::MessageType::GroupAnnouncementPush);
    handler->registerHandler(announceType,
        [this](uint8_t, uint32_t, const QByteArray& body) {
            QJsonDocument doc = QJsonDocument::fromJson(body);
            if (doc.isObject()) {
                m_announcement = doc.object()["content"].toString();
                emit announcementChanged();
            }
        });
}

quint64 GroupController::currentGroupId() const
{
    return m_currentGroupId;
}

QString GroupController::currentGroupName() const
{
    return m_currentGroupName;
}

bool GroupController::isGroupOwner() const
{
    return m_isGroupOwner;
}

int GroupController::currentGroupMemberCount() const
{
    return m_currentGroupMemberCount;
}

QStringList GroupController::currentGroupMemberNicknames() const
{
    return m_currentGroupMemberNicknames;
}

void GroupController::requestGroupList()
{
    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupListRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, QByteArray());
    qDebug() << "GroupController: group list request sent";
}

void GroupController::requestGroupInfo(quint64 groupId)
{
    QJsonObject bodyObj;
    bodyObj["groupId"] = static_cast<qint64>(groupId);
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupInfoRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "GroupController: group info request sent for groupId:" << groupId;
}

void GroupController::createGroup(const QString& name, const QVariantList& memberIds)
{
    if (name.isEmpty()) {
        emit groupCreateFailed("Group name cannot be empty");
        return;
    }

    QJsonObject bodyObj;
    bodyObj["name"] = name;

    QJsonArray membersArray;
    for (const auto& id : memberIds) {
        membersArray.append(static_cast<qint64>(id.toULongLong()));
    }
    bodyObj["memberIds"] = membersArray;

    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupCreateRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "GroupController: create group request sent, name:" << name;
}

void GroupController::addMembers(quint64 groupId, const QVariantList& memberIds)
{
    if (memberIds.isEmpty()) {
        qDebug() << "GroupController: addMembers called with empty member list";
        return;
    }

    QJsonObject bodyObj;
    bodyObj["groupId"] = static_cast<qint64>(groupId);

    QJsonArray membersArray;
    for (const auto& id : memberIds) {
        membersArray.append(static_cast<qint64>(id.toULongLong()));
    }
    bodyObj["memberIds"] = membersArray;

    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupMemberAddRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "GroupController: add members request sent for groupId:" << groupId;
}

void GroupController::removeMember(quint64 groupId, quint64 memberId)
{
    QJsonObject bodyObj;
    bodyObj["groupId"] = static_cast<qint64>(groupId);
    bodyObj["memberId"] = static_cast<qint64>(memberId);
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupMemberRemoveRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "GroupController: remove member request sent, groupId:" << groupId
             << "memberId:" << memberId;
}

void GroupController::updateGroupName(quint64 groupId, const QString& name)
{
    if (name.isEmpty()) {
        qDebug() << "GroupController: updateGroupName called with empty name";
        return;
    }

    QJsonObject bodyObj;
    bodyObj["groupId"] = static_cast<qint64>(groupId);
    bodyObj["name"] = name;
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupUpdateRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "GroupController: update group name request sent, groupId:" << groupId;
}

void GroupController::uploadGroupAvatar(quint64 groupId, const QString& imagePath)
{
    QFile file(imagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "GroupController: failed to open avatar file:" << imagePath;
        return;
    }

    QByteArray imageData = file.readAll();
    file.close();

    QJsonObject bodyObj;
    bodyObj["groupId"] = static_cast<qint64>(groupId);
    bodyObj["avatarData"] = QString(imageData.toBase64());
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupAvatarUploadReq);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "GroupController: group avatar upload request sent, groupId:" << groupId;
}

void GroupController::leaveGroup(quint64 groupId)
{
    QJsonObject bodyObj;
    bodyObj["groupId"] = static_cast<qint64>(groupId);
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupLeaveRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "GroupController: leave group request sent, groupId:" << groupId;
}

void GroupController::dissolveGroup(quint64 groupId)
{
    QJsonObject bodyObj;
    bodyObj["groupId"] = static_cast<qint64>(groupId);
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupDissolveRequest);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "GroupController: dissolve group request sent, groupId:" << groupId;
}

void GroupController::transferOwner(quint64 groupId, quint64 newOwnerId)
{
    QJsonObject bodyObj;
    bodyObj["groupId"] = static_cast<qint64>(groupId);
    bodyObj["newOwnerId"] = static_cast<qint64>(newOwnerId);
    QByteArray body = QJsonDocument(bodyObj).toJson(QJsonDocument::Compact);

    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupTransferOwnerReq);
    uint8_t flags = static_cast<uint8_t>(chatroom::protocol::MessageFlag::IsRequest);
    uint32_t seq = chatroom::protocol::ChatProtocol::nextSequence();

    m_appManager->tcpClient()->sendPacket(type, flags, seq, body);
    qDebug() << "GroupController: transfer owner request sent, groupId:" << groupId
             << "newOwnerId:" << newOwnerId;
}

void GroupController::setCurrentGroup(quint64 groupId)
{
    if (m_currentGroupId == groupId) {
        return;
    }
    m_currentGroupId = groupId;
    emit currentGroupIdChanged();

    // Request group info to populate other properties
    if (groupId != 0) {
        requestGroupInfo(groupId);
    }
}

void GroupController::clearCurrentGroup()
{
    if (m_currentGroupId == 0 && m_currentGroupName.isEmpty()
        && !m_isGroupOwner && m_currentGroupMemberCount == 0) {
        return;
    }

    m_currentGroupId = 0;
    m_currentGroupName.clear();
    m_isGroupOwner = false;
    m_currentGroupMemberCount = 0;
    m_currentGroupMemberNicknames.clear();

    emit currentGroupIdChanged();
    emit currentGroupNameChanged();
    emit isGroupOwnerChanged();
    emit currentGroupMemberCountChanged();
    emit currentGroupMemberNicknamesChanged();
}

void GroupController::handleGroupCreateResponse(uint8_t flags, uint32_t sequence,
                                                  const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit groupCreateFailed("Invalid server response");
        return;
    }

    QJsonObject obj = doc.object();
    bool success = obj["success"].toBool(false);

    if (!success) {
        QString reason = obj["reason"].toString("Group creation failed");
        emit groupCreateFailed(reason);
        return;
    }

    quint64 groupId = static_cast<quint64>(obj["groupId"].toInteger(0));
    QString name = obj["name"].toString();

    qDebug() << "GroupController: group created successfully, groupId:" << groupId
             << "name:" << name;
    emit groupCreated(groupId, name);
}

void GroupController::handleGroupListResponse(uint8_t flags, uint32_t sequence,
                                                const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    qDebug() << "GroupController: handleGroupListResponse called";

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "GroupController: failed to parse group list response";
        return;
    }

    QJsonObject obj = doc.object();
    QJsonArray groupsArray = obj["groups"].toArray();

    qDebug() << "GroupController: parsing" << groupsArray.size() << "groups";

    QVector<GroupModel::GroupItemData> groups;
    groups.reserve(groupsArray.size());
    for (const auto& item : groupsArray) {
        QJsonObject g = item.toObject();
        GroupModel::GroupItemData data;
        data.groupId = static_cast<uint64_t>(g["groupId"].toInteger(0));
        data.name = g["name"].toString();
        data.avatar = g["avatar"].toString();
        data.ownerId = static_cast<uint64_t>(g["ownerId"].toInteger(0));
        data.memberCount = g["memberCount"].toInt(0);
        data.isDefault = g["isDefault"].toBool(false);
        groups.append(data);
    }

    qDebug() << "GroupController: about to call refresh, m_groupModel =" << (void*)m_groupModel;

    if (m_groupModel) {
        m_groupModel->refresh(groups);
    }

    qDebug() << "GroupController: group list received," << groups.size() << "groups";
    emit groupListReceived();
}

QString GroupController::announcement() const { return m_announcement; }

void GroupController::requestAnnouncement(quint64 groupId)
{
    QJsonObject obj;
    obj["groupId"] = static_cast<qint64>(groupId);
    uint8_t type = static_cast<uint8_t>(chatroom::protocol::MessageType::GroupAnnouncementRequest);
    m_appManager->tcpClient()->sendPacket(type, 0, chatroom::protocol::ChatProtocol::nextSequence(),
        QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void GroupController::handleGroupInfoResponse(uint8_t flags, uint32_t sequence,
                                                const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "GroupController: failed to parse group info response";
        return;
    }

    QJsonObject obj = doc.object();
    quint64 groupId = static_cast<quint64>(obj["groupId"].toInteger(0));

    if (groupId == m_currentGroupId) {
        QString name = obj["name"].toString();
        quint64 ownerId = static_cast<quint64>(obj["ownerId"].toInteger(0));
        int memberCount = obj["memberCount"].toInt(0);

        m_currentGroupName = name;
        m_isGroupOwner = (ownerId == m_appManager->sessionManager()->userId());
        m_currentGroupMemberCount = memberCount;

        // Extract member nicknames for @mention autocomplete
        QStringList nicknames;
        QJsonArray members = obj["members"].toArray();
        for (const auto& m : members) {
            QJsonObject member = m.toObject();
            QString nickname = member["nickname"].toString();
            if (nickname.isEmpty()) {
                nickname = member["username"].toString();
            }
            if (!nickname.isEmpty()) {
                nicknames.append(nickname);
            }
        }
        // Also include the current user
        QString currentNick = m_appManager->sessionManager()->nickname();
        if (!currentNick.isEmpty() && !nicknames.contains(currentNick)) {
            nicknames.append(currentNick);
        }
        if (nicknames != m_currentGroupMemberNicknames) {
            m_currentGroupMemberNicknames = nicknames;
            emit currentGroupMemberNicknamesChanged();
        }

        emit currentGroupNameChanged();
        emit isGroupOwnerChanged();
        emit currentGroupMemberCountChanged();
    }

    qDebug() << "GroupController: group info received, groupId:" << groupId;
    emit groupInfoReceived(groupId);
}

void GroupController::handleGroupMemberAddNotify(uint8_t flags, uint32_t sequence,
                                                   const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "GroupController: failed to parse member add notify";
        return;
    }

    QJsonObject obj = doc.object();
    quint64 groupId = static_cast<quint64>(obj["groupId"].toInteger(0));

    if (groupId == m_currentGroupId) {
        int memberCount = obj["memberCount"].toInt(m_currentGroupMemberCount + 1);
        if (memberCount != m_currentGroupMemberCount) {
            m_currentGroupMemberCount = memberCount;
            emit currentGroupMemberCountChanged();
        }
    }

    qDebug() << "GroupController: member added to group:" << groupId;
    emit memberAdded(groupId);
}

void GroupController::handleGroupMemberRemoveNotify(uint8_t flags, uint32_t sequence,
                                                      const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "GroupController: failed to parse member remove notify";
        return;
    }

    QJsonObject obj = doc.object();
    quint64 groupId = static_cast<quint64>(obj["groupId"].toInteger(0));

    if (groupId == m_currentGroupId) {
        int memberCount = obj["memberCount"].toInt(m_currentGroupMemberCount - 1);
        if (memberCount != m_currentGroupMemberCount) {
            m_currentGroupMemberCount = memberCount;
            emit currentGroupMemberCountChanged();
        }
    }

    qDebug() << "GroupController: member removed from group:" << groupId;
    emit memberRemoved(groupId);
}

void GroupController::handleGroupUpdateNotify(uint8_t flags, uint32_t sequence,
                                                const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "GroupController: failed to parse group update notify";
        return;
    }

    QJsonObject obj = doc.object();
    quint64 groupId = static_cast<quint64>(obj["groupId"].toInteger(0));

    if (groupId == m_currentGroupId) {
        QString name = obj["name"].toString();
        if (!name.isEmpty() && name != m_currentGroupName) {
            m_currentGroupName = name;
            emit currentGroupNameChanged();
        }
    }

    qDebug() << "GroupController: group updated, groupId:" << groupId;
    emit groupUpdated(groupId);
}

void GroupController::handleGroupLeaveNotify(uint8_t flags, uint32_t sequence,
                                               const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "GroupController: failed to parse group leave notify";
        return;
    }

    QJsonObject obj = doc.object();
    quint64 groupId = static_cast<quint64>(obj["groupId"].toInteger(0));

    if (groupId == m_currentGroupId) {
        clearCurrentGroup();
    }

    qDebug() << "GroupController: member left group:" << groupId;
    emit groupLeft(groupId);
}

void GroupController::handleGroupDissolveNotify(uint8_t flags, uint32_t sequence,
                                                  const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "GroupController: failed to parse group dissolve notify";
        return;
    }

    QJsonObject obj = doc.object();
    quint64 groupId = static_cast<quint64>(obj["groupId"].toInteger(0));

    if (groupId == m_currentGroupId) {
        clearCurrentGroup();
    }

    qDebug() << "GroupController: group dissolved:" << groupId;
    emit groupDissolved(groupId);
}

void GroupController::handleGroupTransferOwnerNotify(uint8_t flags, uint32_t sequence,
                                                       const QByteArray& body)
{
    Q_UNUSED(flags)
    Q_UNUSED(sequence)

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "GroupController: failed to parse transfer owner notify";
        return;
    }

    QJsonObject obj = doc.object();
    quint64 groupId = static_cast<quint64>(obj["groupId"].toInteger(0));
    quint64 newOwnerId = static_cast<quint64>(obj["newOwnerId"].toInteger(0));

    if (groupId == m_currentGroupId) {
        bool newIsOwner = (newOwnerId == m_appManager->sessionManager()->userId());
        if (newIsOwner != m_isGroupOwner) {
            m_isGroupOwner = newIsOwner;
            emit isGroupOwnerChanged();
        }
    }

    qDebug() << "GroupController: owner transferred for group:" << groupId
             << "newOwnerId:" << newOwnerId;
    emit ownerTransferred(groupId);
}

} // namespace chatroom::client
