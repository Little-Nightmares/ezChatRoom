#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QtQml/qqml.h>
#include "models/FriendRequest.h"

namespace chatroom::client {

class FriendRequestModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        RequestIdRole = Qt::UserRole + 1,
        FromUserIdRole,
        FromUsernameRole,
        MessageRole,
        StatusRole
    };
    Q_ENUM(Roles)

    explicit FriendRequestModel(QObject* parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setRequests(const QVector<chatroom::models::FriendRequest>& requests);
    Q_INVOKABLE void clear();

private:
    QList<chatroom::models::FriendRequest> m_requests;
};

} // namespace chatroom::client
