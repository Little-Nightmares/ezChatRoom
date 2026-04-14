#include "FriendRequestModel.h"

namespace client {

FriendRequestModel::FriendRequestModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

FriendRequestModel::~FriendRequestModel()
{
}

int FriendRequestModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 0; // TODO
}

QVariant FriendRequestModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    return QVariant(); // TODO
}

QHash<int, QByteArray> FriendRequestModel::roleNames() const
{
    return m_roles;
}

} // namespace client
