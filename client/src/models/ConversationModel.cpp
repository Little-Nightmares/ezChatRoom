#include "ConversationModel.h"

namespace chatroom::client {

ConversationModel::ConversationModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ConversationModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_conversations.size();
}

QVariant ConversationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_conversations.size()) {
        return {};
    }

    const auto& conv = m_conversations.at(index.row());

    switch (role) {
    case FriendIdRole:
        return static_cast<qulonglong>(conv.friendId);
    case NicknameRole:
        return conv.nickname;
    case AvatarRole:
        return conv.avatar;
    case LastMessageRole:
        return conv.lastMessage;
    case LastTimeRole:
        return conv.lastTime;
    case UnreadCountRole:
        return conv.unreadCount;
    case GroupIdRole:
        return static_cast<qulonglong>(conv.groupId);
    case IsGroupRole:
        return conv.isGroup;
    default:
        return {};
    }
}

QHash<int, QByteArray> ConversationModel::roleNames() const
{
    return {
        {FriendIdRole,    "friendId"},
        {NicknameRole,    "nickname"},
        {AvatarRole,      "avatar"},
        {LastMessageRole, "lastMessage"},
        {LastTimeRole,    "lastTime"},
        {UnreadCountRole, "unreadCount"},
        {GroupIdRole,     "groupId"},
        {IsGroupRole,     "isGroup"}
    };
}

void ConversationModel::refresh(const QVector<ConversationData>& conversations)
{
    beginResetModel();
    m_conversations.clear();
    m_conversations.reserve(conversations.size());
    for (const auto& conv : conversations) {
        m_conversations.append(conv);
    }
    endResetModel();
}

void ConversationModel::updateConversation(uint64_t friendId, const QString& lastMessage,
                                            const QDateTime& lastTime, int unreadDelta)
{
    int idx = findIndexByFriendId(friendId);
    if (idx >= 0) {
        // Update existing conversation and move to top
        m_conversations[idx].lastMessage = lastMessage;
        m_conversations[idx].lastTime = lastTime;
        m_conversations[idx].unreadCount += unreadDelta;
        if (m_conversations[idx].unreadCount < 0) {
            m_conversations[idx].unreadCount = 0;
        }

        if (idx == 0) {
            // Already at top, just notify data change
            emit dataChanged(index(0), index(0));
        } else {
            // Remove from old position, insert at top
            beginMoveRows(QModelIndex(), idx, idx, QModelIndex(), 0);
            ConversationData data = m_conversations.takeAt(idx);
            m_conversations.prepend(data);
            endMoveRows();
        }
    } else {
        // New conversation
        beginInsertRows(QModelIndex(), 0, 0);
        ConversationData data;
        data.friendId = friendId;
        data.lastMessage = lastMessage;
        data.lastTime = lastTime;
        data.unreadCount = (unreadDelta > 0) ? unreadDelta : 1;
        m_conversations.prepend(data);
        endInsertRows();
    }
}

void ConversationModel::clear()
{
    beginResetModel();
    m_conversations.clear();
    endResetModel();
}

void ConversationModel::clearUnreadCount(uint64_t friendId)
{
    int idx = findIndexByFriendId(friendId);
    if (idx >= 0 && m_conversations[idx].unreadCount > 0) {
        m_conversations[idx].unreadCount = 0;
        QModelIndex modelIdx = index(idx, 0);
        emit dataChanged(modelIdx, modelIdx, {UnreadCountRole});
    }
}

void ConversationModel::updateGroupConversation(uint64_t groupId, const QString& nickname,
                                                   const QString& avatar, const QString& lastMessage,
                                                   const QDateTime& lastTime, int unreadDelta)
{
    int idx = findIndexByGroupId(groupId);
    if (idx >= 0) {
        // Update existing group conversation and move to top
        m_conversations[idx].nickname = nickname;
        m_conversations[idx].avatar = avatar;
        m_conversations[idx].lastMessage = lastMessage;
        m_conversations[idx].lastTime = lastTime;
        m_conversations[idx].unreadCount += unreadDelta;
        if (m_conversations[idx].unreadCount < 0) {
            m_conversations[idx].unreadCount = 0;
        }

        if (idx == 0) {
            emit dataChanged(index(0), index(0));
        } else {
            beginMoveRows(QModelIndex(), idx, idx, QModelIndex(), 0);
            ConversationData data = m_conversations.takeAt(idx);
            m_conversations.prepend(data);
            endMoveRows();
        }
    } else {
        // New group conversation
        beginInsertRows(QModelIndex(), 0, 0);
        ConversationData data;
        data.groupId = groupId;
        data.isGroup = true;
        data.nickname = nickname;
        data.avatar = avatar;
        data.lastMessage = lastMessage;
        data.lastTime = lastTime;
        data.unreadCount = (unreadDelta > 0) ? unreadDelta : 1;
        m_conversations.prepend(data);
        endInsertRows();
    }
}

void ConversationModel::removeGroupConversation(uint64_t groupId)
{
    int idx = findIndexByGroupId(groupId);
    if (idx >= 0) {
        beginRemoveRows(QModelIndex(), idx, idx);
        m_conversations.removeAt(idx);
        endRemoveRows();
    }
}

int ConversationModel::findIndexByGroupId(uint64_t groupId) const
{
    for (int i = 0; i < m_conversations.size(); ++i) {
        if (m_conversations.at(i).isGroup && m_conversations.at(i).groupId == groupId) {
            return i;
        }
    }
    return -1;
}

int ConversationModel::findIndexByFriendId(uint64_t friendId) const
{
    for (int i = 0; i < m_conversations.size(); ++i) {
        if (m_conversations.at(i).friendId == friendId) {
            return i;
        }
    }
    return -1;
}

} // namespace chatroom::client
