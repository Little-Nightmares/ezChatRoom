#include "GroupModel.h"

namespace chatroom::client {

GroupModel::GroupModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int GroupModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_groups.size();
}

QVariant GroupModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_groups.size()) {
        return {};
    }

    const auto& group = m_groups.at(index.row());

    switch (role) {
    case GroupIdRole:
        return static_cast<qulonglong>(group.groupId);
    case GroupNameRole:
        return group.name;
    case GroupAvatarRole:
        return group.avatar;
    case OwnerIdRole:
        return static_cast<qulonglong>(group.ownerId);
    case MemberCountRole:
        return group.memberCount;
    case IsDefaultRole:
        return group.isDefault;
    case LastMessageRole:
        return group.lastMessage;
    case LastTimeRole:
        return group.lastTime;
    case UnreadCountRole:
        return group.unreadCount;
    default:
        return {};
    }
}

QHash<int, QByteArray> GroupModel::roleNames() const
{
    return {
        {GroupIdRole,        "groupId"},
        {GroupNameRole,      "groupName"},
        {GroupAvatarRole,    "groupAvatar"},
        {OwnerIdRole,        "ownerId"},
        {MemberCountRole,    "memberCount"},
        {IsDefaultRole,      "isDefault"},
        {LastMessageRole,    "lastMessage"},
        {LastTimeRole,       "lastTime"},
        {UnreadCountRole,    "unreadCount"}
    };
}

void GroupModel::refresh(const QVector<GroupItemData>& groups)
{
    beginResetModel();
    m_groups.clear();
    endResetModel();

    if (!groups.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, groups.size() - 1);
        m_groups = groups;
        endInsertRows();
    }
}

void GroupModel::updateGroup(uint64_t groupId, const QString& lastMessage,
                              qint64 lastTime, int unreadDelta)
{
    int idx = findIndexByGroupId(groupId);
    if (idx >= 0) {
        // Update existing group and move to top
        m_groups[idx].lastMessage = lastMessage;
        m_groups[idx].lastTime = lastTime;
        m_groups[idx].unreadCount += unreadDelta;
        if (m_groups[idx].unreadCount < 0) {
            m_groups[idx].unreadCount = 0;
        }

        if (idx == 0) {
            // Already at top, just notify data change
            emit dataChanged(index(0), index(0));
        } else {
            // Remove from old position, insert at top
            beginMoveRows(QModelIndex(), idx, idx, QModelIndex(), 0);
            GroupItemData data = m_groups.takeAt(idx);
            m_groups.prepend(data);
            endMoveRows();
        }
    } else {
        // New group
        beginInsertRows(QModelIndex(), 0, 0);
        GroupItemData data;
        data.groupId = groupId;
        data.lastMessage = lastMessage;
        data.lastTime = lastTime;
        data.unreadCount = (unreadDelta > 0) ? unreadDelta : 1;
        m_groups.prepend(data);
        endInsertRows();
    }
}

void GroupModel::removeGroup(uint64_t groupId)
{
    int idx = findIndexByGroupId(groupId);
    if (idx >= 0) {
        beginRemoveRows(QModelIndex(), idx, idx);
        m_groups.removeAt(idx);
        endRemoveRows();
    }
}

void GroupModel::clear()
{
    beginResetModel();
    m_groups.clear();
    endResetModel();
}

void GroupModel::clearUnreadCount(uint64_t groupId)
{
    int idx = findIndexByGroupId(groupId);
    if (idx >= 0 && m_groups[idx].unreadCount > 0) {
        m_groups[idx].unreadCount = 0;
        QModelIndex modelIdx = index(idx, 0);
        emit dataChanged(modelIdx, modelIdx, {UnreadCountRole});
    }
}

int GroupModel::findIndexByGroupId(uint64_t groupId) const
{
    for (int i = 0; i < m_groups.size(); ++i) {
        if (m_groups.at(i).groupId == groupId) {
            return i;
        }
    }
    return -1;
}

} // namespace chatroom::client
