#pragma once

#include <QString>
#include <QByteArray>

namespace chatroom::server {

class PasswordHasher {
public:
    PasswordHasher() = delete;

    // Generate password hash: SHA256(salt + password), returns "salt_hex:hash_hex"
    static QString hashPassword(const QString& password);

    // Verify password against stored hash
    static bool verifyPassword(const QString& password, const QString& storedHash);

private:
    static constexpr int SALT_LENGTH = 32;
};

} // namespace chatroom::server
