#include "ConversationModel.h"

namespace client {

ConversationModel::ConversationModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_roles = {
        {Qt::UserRole + 1, "conversationId"},
        {Qt::UserRole + 2, "title"},
        {Qt::UserRole + 3, "lastMessage"},
        {Qt::UserRole + 4, "lastTime"},
        {Qt::UserRole + 5, "unreadCount"},
        {Qt::UserRole + 6, "avatarUrl"}
    };
}

ConversationModel::~ConversationModel()
{
}

int ConversationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_conversations.size();
}

QVariant ConversationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_conversations.size())
        return QVariant();

    const auto roleIt = m_roles.constFind(role);
    if (roleIt == m_roles.constEnd())
        return QVariant();

    return m_conversations.at(index.row()).value(QString::fromUtf8(roleIt.value()));
}

QHash<int, QByteArray> ConversationModel::roleNames() const
{
    return m_roles;
}

void ConversationModel::setConversations(const QVariantList &conversations)
{
    beginResetModel();
    m_conversations.clear();
    m_conversations.reserve(conversations.size());
    for (const QVariant &conversation : conversations)
        m_conversations.append(conversation.toMap());
    endResetModel();
}

void ConversationModel::appendConversation(const QVariantMap &conversation)
{
    const int row = m_conversations.size();
    beginInsertRows(QModelIndex(), row, row);
    m_conversations.append(conversation);
    endInsertRows();
}

void ConversationModel::clear()
{
    beginResetModel();
    m_conversations.clear();
    endResetModel();
}

} // namespace client
