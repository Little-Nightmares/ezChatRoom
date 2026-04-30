#include "AppManager.h"

namespace client {

AppManager::AppManager(QObject *parent)
    : QObject(parent)
{
}

AppManager::~AppManager()
{
}

// Getter for isConnected
bool AppManager::isConnected() const
{
    return m_isConnected;
}

// Setter for isConnected
void AppManager::setIsConnected(bool connected)
{
    if (m_isConnected != connected) {
        m_isConnected = connected;
        emit isConnectedChanged();
    }
}

// Getter for notificationsEnabled
bool AppManager::notificationsEnabled() const
{
    return m_notificationsEnabled;
}

// Setter for notificationsEnabled
void AppManager::setNotificationsEnabled(bool enabled)
{
    if (m_notificationsEnabled != enabled) {
        m_notificationsEnabled = enabled;
        emit notificationsEnabledChanged();
    }
}

} // namespace client
