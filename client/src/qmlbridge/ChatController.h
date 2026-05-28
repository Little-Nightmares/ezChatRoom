#pragma once

#include <QObject>
#include <QDateTime>
#include <cstdint>
#include <QMap>
#include <QtQml/qqml.h>

namespace chatroom::client {

class MessageModel;
class SessionManager;
class AppManager;

class ChatController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("ChatController is created by AppCore and exposed to QML as a context property")
    Q_PROPERTY(MessageModel* messageModel READ messageModel CONSTANT)
    Q_PROPERTY(quint64 currentChatFriendId READ currentChatFriendId
               WRITE setCurrentChatFriendId NOTIFY currentChatFriendIdChanged)
    Q_PROPERTY(QString currentChatNickname READ currentChatNickname
               NOTIFY currentChatFriendIdChanged)
    Q_PROPERTY(quint64 currentGroupId READ currentGroupId WRITE setCurrentGroupId NOTIFY currentGroupIdChanged)
    Q_PROPERTY(QString currentGroupName READ currentGroupName NOTIFY currentGroupIdChanged)
    Q_PROPERTY(bool isInGroupChat READ isInGroupChat NOTIFY currentGroupIdChanged)

public:
    explicit ChatController(AppManager* appManager, QObject* parent = nullptr);

    MessageModel* messageModel() const;
    quint64 currentChatFriendId() const;
    void setCurrentChatFriendId(quint64 friendId);
    QString currentChatNickname() const;

    quint64 currentGroupId() const;
    void setCurrentGroupId(quint64 groupId);
    QString currentGroupName() const;
    bool isInGroupChat() const;

    Q_INVOKABLE void sendMessage(const QString& content);
    Q_INVOKABLE void loadChatHistory(quint64 friendId);
    Q_INVOKABLE void sendGroupMessage(const QString& content);
    Q_INVOKABLE void loadGroupChatHistory(quint64 groupId);
    Q_INVOKABLE void clearCurrentChat();
    Q_INVOKABLE void clearAllHistory();
    Q_INVOKABLE void searchMessages(const QString& keyword);
    Q_INVOKABLE void clearSearch();
    Q_INVOKABLE void recallMessage(qint64 messageId);
    Q_INVOKABLE void uploadAvatar(const QString& imagePath);

    // File transfer
    Q_INVOKABLE void sendFile(const QString& filePath);
    Q_INVOKABLE void downloadFile(quint64 fileId);
    void handleFileUploadResponse(const QByteArray& body);
    void handleFileDownloadResponse(const QByteArray& body);

    // Called when a chat message is received from the server
    void onChatMessageReceived(uint64_t senderId, const QString& content,
                               const QDateTime& timestamp, bool encrypted);
    // Called when offline messages are pushed
    void onOfflineMessageReceived(uint64_t senderId, const QString& content,
                                  const QDateTime& timestamp, bool encrypted);

    // Map sequence number to local message ID for ACK matching
    void registerPendingAck(uint32_t sequence, uint64_t messageId);
    bool acknowledgeMessage(uint32_t sequence);

signals:
    void currentChatFriendIdChanged();
    void currentGroupIdChanged();
    void needKeyExchange(quint64 friendId);
    void messageSent();
    void unreadCleared(quint64 friendId);
    void messageSendFailed(const QString& error);

private:
    AppManager* m_appManager = nullptr;
    MessageModel* m_messageModel = nullptr;
    quint64 m_currentChatFriendId = 0;
    QString m_currentChatNickname;
    quint64 m_currentGroupId = 0;
    QString m_currentGroupName;
    QMap<uint32_t, uint64_t> m_pendingAcks; // sequence -> messageId
};

} // namespace chatroom::client
