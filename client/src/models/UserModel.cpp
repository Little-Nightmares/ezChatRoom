#include "UserModel.h"

namespace client {

UserModel::UserModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

UserModel::~UserModel()
{
}

int UserModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 0; // TODO
}

QVariant UserModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    return QVariant(); // TODO
}

QHash<int, QByteArray> UserModel::roleNames() const
{
    return m_roles;
}

} // namespace client
