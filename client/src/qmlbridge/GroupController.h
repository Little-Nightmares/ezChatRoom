#pragma once

#include <QObject>
#include <QString>
#include <cstdint>
#include <QtQml/qqml.h>

namespace chatroom::client {

class AppManager;
class GroupModel;

class GroupController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("GroupController is created by AppCore and exposed to QML as a context property")
    Q_PROPERTY(quint64 currentGroupId READ currentGroupId
               NOTIFY currentGroupIdChanged)
    Q_PROPERTY(QString currentGroupName READ currentGroupName
               NOTIFY currentGroupNameChanged)
    Q_PROPERTY(bool isGroupOwner READ isGroupOwner
               NOTIFY isGroupOwnerChanged)
    Q_PROPERTY(int currentGroupMemberCount READ currentGroupMemberCount
               NOTIFY currentGroupMemberCountChanged)
    Q_PROPERTY(QStringList currentGroupMemberNicknames READ currentGroupMemberNicknames
               NOTIFY currentGroupMemberNicknamesChanged)
    Q_PROPERTY(QString announcement READ announcement NOTIFY announcementChanged)

public:
    explicit GroupController(AppManager* appManager, GroupModel* groupModel, QObject* parent = nullptr);

    quint64 currentGroupId() const;
    QString currentGroupName() const;
    bool isGroupOwner() const;
    int currentGroupMemberCount() const;

    Q_INVOKABLE void requestGroupList();
    Q_INVOKABLE void requestGroupInfo(quint64 groupId);
    Q_INVOKABLE void createGroup(const QString& name, const QVariantList& memberIds);
    Q_INVOKABLE void addMembers(quint64 groupId, const QVariantList& memberIds);
    Q_INVOKABLE void removeMember(quint64 groupId, quint64 memberId);
    Q_INVOKABLE void updateGroupName(quint64 groupId, const QString& name);
    Q_INVOKABLE void uploadGroupAvatar(quint64 groupId, const QString& imagePath);
    Q_INVOKABLE void leaveGroup(quint64 groupId);
    Q_INVOKABLE void dissolveGroup(quint64 groupId);
    Q_INVOKABLE void transferOwner(quint64 groupId, quint64 newOwnerId);
    Q_INVOKABLE void setCurrentGroup(quint64 groupId);
    Q_INVOKABLE void clearCurrentGroup();
    Q_INVOKABLE void requestAnnouncement(quint64 groupId);
    QString announcement() const;
    QStringList currentGroupMemberNicknames() const;

signals:
    void groupCreated(quint64 groupId, const QString& name);
    void groupCreateFailed(const QString& error);
    void groupListReceived();
    void groupInfoReceived(quint64 groupId);
    void memberAdded(quint64 groupId);
    void memberRemoved(quint64 groupId);
    void groupUpdated(quint64 groupId);
    void groupLeft(quint64 groupId);
    void groupDissolved(quint64 groupId);
    void ownerTransferred(quint64 groupId);
    void currentGroupIdChanged();
    void currentGroupNameChanged();
    void isGroupOwnerChanged();
    void currentGroupMemberCountChanged();
    void currentGroupMemberNicknamesChanged();
    void announcementChanged();

private:
    void handleGroupCreateResponse(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleGroupListResponse(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleGroupInfoResponse(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleGroupMemberAddNotify(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleGroupMemberRemoveNotify(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleGroupUpdateNotify(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleGroupLeaveNotify(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleGroupDissolveNotify(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleGroupTransferOwnerNotify(uint8_t flags, uint32_t sequence, const QByteArray& body);

    AppManager* m_appManager = nullptr;
    GroupModel* m_groupModel = nullptr;
    quint64 m_currentGroupId = 0;
    QString m_currentGroupName;
    bool m_isGroupOwner = false;
    int m_currentGroupMemberCount = 0;
    QStringList m_currentGroupMemberNicknames;
    QString m_announcement;
};

} // namespace chatroom::client
