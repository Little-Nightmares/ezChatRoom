#include "MessageModel.h"

namespace chatroom::client {

MessageModel::MessageModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int MessageModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_messages.size();
}

QVariant MessageModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_messages.size()) {
        return {};
    }

    const auto& msg = m_messages.at(index.row());

    switch (role) {
    case MessageIdRole:
        return static_cast<qint64>(msg.messageId);
    case SenderIdRole:
        return static_cast<qulonglong>(msg.senderId);
    case ContentRole:
        return msg.content;
    case TimestampRole:
        return msg.timestamp;
    case IsMineRole:
        return msg.isMine;
    case TypeRole:
        return static_cast<int>(msg.type);
    case StatusRole:
        return static_cast<int>(msg.status);
    case GroupIdRole:
        return static_cast<qulonglong>(msg.groupId);
    case SenderNicknameRole:
        return msg.senderNickname;
    default:
        return {};
    }
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    return {
        {MessageIdRole, "messageId"},
        {SenderIdRole, "senderId"},
        {ContentRole,  "content"},
        {TimestampRole,"timestamp"},
        {IsMineRole,   "isMine"},
        {TypeRole,     "type"},
        {StatusRole,   "status"},
        {GroupIdRole,  "groupId"},
        {SenderNicknameRole, "senderNickname"}
    };
}

void MessageModel::addMessage(const chatroom::models::ChatMessage& msg)
{
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append(msg);
    endInsertRows();
}

void MessageModel::clear()
{
    beginResetModel();
    m_messages.clear();
    endResetModel();
}

void MessageModel::loadMessages(const QVector<chatroom::models::ChatMessage>& messages)
{
    beginResetModel();
    m_messages.clear();
    m_messages.reserve(messages.size());
    for (const auto& msg : messages) {
        m_messages.append(msg);
    }
    endResetModel();
}

void MessageModel::updateMessageStatus(int row, chatroom::models::ChatMessage::MessageStatus status)
{
    if (row >= 0 && row < m_messages.size()) {
        m_messages[row].status = status;
        QModelIndex idx = index(row, 0);
        emit dataChanged(idx, idx, {StatusRole});
    }
}

void MessageModel::updateMessageContent(qint64 messageId, const QString& newContent)
{
    for (int i = 0; i < m_messages.size(); ++i) {
        if (m_messages[i].messageId == static_cast<uint64_t>(messageId)) {
            m_messages[i].content = newContent;
            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {ContentRole});
            return;
        }
    }
}

void MessageModel::markLastSendingAsDelivered()
{
    // Search from end to find the last message with Sending status
    for (int i = m_messages.size() - 1; i >= 0; --i) {
        if (m_messages[i].status == chatroom::models::ChatMessage::MessageStatus::Sending) {
            m_messages[i].status = chatroom::models::ChatMessage::MessageStatus::Delivered;
            QModelIndex idx = index(i, 0);
            emit dataChanged(idx, idx, {StatusRole});
            return;
        }
    }
}

} // namespace chatroom::client
