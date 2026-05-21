#include "UserModel.h"

namespace client {

UserModel::UserModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_roles = {
        {Qt::UserRole + 1, "userId"},
        {Qt::UserRole + 2, "username"},
        {Qt::UserRole + 3, "displayName"},
        {Qt::UserRole + 4, "avatarUrl"},
        {Qt::UserRole + 5, "online"}
    };
}

UserModel::~UserModel()
{
}

int UserModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_users.size();
}

QVariant UserModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_users.size())
        return QVariant();

    const auto roleIt = m_roles.constFind(role);
    if (roleIt == m_roles.constEnd())
        return QVariant();

    return m_users.at(index.row()).value(QString::fromUtf8(roleIt.value()));
}

QHash<int, QByteArray> UserModel::roleNames() const
{
    return m_roles;
}

void UserModel::setUsers(const QVariantList &users)
{
    beginResetModel();
    m_users.clear();
    m_users.reserve(users.size());
    for (const QVariant &user : users)
        m_users.append(user.toMap());
    endResetModel();
}

void UserModel::appendUser(const QVariantMap &user)
{
    const int row = m_users.size();
    beginInsertRows(QModelIndex(), row, row);
    m_users.append(user);
    endInsertRows();
}

void UserModel::clear()
{
    beginResetModel();
    m_users.clear();
    endResetModel();
}

} // namespace client
