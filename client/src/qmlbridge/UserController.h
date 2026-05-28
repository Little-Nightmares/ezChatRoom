#pragma once

#include <QObject>
#include <QtQml/qqml.h>

namespace chatroom::client {

class AppManager;

class UserController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("UserController is created by AppCore and exposed to QML as a context property")

public:
    explicit UserController(AppManager* appManager, QObject* parent = nullptr);

    Q_INVOKABLE void login(const QString& username, const QString& password);
    Q_INVOKABLE void registerUser(const QString& username, const QString& password,
                                  const QString& nickname);
    Q_INVOKABLE void logout();

signals:
    void loginSuccess();
    void loginFailed(const QString& reason);
    void registerSuccess();
    void registerFailed(const QString& reason);

private:
    void handleLoginResponse(uint8_t flags, uint32_t sequence, const QByteArray& body);
    void handleRegisterResponse(uint8_t flags, uint32_t sequence, const QByteArray& body);

    AppManager* m_appManager = nullptr;
};

} // namespace chatroom::client
