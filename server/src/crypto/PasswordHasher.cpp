#include "crypto/PasswordHasher.h"
#include "crypto/CryptoUtils.h"

namespace chatroom::server {

QString PasswordHasher::hashPassword(const QString& password) {
    // Generate 32-byte random salt
    QByteArray salt = chatroom::crypto::CryptoUtils::randomBytes(SALT_LENGTH);

    // Concatenate salt + password
    QByteArray data = salt + password.toUtf8();

    // Compute SHA-256(salt + password)
    QByteArray hash = chatroom::crypto::CryptoUtils::sha256(data);

    // Return "salt_hex:hash_hex"
    return QString(salt.toHex()) + ":" + QString(hash.toHex());
}

bool PasswordHasher::verifyPassword(const QString& password, const QString& storedHash) {
    // Split storedHash into salt and hash parts
    int separatorIndex = storedHash.indexOf(':');
    if (separatorIndex < 0) {
        return false;
    }

    QString saltHex = storedHash.left(separatorIndex);
    QString hashHex = storedHash.mid(separatorIndex + 1);

    QByteArray salt = QByteArray::fromHex(saltHex.toUtf8());
    QByteArray expectedHash = QByteArray::fromHex(hashHex.toUtf8());

    // Recompute hash: SHA256(salt + password)
    QByteArray data = salt + password.toUtf8();
    QByteArray computedHash = chatroom::crypto::CryptoUtils::sha256(data);

    // Compare hashes
    return computedHash == expectedHash;
}

} // namespace chatroom::server
