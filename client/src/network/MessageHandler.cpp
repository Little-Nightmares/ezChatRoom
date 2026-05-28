#include "MessageHandler.h"

#include <QDebug>

namespace chatroom::client {

MessageHandler::MessageHandler(QObject* parent)
    : QObject(parent)
{
}

void MessageHandler::registerHandler(uint8_t messageType, HandlerFunc handler)
{
    m_handlers.insert(messageType, std::move(handler));
}

void MessageHandler::handlePacket(uint8_t messageType, uint8_t flags,
                                  uint32_t sequence, const QByteArray& body)
{
    auto it = m_handlers.find(messageType);
    if (it != m_handlers.end()) {
        it.value()(flags, sequence, body);
    } else {
        qDebug() << "MessageHandler: no handler registered for message type"
                 << Qt::hex << static_cast<int>(messageType);
    }
}

} // namespace chatroom::client
