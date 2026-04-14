#include "TcpServer.h"

namespace server {

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
{
}

TcpServer::~TcpServer()
{
}

} // namespace server
