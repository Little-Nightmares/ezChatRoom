#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QtQml/qqml.h>
#include "models/ChatMessage.h"

namespace chatroom::client {

class MessageModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        MessageIdRole = Qt::UserRole + 1,
        SenderIdRole,
        ContentRole,
        TimestampRole,
        IsMineRole,
        TypeRole,
        StatusRole,
        GroupIdRole,
        SenderNicknameRole
    };
    Q_ENUM(Roles)

    explicit MessageModel(QObject* parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addMessage(const chatroom::models::ChatMessage& msg);
    Q_INVOKABLE void clear();
    void loadMessages(const QVector<chatroom::models::ChatMessage>& messages);
    Q_INVOKABLE void updateMessageStatus(int index, chatroom::models::ChatMessage::MessageStatus status);
    Q_INVOKABLE void updateMessageContent(qint64 messageId, const QString& newContent);
    Q_INVOKABLE void markLastSendingAsDelivered();

private:
    QList<chatroom::models::ChatMessage> m_messages;
};

} // namespace chatroom::client
