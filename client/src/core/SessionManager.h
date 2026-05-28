#pragma once

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QString>
#include <cstdint>

namespace chatroom::client {

class SessionManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool loggedIn READ isLoggedIn NOTIFY loginStateChanged)
    Q_PROPERTY(quint64 userId READ userId NOTIFY userInfoChanged)
    Q_PROPERTY(QString username READ username NOTIFY userInfoChanged)
    Q_PROPERTY(QString nickname READ nickname NOTIFY userInfoChanged)
    Q_PROPERTY(QString avatar READ avatar NOTIFY avatarChanged)

public:
    explicit SessionManager(QObject* parent = nullptr);

    bool isLoggedIn() const;
    quint64 userId() const;
    QString username() const;
    QString nickname() const;
    QString avatar() const;

    void setLoggedIn(bool logged, quint64 userId,
                     const QString& username, const QString& nickname);
    void logout();

    // Saved credentials for auto-reconnect
    QString savedUsername() const { return m_savedUsername; }
    QString savedPasswordHash() const { return m_savedPasswordHash; }
    void saveCredentials(const QString& username, const QString& passwordHash);
    void clearCredentials();

    void setAvatar(const QString& avatar);

    // Key management
    void generateKeyPair();
    QByteArray getLocalPublicKey() const;
    QByteArray getLocalPrivateKey() const;
    void setLocalPublicKey(const QByteArray& key);
    void setLocalPrivateKey(const QByteArray& key);

    // Session keys (per-friend AES keys)
    void storeSessionKey(uint64_t friendId, const QByteArray& key);
    QByteArray getSessionKey(uint64_t friendId) const;
    bool hasSessionKey(uint64_t friendId) const;
    void removeSessionKey(uint64_t friendId);

signals:
    void loginStateChanged();
    void userInfoChanged();
    void avatarChanged();

private:
    bool m_loggedIn = false;
    quint64 m_userId = 0;
    QString m_username;
    QString m_nickname;
    QString m_avatar;

    QString m_savedUsername;
    QString m_savedPasswordHash;

    QByteArray m_localPublicKey;
    QByteArray m_localPrivateKey;
    QMap<uint64_t, QByteArray> m_sessionKeys;
};

} // namespace chatroom::client
