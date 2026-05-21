#pragma once

#include <QAbstractListModel>
#include <QList>
#include <cstdint>
#include <QtQml/qqml.h>

namespace chatroom::client {

class GroupModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    struct GroupItemData {
        uint64_t    groupId = 0;
        QString     name;
        QString     avatar;
        uint64_t    ownerId = 0;
        int         memberCount = 0;
        bool        isDefault = false;
        QString     lastMessage;
        qint64      lastTime = 0;
        int         unreadCount = 0;
    };

    enum Roles {
        GroupIdRole = Qt::UserRole + 1,
        GroupNameRole,
        GroupAvatarRole,
        OwnerIdRole,
        MemberCountRole,
        IsDefaultRole,
        LastMessageRole,
        LastTimeRole,
        UnreadCountRole
    };
    Q_ENUM(Roles)

    explicit GroupModel(QObject* parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh(const QVector<GroupItemData>& groups);
    Q_INVOKABLE void updateGroup(uint64_t groupId, const QString& lastMessage,
                                  qint64 lastTime, int unreadDelta = 0);
    Q_INVOKABLE void removeGroup(uint64_t groupId);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void clearUnreadCount(uint64_t groupId);

    int findIndexByGroupId(uint64_t groupId) const;

private:
    QList<GroupItemData> m_groups;
};

} // namespace chatroom::client
