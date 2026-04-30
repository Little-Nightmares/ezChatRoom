#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QtQml/qqmlregistration.h>


namespace client {

class AppManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected WRITE setIsConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(bool notificationsEnabled READ notificationsEnabled WRITE setNotificationsEnabled NOTIFY notificationsEnabledChanged)

public:
    explicit AppManager(QObject *parent = nullptr);
    ~AppManager() override;

    // Getter for isConnected
    bool isConnected() const;
    // Setter for isConnected
    void setIsConnected(bool connected);

    // Getter for notificationsEnabled
    bool notificationsEnabled() const;
    // Setter for notificationsEnabled
    void setNotificationsEnabled(bool enabled);

signals:
    void isConnectedChanged();
    void notificationsEnabledChanged();

private:
    bool m_isConnected = false; // 默认离线
    bool m_notificationsEnabled = true; // 默认开启通知

};

} // namespace client

#endif // APPMANAGER_H
