#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QDateTime>
#include <cstdint>
#include <QtQml/qqml.h>

namespace chatroom::client {

class ConversationModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    struct ConversationData {
        uint64_t    friendId = 0;
        QString     nickname;
        QString     avatar;
        QString     lastMessage;
        QDateTime   lastTime;
        int         unreadCount = 0;
        uint64_t    groupId = 0;
        bool        isGroup = false;
    };

    enum Roles {
        FriendIdRole = Qt::UserRole + 1,
        NicknameRole,
        AvatarRole,
        LastMessageRole,
        LastTimeRole,
        UnreadCountRole,
        GroupIdRole,
        IsGroupRole
    };
    Q_ENUM(Roles)

    explicit ConversationModel(QObject* parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh(const QVector<ConversationData>& conversations);
    Q_INVOKABLE void updateConversation(uint64_t friendId, const QString& lastMessage,
                                         const QDateTime& lastTime, int unreadDelta = 0);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void clearUnreadCount(uint64_t friendId);

    Q_INVOKABLE void updateGroupConversation(uint64_t groupId, const QString& nickname, const QString& avatar,
                                               const QString& lastMessage, const QDateTime& lastTime, int unreadDelta = 0);
    Q_INVOKABLE void removeGroupConversation(uint64_t groupId);
    int findIndexByGroupId(uint64_t groupId) const;

    int findIndexByFriendId(uint64_t friendId) const;

private:
    QList<ConversationData> m_conversations;
};

} // namespace chatroom::client
