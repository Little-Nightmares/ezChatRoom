#include "FriendRequestModel.h"

namespace client {

FriendRequestModel::FriendRequestModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_roles = {
        {Qt::UserRole + 1, "requestId"},
        {Qt::UserRole + 2, "userId"},
        {Qt::UserRole + 3, "username"},
        {Qt::UserRole + 4, "message"},
        {Qt::UserRole + 5, "status"},
        {Qt::UserRole + 6, "createdAt"}
    };
}

FriendRequestModel::~FriendRequestModel()
{
}

int FriendRequestModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_requests.size();
}

QVariant FriendRequestModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_requests.size())
        return QVariant();

    const auto roleIt = m_roles.constFind(role);
    if (roleIt == m_roles.constEnd())
        return QVariant();

    return m_requests.at(index.row()).value(QString::fromUtf8(roleIt.value()));
}

QHash<int, QByteArray> FriendRequestModel::roleNames() const
{
    return m_roles;
}

void FriendRequestModel::setRequests(const QVariantList &requests)
{
    beginResetModel();
    m_requests.clear();
    m_requests.reserve(requests.size());
    for (const QVariant &request : requests)
        m_requests.append(request.toMap());
    endResetModel();
}

void FriendRequestModel::appendRequest(const QVariantMap &request)
{
    const int row = m_requests.size();
    beginInsertRows(QModelIndex(), row, row);
    m_requests.append(request);
    endInsertRows();
}

void FriendRequestModel::clear()
{
    beginResetModel();
    m_requests.clear();
    endResetModel();
}

} // namespace client
