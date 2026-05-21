#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

namespace client {

class TcpClient;

class UserController : public QObject
{
    Q_OBJECT

public:
    explicit UserController(QObject *parent = nullptr);
    ~UserController() override;

signals:
    void loginStarted(const QString &username, const QString &host, int port);
    void loginFailed(const QString &message);
    void serverConnected();
    void serverDisconnected();
    void logoutRequested();

public slots:
    void login(const QString &username,
               const QString &password,
               const QString &host,
               int port);
    void logout();

private:
    TcpClient *m_client = nullptr;
};

} // namespace client

#endif // USERCONTROLLER_H
