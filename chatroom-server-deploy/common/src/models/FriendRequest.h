#ifndef CHATROOM_MODELS_FRIEND_REQUEST_H
#define CHATROOM_MODELS_FRIEND_REQUEST_H

#include <QDateTime>
#include <QString>

#include <cstdint>

namespace chatroom {
namespace models {

enum class RequestStatus : uint8_t {
    Pending  = 0x00,
    Accepted = 0x01,
    Rejected = 0x02,
    Expired  = 0x03
};

struct FriendRequest {
    uint64_t     requestId  = 0;
    uint64_t     fromUserId = 0;
    uint64_t     toUserId   = 0;
    QString      fromUsername;
    QString      message;
    RequestStatus status    = RequestStatus::Pending;
    QDateTime    createdAt;
    QDateTime    processedAt;

    FriendRequest() = default;

    FriendRequest(uint64_t reqId, uint64_t from, uint64_t to,
                  const QString& msg, RequestStatus st = RequestStatus::Pending,
                  const QDateTime& created = QDateTime(),
                  const QDateTime& processed = QDateTime())
        : requestId(reqId)
        , fromUserId(from)
        , toUserId(to)
        , message(msg)
        , status(st)
        , createdAt(created)
        , processedAt(processed)
    {}
};

} // namespace models
} // namespace chatroom

#endif // CHATROOM_MODELS_FRIEND_REQUEST_H
