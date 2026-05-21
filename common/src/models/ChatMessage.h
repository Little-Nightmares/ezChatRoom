#ifndef CHATROOM_MODELS_CHAT_MESSAGE_H
#define CHATROOM_MODELS_CHAT_MESSAGE_H

#include <QDateTime>
#include <QString>

#include <cstdint>

namespace chatroom {
namespace models {

enum class ChatMessageType : uint8_t {
    Text     = 0x00,
    Image    = 0x01,
    File     = 0x02,
    System   = 0x03
};

struct ChatMessage {
    uint64_t    messageId  = 0;
    uint64_t    senderId   = 0;
    uint64_t    receiverId = 0;
    uint64_t    groupId    = 0;        // 群消息时为群组ID, 私聊时为0
    QString     senderNickname;         // 群消息中显示发送者昵称
    QString     content;
    QDateTime   timestamp;
    bool        isRead     = false;
    bool        isMine     = false;
    ChatMessageType type   = ChatMessageType::Text;

    enum class MessageStatus : uint8_t {
        Sending   = 0,
        Sent      = 1,
        Delivered = 2,
        Read      = 3,
        Failed    = 4
    };

    MessageStatus status = MessageStatus::Sent;

    ChatMessage() = default;

    ChatMessage(uint64_t msgId, uint64_t sender, uint64_t receiver,
                const QString& msgContent, const QDateTime& ts,
                bool read = false, bool mine = false,
                ChatMessageType msgType = ChatMessageType::Text,
                uint64_t grpId = 0,
                const QString& senderNick = "")
        : messageId(msgId)
        , senderId(sender)
        , receiverId(receiver)
        , groupId(grpId)
        , senderNickname(senderNick)
        , content(msgContent)
        , timestamp(ts)
        , isRead(read)
        , isMine(mine)
        , type(msgType)
    {}
};

} // namespace models
} // namespace chatroom

#endif // CHATROOM_MODELS_CHAT_MESSAGE_H
