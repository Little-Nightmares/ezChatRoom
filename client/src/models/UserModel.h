#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QtQml/qqml.h>
#include "models/UserInfo.h"

namespace chatroom::client {

class UserModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        UserIdRole = Qt::UserRole + 1,
        UsernameRole,
        NicknameRole,
        AvatarRole,
        IsOnlineRole
    };
    Q_ENUM(Roles)

    explicit UserModel(QObject* parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setUsers(const QVector<chatroom::models::UserInfo>& users);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void updateOnlineStatus(uint64_t userId, bool online);
    Q_INVOKABLE bool isOnline(uint64_t userId) const;

private:
    QList<chatroom::models::UserInfo> m_users;
};

} // namespace chatroom::client
