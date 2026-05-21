#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QHostAddress>

namespace server {

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer() override;

    bool start(quint16 port = 6667, const QHostAddress &address = QHostAddress::Any);
    void stop();
    bool isRunning() const;

signals:
    void started(quint16 port);
    void stopped();
    void clientAccepted(qintptr socketDescriptor);

public slots:

protected:
    void incomingConnection(qintptr socketDescriptor) override;
};

} // namespace server

#endif // TCPSERVER_H
