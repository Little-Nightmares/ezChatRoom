#include "ThreadPool.h"

namespace server {

ThreadPool::ThreadPool(QObject *parent)
    : QObject(parent)
{
}

ThreadPool::~ThreadPool()
{
}

} // namespace server
