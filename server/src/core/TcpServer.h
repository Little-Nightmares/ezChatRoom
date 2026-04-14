#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>


namespace server {

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer() override;

signals:

public slots:

private:
};

} // namespace server

#endif // TCPSERVER_H
