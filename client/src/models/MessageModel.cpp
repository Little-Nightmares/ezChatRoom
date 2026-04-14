#include "MessageModel.h"

namespace client {

MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

MessageModel::~MessageModel()
{
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 0; // TODO
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    return QVariant(); // TODO
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    return m_roles;
}

} // namespace client
