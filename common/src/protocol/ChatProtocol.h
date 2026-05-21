#ifndef CHATROOM_PROTOCOL_CHAT_PROTOCOL_H
#define CHATROOM_PROTOCOL_CHAT_PROTOCOL_H

#include "MessageTypes.h"

#include <QByteArray>
#include <QDataStream>

#include <atomic>
#include <cstdint>

namespace chatroom {
namespace protocol {

constexpr uint32_t MAGIC_NUMBER = 0x43524D47; // "CRMG"
constexpr uint8_t  PROTOCOL_VERSION = 0x01;
constexpr size_t   HEADER_SIZE = 20;

struct PacketHeader {
    uint32_t    magic;
    uint8_t     version;
    uint8_t     messageType;
    uint8_t     flags;
    uint8_t     reserved;
    uint32_t    bodyLength;
    uint32_t    sequence;
    uint32_t    timestamp;
};

class ChatProtocol {
public:
    // Pack a message into a QByteArray with header + body
    // Header layout (20 bytes, BigEndian):
    //   uint32  magic        (4 bytes)
    //   uint8   version      (1 byte)
    //   uint8   messageType  (1 byte)
    //   uint8   flags        (1 byte)
    //   uint8   reserved     (1 byte)
    //   uint32  bodyLength   (4 bytes)
    //   uint32  sequence     (4 bytes)
    //   uint32  timestamp    (4 bytes)
    static QByteArray pack(MessageType type, uint8_t flags,
                           uint32_t sequence, const QByteArray& body);

    // Parse a buffer, extract header and body if complete.
    // Returns bytes consumed (>0) on success, 0 if incomplete, -1 on error.
    static int parse(const QByteArray& buffer,
                     PacketHeader& outHeader, QByteArray& outBody);

    // Thread-safe atomic sequence number generator
    static uint32_t nextSequence();

    // Current Unix timestamp in seconds
    static uint32_t currentTime();
};

} // namespace protocol
} // namespace chatroom

#endif // CHATROOM_PROTOCOL_CHAT_PROTOCOL_H
