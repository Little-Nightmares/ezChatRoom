#include "MessageRepository.h"

namespace server {

MessageRepository::MessageRepository(QObject *parent)
    : QObject(parent)
{
}

MessageRepository::~MessageRepository()
{
}

} // namespace server
