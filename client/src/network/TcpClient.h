#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QByteArray>
#include <QObject>
#include <QTcpSocket>

namespace client {

class TcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient() override;

    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &message);

public slots:
    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    bool sendRawData(const QByteArray &data);

private:
    QTcpSocket *m_socket = nullptr;
};

} // namespace client

#endif // TCPCLIENT_H
