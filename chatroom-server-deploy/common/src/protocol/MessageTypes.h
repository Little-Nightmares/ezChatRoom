#ifndef CHATROOM_PROTOCOL_MESSAGE_TYPES_H
#define CHATROOM_PROTOCOL_MESSAGE_TYPES_H

#include <cstdint>

namespace chatroom {
namespace protocol {

enum class MessageType : uint8_t {
    Heartbeat             = 0x01,
    HeartbeatAck          = 0x02,
    LoginRequest          = 0x10,
    LoginResponse         = 0x11,
    RegisterRequest       = 0x12,
    RegisterResponse      = 0x13,
    LogoutRequest         = 0x14,
    FriendSearchRequest   = 0x20,
    FriendSearchResponse  = 0x21,
    FriendAddRequest      = 0x22,
    FriendAddResponse     = 0x23,
    FriendAcceptRequest   = 0x24,
    FriendAcceptResponse  = 0x25,
    FriendRejectRequest   = 0x26,
    FriendRejectResponse  = 0x27,
    FriendDeleteRequest   = 0x28,
    FriendDeleteResponse  = 0x29,
    FriendListRequest     = 0x2A,
    FriendListResponse    = 0x2B,
    FriendRequestListRequest  = 0x2C,
    FriendRequestListResponse = 0x2D,
    FriendRequestNotify       = 0x2E,  // 服务端推送好友请求通知给目标用户
    FriendOnlineNotify        = 0x2F,  // 好友上线通知
    FriendOfflineNotify       = 0x30,  // 好友下线通知
    ChatMessage           = 0x31,
    ChatMessageAck        = 0x32,
    OfflineMessagePush    = 0x33,
    OfflineMessageDone    = 0x34,
    ServerKickNotify      = 0x35,
    KeyExchangeRequest    = 0x40,
    KeyExchangeResponse   = 0x41,
    MessageRecallRequest  = 0x42,
    MessageRecallNotify   = 0x43,
    AvatarUploadRequest   = 0x50,
    AvatarUploadResponse  = 0x51,

    // Group chat messages (0x60 - 0x78)
    GroupCreateRequest       = 0x60,
    GroupCreateResponse      = 0x61,
    GroupListRequest         = 0x62,
    GroupListResponse        = 0x63,
    GroupInfoRequest         = 0x64,
    GroupInfoResponse        = 0x65,
    GroupMessage             = 0x66,
    GroupMessageAck          = 0x67,
    GroupMessageNotify       = 0x68,
    GroupMemberAddRequest    = 0x69,
    GroupMemberAddNotify     = 0x6A,
    GroupMemberRemoveRequest = 0x6B,
    GroupMemberRemoveNotify  = 0x6C,
    GroupUpdateRequest       = 0x6D,
    GroupUpdateNotify        = 0x6E,
    GroupLeaveRequest        = 0x6F,
    GroupLeaveNotify         = 0x70,
    GroupOfflineMsgPush      = 0x71,
    GroupOfflineMsgDone      = 0x72,
    GroupDissolveRequest     = 0x73,
    GroupDissolveNotify      = 0x74,
    GroupAvatarUploadReq     = 0x75,
    GroupAvatarUploadResp    = 0x76,
    GroupTransferOwnerReq    = 0x77,
    GroupTransferOwnerNotify = 0x78,
    GroupMemberAddResponse       = 0x79,
    GroupMemberRemoveResponse    = 0x7A,
    GroupUpdateResponse          = 0x7B,
    GroupLeaveResponse           = 0x7C,
    GroupDissolveResponse        = 0x7D,
    GroupTransferOwnerResponse   = 0x7E,

    // File transfer (0x80 - 0x8F)
    FileUploadRequest        = 0x80,
    FileUploadResponse       = 0x81,
    FileDownloadRequest      = 0x82,
    FileDownloadResponse     = 0x83,
    FileChunkTransfer        = 0x84,
    FileChunkAck             = 0x85,

    // Message search (0x90 - 0x93)
    MessageSearchRequest     = 0x90,
    MessageSearchResponse    = 0x91,

    // Group enhancements (0x94 - 0x9F)
    GroupAnnouncementRequest = 0x94,
    GroupAnnouncementPush    = 0x95,
    GroupMessageReadNotify   = 0x96,
    GroupAtNotify            = 0x97,
    GroupKeyRequest          = 0x98,
    GroupKeyResponse         = 0x99,

    // Message features (0xA0 - 0xA3)
    MessageReply             = 0xA0,
    MessageReplyNotify       = 0xA1
};

enum class MessageFlag : uint8_t {
    None       = 0x00,
    Encrypted  = 0x01,
    Compressed = 0x02,
    IsRequest  = 0x04,
    IsResponse = 0x08
};

inline constexpr MessageFlag operator|(MessageFlag a, MessageFlag b) {
    return static_cast<MessageFlag>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
    );
}

inline constexpr MessageFlag& operator|=(MessageFlag& a, MessageFlag b) {
    a = a | b;
    return a;
}

inline constexpr bool hasFlag(MessageFlag flags, MessageFlag flag) {
    return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flag)) != 0;
}

} // namespace protocol
} // namespace chatroom

#endif // CHATROOM_PROTOCOL_MESSAGE_TYPES_H
