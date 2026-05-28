#pragma once

#include <QObject>
#include <memory>
#include <QMap>
#include <QTimer>
#include <QtQml/qqml.h>
#include "qmlbridge/ChatController.h"
#include "qmlbridge/UserController.h"
#include "qmlbridge/FriendController.h"
#include "qmlbridge/GroupController.h"
#include "models/MessageModel.h"
#include "models/ConversationModel.h"
#include "models/FriendRequestModel.h"
#include "models/UserModel.h"
#include "models/GroupModel.h"
#include "core/SessionManager.h"
#include "qmlbridge/ThemeManager.h"

#ifdef CHATROOM_HAS_MULTIMEDIA
#include <QSoundEffect>
#include <QUrl>
#endif

namespace chatroom::client {

class AppManager;

class AppCore : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(ChatController* chatController READ chatController CONSTANT)
    Q_PROPERTY(UserController* userController READ userController CONSTANT)
    Q_PROPERTY(FriendController* friendController READ friendController CONSTANT)
    Q_PROPERTY(GroupController* groupController READ groupController CONSTANT)
    Q_PROPERTY(GroupModel* groupModel READ groupModel CONSTANT)
    Q_PROPERTY(MessageModel* messageModel READ messageModel CONSTANT)
    Q_PROPERTY(ConversationModel* conversationModel READ conversationModel CONSTANT)
    Q_PROPERTY(FriendRequestModel* friendRequestModel READ friendRequestModel CONSTANT)
    Q_PROPERTY(UserModel* userModel READ userModel CONSTANT)
    Q_PROPERTY(SessionManager* sessionManager READ sessionManager CONSTANT)
    Q_PROPERTY(AppManager* appManager READ appManager CONSTANT)
    Q_PROPERTY(ThemeManager* themeManager READ themeManager CONSTANT)
    Q_PROPERTY(QString chatBackground READ chatBackground NOTIFY chatBackgroundChanged)
    Q_PROPERTY(bool loggedIn READ isLoggedIn NOTIFY loginStateChanged)

public:
    explicit AppCore(QObject* parent = nullptr);
    ~AppCore();

    ChatController* chatController() const;
    UserController* userController() const;
    FriendController* friendController() const;
    GroupController* groupController() const;
    GroupModel* groupModel() const;
    MessageModel* messageModel() const;
    ConversationModel* conversationModel() const;
    FriendRequestModel* friendRequestModel() const;
    UserModel* userModel() const;
    SessionManager* sessionManager() const;
    AppManager* appManager() const;
    ThemeManager* themeManager() const;
    bool isLoggedIn() const;
    QString chatBackground() const { return m_chatBackground; }

    Q_INVOKABLE void initialize();
    Q_INVOKABLE void setChatBackground(const QString& colorOrPath);
    Q_INVOKABLE void clearChatBackground();
    Q_INVOKABLE void connectToServer(const QString& host, int port);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void showToast(const QString& message, const QString& type = "info");

signals:
    void loginStateChanged();
    void connected();
    void disconnected();
    void connectionError(const QString& error);
    void newMessagesReceived(quint64 senderId, const QString& senderName, int count);
    void kicked(const QString& reason);
    void showNotification(const QString& message, const QString& type);
    void chatBackgroundChanged();

private slots:
    void onNeedKeyExchange(quint64 friendId);
    void handleKeyExchangeRequest(uint8_t flags, uint32_t sequence, const QByteArray& body);

private:
    void registerMessageHandlers();
    void handleChatMessage(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleChatMessageAck(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleOfflineMessagePush(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleOfflineMessageDone(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleKeyExchangeResponse(uint8_t flags, uint32_t sequence, const QByteArray& body);

    // Message notification aggregation
    QMap<quint64, int> m_pendingMessageCounts;
    QMap<quint64, QTimer*> m_notificationTimers;

#ifdef CHATROOM_HAS_MULTIMEDIA
    QSoundEffect* m_messageSound = nullptr;
#endif

    void setupMessageNotification();
    void triggerNotification(quint64 senderId);
    void playNotificationSound();

    std::unique_ptr<AppManager> m_appManager;
    ChatController* m_chatController = nullptr;
    UserController* m_userController = nullptr;
    FriendController* m_friendController = nullptr;
    GroupModel* m_groupModel = nullptr;
    GroupController* m_groupController = nullptr;
    ThemeManager* m_themeManager = nullptr;
    ConversationModel* m_conversationModel = nullptr;
    QString m_chatBackground;
};

} // namespace chatroom::client
