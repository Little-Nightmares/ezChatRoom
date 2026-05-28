#include "SessionManager.h"
#include "crypto/CryptoUtils.h"

#include <QDebug>

namespace chatroom::client {

SessionManager::SessionManager(QObject* parent)
    : QObject(parent)
{
}

bool SessionManager::isLoggedIn() const
{
    return m_loggedIn;
}

quint64 SessionManager::userId() const
{
    return m_userId;
}

QString SessionManager::username() const
{
    return m_username;
}

QString SessionManager::nickname() const
{
    return m_nickname;
}

QString SessionManager::avatar() const
{
    return m_avatar;
}

void SessionManager::setAvatar(const QString& avatar)
{
    if (m_avatar != avatar) {
        m_avatar = avatar;
        emit avatarChanged();
    }
}

void SessionManager::setLoggedIn(bool logged, quint64 userId,
                                 const QString& username, const QString& nickname)
{
    if (m_loggedIn == logged && m_userId == userId) {
        return;
    }
    m_loggedIn = logged;
    m_userId = userId;
    m_username = username;
    m_nickname = nickname;
    emit loginStateChanged();
    emit userInfoChanged();
    qDebug() << "SessionManager: login state changed, logged=" << logged
             << "userId=" << userId << "username=" << username;
}

void SessionManager::logout()
{
    m_loggedIn = false;
    m_userId = 0;
    m_username.clear();
    m_nickname.clear();
    m_sessionKeys.clear();
    m_savedUsername.clear();
    m_savedPasswordHash.clear();
    emit loginStateChanged();
    emit userInfoChanged();
    qDebug() << "SessionManager: logged out";
}

void SessionManager::saveCredentials(const QString& username, const QString& passwordHash)
{
    m_savedUsername = username;
    m_savedPasswordHash = passwordHash;
}

void SessionManager::clearCredentials()
{
    m_savedUsername.clear();
    m_savedPasswordHash.clear();
}

void SessionManager::generateKeyPair()
{
    auto [pubKey, privKey] = chatroom::crypto::CryptoUtils::generateRsaKeyPair();
    m_localPublicKey = pubKey;
    m_localPrivateKey = privKey;
    qDebug() << "SessionManager: generated new RSA key pair, pub key size:"
             << pubKey.size() << "priv key size:" << privKey.size();
}

QByteArray SessionManager::getLocalPublicKey() const
{
    return m_localPublicKey;
}

QByteArray SessionManager::getLocalPrivateKey() const
{
    return m_localPrivateKey;
}

void SessionManager::setLocalPublicKey(const QByteArray& key)
{
    m_localPublicKey = key;
}

void SessionManager::setLocalPrivateKey(const QByteArray& key)
{
    m_localPrivateKey = key;
}

void SessionManager::storeSessionKey(uint64_t friendId, const QByteArray& key)
{
    m_sessionKeys.insert(friendId, key);
    qDebug() << "SessionManager: stored session key for friend" << friendId;
}

QByteArray SessionManager::getSessionKey(uint64_t friendId) const
{
    return m_sessionKeys.value(friendId, QByteArray());
}

bool SessionManager::hasSessionKey(uint64_t friendId) const
{
    return m_sessionKeys.contains(friendId);
}

void SessionManager::removeSessionKey(uint64_t friendId)
{
    m_sessionKeys.remove(friendId);
}

} // namespace chatroom::client
