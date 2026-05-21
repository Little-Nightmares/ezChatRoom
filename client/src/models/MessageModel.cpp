#include "MessageModel.h"

namespace client {

MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_roles = {
        {Qt::UserRole + 1, "messageId"},
        {Qt::UserRole + 2, "senderId"},
        {Qt::UserRole + 3, "receiverId"},
        {Qt::UserRole + 4, "content"},
        {Qt::UserRole + 5, "timestamp"},
        {Qt::UserRole + 6, "mine"},
        {Qt::UserRole + 7, "status"}
    };
}

MessageModel::~MessageModel()
{
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_messages.size();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_messages.size())
        return QVariant();

    const auto roleIt = m_roles.constFind(role);
    if (roleIt == m_roles.constEnd())
        return QVariant();

    return m_messages.at(index.row()).value(QString::fromUtf8(roleIt.value()));
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    return m_roles;
}

void MessageModel::setMessages(const QVariantList &messages)
{
    beginResetModel();
    m_messages.clear();
    m_messages.reserve(messages.size());
    for (const QVariant &message : messages)
        m_messages.append(message.toMap());
    endResetModel();
}

void MessageModel::appendMessage(const QVariantMap &message)
{
    const int row = m_messages.size();
    beginInsertRows(QModelIndex(), row, row);
    m_messages.append(message);
    endInsertRows();
}

void MessageModel::clear()
{
    beginResetModel();
    m_messages.clear();
    endResetModel();
}

} // namespace client
