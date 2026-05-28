#pragma once

#include <QObject>
#include <QMap>
#include <functional>
#include <cstdint>

namespace chatroom::client {

class MessageHandler : public QObject {
    Q_OBJECT
public:
    using HandlerFunc = std::function<void(uint8_t flags, uint32_t sequence,
                                            const QByteArray& body)>;

    explicit MessageHandler(QObject* parent = nullptr);

    void registerHandler(uint8_t messageType, HandlerFunc handler);

    void handlePacket(uint8_t messageType, uint8_t flags,
                      uint32_t sequence, const QByteArray& body);

private:
    QMap<uint8_t, HandlerFunc> m_handlers;
};

} // namespace chatroom::client
