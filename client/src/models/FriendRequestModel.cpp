#include "FriendRequestModel.h"

namespace chatroom::client {

FriendRequestModel::FriendRequestModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int FriendRequestModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_requests.size();
}

QVariant FriendRequestModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_requests.size()) {
        return {};
    }

    const auto& req = m_requests.at(index.row());

    switch (role) {
    case RequestIdRole:
        return static_cast<qulonglong>(req.requestId);
    case FromUserIdRole:
        return static_cast<qulonglong>(req.fromUserId);
    case FromUsernameRole:
        return req.fromUsername;
    case MessageRole:
        return req.message;
    case StatusRole:
        return static_cast<int>(req.status);
    default:
        return {};
    }
}

QHash<int, QByteArray> FriendRequestModel::roleNames() const
{
    return {
        {RequestIdRole,    "requestId"},
        {FromUserIdRole,   "fromUserId"},
        {FromUsernameRole, "fromUsername"},
        {MessageRole,      "message"},
        {StatusRole,       "status"}
    };
}

void FriendRequestModel::setRequests(const QVector<chatroom::models::FriendRequest>& requests)
{
    beginResetModel();
    m_requests.clear();
    m_requests.reserve(requests.size());
    for (const auto& req : requests) {
        m_requests.append(req);
    }
    endResetModel();
}

void FriendRequestModel::clear()
{
    beginResetModel();
    m_requests.clear();
    endResetModel();
}

} // namespace chatroom::client
