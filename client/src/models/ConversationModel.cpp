#include "ConversationModel.h"

namespace client {

ConversationModel::ConversationModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ConversationModel::~ConversationModel()
{
}

int ConversationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 0; // TODO
}

QVariant ConversationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    return QVariant(); // TODO
}

QHash<int, QByteArray> ConversationModel::roleNames() const
{
    return m_roles;
}

} // namespace client
