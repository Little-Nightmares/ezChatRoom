#include "UserModel.h"

namespace chatroom::client {

UserModel::UserModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int UserModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_users.size();
}

QVariant UserModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_users.size()) {
        return {};
    }

    const auto& user = m_users.at(index.row());

    switch (role) {
    case UserIdRole:
        return static_cast<qulonglong>(user.userId);
    case UsernameRole:
        return user.username;
    case NicknameRole:
        return user.nickname;
    case AvatarRole:
        return user.avatar;
    case IsOnlineRole:
        return user.isOnline;
    default:
        return {};
    }
}

QHash<int, QByteArray> UserModel::roleNames() const
{
    return {
        {UserIdRole,   "userId"},
        {UsernameRole, "username"},
        {NicknameRole, "nickname"},
        {AvatarRole,   "avatar"},
        {IsOnlineRole, "isOnline"}
    };
}

void UserModel::setUsers(const QVector<chatroom::models::UserInfo>& users)
{
    beginResetModel();
    m_users.clear();
    m_users.reserve(users.size());
    for (const auto& user : users) {
        m_users.append(user);
    }
    endResetModel();
}

void UserModel::clear()
{
    beginResetModel();
    m_users.clear();
    endResetModel();
}

void UserModel::updateOnlineStatus(uint64_t userId, bool online)
{
    for (int i = 0; i < m_users.size(); ++i) {
        if (m_users[i].userId == userId) {
            m_users[i].isOnline = online;
            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {IsOnlineRole});
            return;
        }
    }
}

bool UserModel::isOnline(uint64_t userId) const
{
    for (const auto& u : m_users) {
        if (u.userId == userId) {
            return u.isOnline;
        }
    }
    return false;
}

} // namespace chatroom::client
