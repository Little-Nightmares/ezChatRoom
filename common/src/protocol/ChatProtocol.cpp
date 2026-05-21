#include "ChatProtocol.h"

#include <QDateTime>
#include <QIODevice>

namespace chatroom {
namespace protocol {

namespace {
    std::atomic<uint32_t> g_sequenceCounter{0};
}

QByteArray ChatProtocol::pack(MessageType type, uint8_t flags,
                               uint32_t sequence, const QByteArray& body)
{
    QByteArray packet;
    packet.reserve(static_cast<int>(HEADER_SIZE + body.size()));

    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);

    // Write 20-byte header
    stream << static_cast<quint32>(MAGIC_NUMBER);   // 4 bytes
    stream << static_cast<quint8>(PROTOCOL_VERSION); // 1 byte
    stream << static_cast<quint8>(type);             // 1 byte
    stream << static_cast<quint8>(flags);            // 1 byte
    stream << static_cast<quint8>(0x00);             // 1 byte (reserved)
    stream << static_cast<quint32>(body.size());     // 4 bytes
    stream << static_cast<quint32>(sequence);        // 4 bytes
    stream << static_cast<quint32>(currentTime());   // 4 bytes

    // Append body
    packet.append(body);

    return packet;
}

int ChatProtocol::parse(const QByteArray& buffer,
                         PacketHeader& outHeader, QByteArray& outBody)
{
    // Not enough data for header
    if (static_cast<size_t>(buffer.size()) < HEADER_SIZE) {
        return 0;
    }

    QDataStream stream(buffer);
    stream.setByteOrder(QDataStream::BigEndian);

    quint32 magic = 0;
    quint8 version = 0;
    quint8 messageType = 0;
    quint8 flags = 0;
    quint8 reserved = 0;
    quint32 bodyLength = 0;
    quint32 sequence = 0;
    quint32 timestamp = 0;

    stream >> magic;
    stream >> version;
    stream >> messageType;
    stream >> flags;
    stream >> reserved;
    stream >> bodyLength;
    stream >> sequence;
    stream >> timestamp;

    // Validate magic number
    if (magic != MAGIC_NUMBER) {
        return -1;
    }

    // Validate protocol version
    if (version != PROTOCOL_VERSION) {
        return -1;
    }

    // Sanity check: body length should not exceed 10MB
    constexpr uint32_t MAX_BODY_LENGTH = 10 * 1024 * 1024;
    if (bodyLength > MAX_BODY_LENGTH) {
        return -1;
    }

    // Check if we have the full body
    size_t totalSize = HEADER_SIZE + bodyLength;
    if (static_cast<size_t>(buffer.size()) < totalSize) {
        return 0; // Incomplete
    }

    // Fill output header
    outHeader.magic = magic;
    outHeader.version = version;
    outHeader.messageType = messageType;
    outHeader.flags = flags;
    outHeader.reserved = reserved;
    outHeader.bodyLength = bodyLength;
    outHeader.sequence = sequence;
    outHeader.timestamp = timestamp;

    // Extract body
    outBody = buffer.mid(static_cast<int>(HEADER_SIZE), static_cast<int>(bodyLength));

    return static_cast<int>(totalSize);
}

uint32_t ChatProtocol::nextSequence()
{
    return g_sequenceCounter.fetch_add(1, std::memory_order_relaxed) + 1;
}

uint32_t ChatProtocol::currentTime()
{
    return static_cast<uint32_t>(
        QDateTime::currentDateTime().toSecsSinceEpoch()
    );
}

} // namespace protocol
} // namespace chatroom
