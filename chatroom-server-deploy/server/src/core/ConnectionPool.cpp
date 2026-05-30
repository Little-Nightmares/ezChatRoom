#include "ConnectionPool.h"

namespace server {

ConnectionPool::ConnectionPool(QObject *parent)
    : QObject(parent)
{
}

ConnectionPool::~ConnectionPool()
{
}

} // namespace server
