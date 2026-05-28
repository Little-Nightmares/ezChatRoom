#pragma once

#include <QObject>
#include <cstdint>
#include <QtQml/qqml.h>

namespace chatroom::client {

class UserModel;
class FriendRequestModel;
class AppManager;

class FriendController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("FriendController is created by AppCore and exposed to QML as a context property")
    Q_PROPERTY(UserModel* userModel READ userModel CONSTANT)
    Q_PROPERTY(FriendRequestModel* friendRequestModel READ friendRequestModel CONSTANT)

public:
    explicit FriendController(AppManager* appManager, QObject* parent = nullptr);

    UserModel* userModel() const;
    FriendRequestModel* friendRequestModel() const;

    Q_INVOKABLE void searchUser(const QString& keyword);
    Q_INVOKABLE void addFriend(quint64 userId, const QString& message);
    Q_INVOKABLE void acceptFriendRequest(quint64 requestId);
    Q_INVOKABLE void rejectFriendRequest(quint64 requestId);
    Q_INVOKABLE void deleteFriend(quint64 friendId);
    Q_INVOKABLE void requestFriendList();
    Q_INVOKABLE void requestFriendRequestList();
    Q_INVOKABLE bool isFriendOnline(quint64 userId) const;

signals:
    void searchResultReady();
    void friendRequestAccepted();
    void friendRequestRejected();
    void friendDeleted();
    void friendAdded();
    void friendListReady();
    void friendRequestListReady();
    void newFriendRequestReceived();  // 收到新的好友请求通知
    void friendOnlineStatusChanged(quint64 userId, bool online);  // 好友上下线

private:
    void handleFriendSearchResponse(uint8_t flags, uint32_t sequence,
                                     const QByteArray& body);
    void handleFriendAddResponse(uint8_t flags, uint32_t sequence,
                                  const QByteArray& body);
    void handleFriendAcceptResponse(uint8_t flags, uint32_t sequence,
                                     const QByteArray& body);
    void handleFriendRejectResponse(uint8_t flags, uint32_t sequence,
                                     const QByteArray& body);
    void handleFriendDeleteResponse(uint8_t flags, uint32_t sequence,
                                   const QByteArray& body);
    void handleFriendListResponse(uint8_t flags, uint32_t sequence,
                                   const QByteArray& body);
    void handleFriendRequestListResponse(uint8_t flags, uint32_t sequence,
                                          const QByteArray& body);
    void handleFriendRequestNotify(uint8_t flags, uint32_t sequence,
                                    const QByteArray& body);
    void handleFriendOnlineNotify(uint8_t flags, uint32_t sequence,
                                   const QByteArray& body);
    void handleFriendOfflineNotify(uint8_t flags, uint32_t sequence,
                                    const QByteArray& body);

    AppManager* m_appManager = nullptr;
    UserModel* m_userModel = nullptr;
    FriendRequestModel* m_friendRequestModel = nullptr;
};

} // namespace chatroom::client
