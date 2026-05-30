#ifndef CHATROOM_CRYPTO_CRYPTO_UTILS_H
#define CHATROOM_CRYPTO_CRYPTO_UTILS_H

#include <QByteArray>

#include <cstdint>
#include <utility>

namespace chatroom {
namespace crypto {

class CryptoUtils {
public:
    // SHA-256 hash
    static QByteArray sha256(const QByteArray& data);

    // AES-256-GCM encrypt
    // Returns: IV(12 bytes) + Ciphertext + Tag(16 bytes)
    static QByteArray aes256GcmEncrypt(const QByteArray& plaintext,
                                        const QByteArray& key,
                                        const QByteArray& iv);

    // AES-256-GCM decrypt
    // Input format: IV(12 bytes) + Ciphertext + Tag(16 bytes)
    // Returns plaintext on success, empty QByteArray on failure
    static QByteArray aes256GcmDecrypt(const QByteArray& encryptedData,
                                        const QByteArray& key);

    // Generate RSA-2048 key pair
    // Returns {publicKeyPem, privateKeyPem}
    static std::pair<QByteArray, QByteArray> generateRsaKeyPair();

    // RSA encrypt with OAEP padding
    static QByteArray rsaEncrypt(const QByteArray& data,
                                  const QByteArray& publicKeyPem);

    // RSA decrypt with OAEP padding
    static QByteArray rsaDecrypt(const QByteArray& encryptedData,
                                  const QByteArray& privateKeyPem);

    // Generate random bytes
    static QByteArray randomBytes(int size);

    // Generate 32-byte AES-256 key
    static QByteArray generateAesKey();

    // Generate 12-byte IV for AES-GCM
    static QByteArray generateIv();
};

} // namespace crypto
} // namespace chatroom

#endif // CHATROOM_CRYPTO_CRYPTO_UTILS_H
