#include "CryptoUtils.h"

#include <QDebug>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

namespace chatroom {
namespace crypto {

namespace {

// RAII wrapper for EVP_CIPHER_CTX
struct CipherCtxDeleter {
    void operator()(EVP_CIPHER_CTX* ctx) const {
        if (ctx) EVP_CIPHER_CTX_free(ctx);
    }
};
using UniqueCipherCtx = std::unique_ptr<EVP_CIPHER_CTX, CipherCtxDeleter>;

// RAII wrapper for EVP_PKEY
struct PkeyDeleter {
    void operator()(EVP_PKEY* pkey) const {
        if (pkey) EVP_PKEY_free(pkey);
    }
};
using UniquePkey = std::unique_ptr<EVP_PKEY, PkeyDeleter>;

// RAII wrapper for BIO
struct BioDeleter {
    void operator()(BIO* bio) const {
        if (bio) BIO_free_all(bio);
    }
};
using UniqueBio = std::unique_ptr<BIO, BioDeleter>;

} // anonymous namespace

// ---------------------------------------------------------------------------
// SHA-256
// ---------------------------------------------------------------------------
QByteArray CryptoUtils::sha256(const QByteArray& data)
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        qWarning() << "CryptoUtils::sha256: failed to create EVP_MD_CTX";
        return {};
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        qWarning() << "CryptoUtils::sha256: DigestInit failed";
        EVP_MD_CTX_free(ctx);
        return {};
    }

    if (EVP_DigestUpdate(ctx, data.constData(), static_cast<size_t>(data.size())) != 1) {
        qWarning() << "CryptoUtils::sha256: DigestUpdate failed";
        EVP_MD_CTX_free(ctx);
        return {};
    }

    if (EVP_DigestFinal_ex(ctx, hash, &hashLen) != 1) {
        qWarning() << "CryptoUtils::sha256: DigestFinal failed";
        EVP_MD_CTX_free(ctx);
        return {};
    }

    EVP_MD_CTX_free(ctx);

    return QByteArray(reinterpret_cast<const char*>(hash), static_cast<int>(hashLen));
}

// ---------------------------------------------------------------------------
// AES-256-GCM Encrypt
// ---------------------------------------------------------------------------
QByteArray CryptoUtils::aes256GcmEncrypt(const QByteArray& plaintext,
                                          const QByteArray& key,
                                          const QByteArray& iv)
{
    // Validate key size (32 bytes for AES-256)
    if (key.size() != 32) {
        qWarning() << "CryptoUtils::aes256GcmEncrypt: key must be 32 bytes, got" << key.size();
        return {};
    }

    // Validate IV size (12 bytes for GCM)
    if (iv.size() != 12) {
        qWarning() << "CryptoUtils::aes256GcmEncrypt: IV must be 12 bytes, got" << iv.size();
        return {};
    }

    UniqueCipherCtx ctx(EVP_CIPHER_CTX_new());
    if (!ctx) {
        qWarning() << "CryptoUtils::aes256GcmEncrypt: failed to create cipher context";
        return {};
    }

    const EVP_CIPHER* cipher = EVP_aes_256_gcm();
    if (!cipher) {
        qWarning() << "CryptoUtils::aes256GcmEncrypt: failed to get AES-256-GCM cipher";
        return {};
    }

    // Initialize encryption
    if (EVP_EncryptInit_ex(ctx.get(), cipher, nullptr,
                           reinterpret_cast<const unsigned char*>(key.constData()),
                           reinterpret_cast<const unsigned char*>(iv.constData())) != 1) {
        qWarning() << "CryptoUtils::aes256GcmEncrypt: EncryptInit failed";
        return {};
    }

    // Allocate output buffer: plaintext + potential padding (GCM is stream-like, no padding needed)
    int maxOutLen = plaintext.size() + EVP_MAX_BLOCK_LENGTH;
    QByteArray ciphertext;
    ciphertext.resize(maxOutLen);

    int outLen = 0;
    int cipherLen = 0;

    // Encrypt plaintext
    if (EVP_EncryptUpdate(ctx.get(),
                          reinterpret_cast<unsigned char*>(ciphertext.data()),
                          &outLen,
                          reinterpret_cast<const unsigned char*>(plaintext.constData()),
                          plaintext.size()) != 1) {
        qWarning() << "CryptoUtils::aes256GcmEncrypt: EncryptUpdate failed";
        return {};
    }
    cipherLen = outLen;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx.get(),
                            reinterpret_cast<unsigned char*>(ciphertext.data()) + outLen,
                            &outLen) != 1) {
        qWarning() << "CryptoUtils::aes256GcmEncrypt: EncryptFinal failed";
        return {};
    }
    cipherLen += outLen;

    // Get the authentication tag (16 bytes)
    QByteArray tag;
    tag.resize(16);
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_GET_TAG, 16,
                            tag.data()) != 1) {
        qWarning() << "CryptoUtils::aes256GcmEncrypt: failed to get GCM tag";
        return {};
    }

    // Assemble result: IV(12) + Ciphertext + Tag(16)
    ciphertext.resize(cipherLen);
    QByteArray result;
    result.reserve(iv.size() + cipherLen + tag.size());
    result.append(iv);
    result.append(ciphertext);
    result.append(tag);

    return result;
}

// ---------------------------------------------------------------------------
// AES-256-GCM Decrypt
// ---------------------------------------------------------------------------
QByteArray CryptoUtils::aes256GcmDecrypt(const QByteArray& encryptedData,
                                          const QByteArray& key)
{
    // Validate key size
    if (key.size() != 32) {
        qWarning() << "CryptoUtils::aes256GcmDecrypt: key must be 32 bytes, got" << key.size();
        return {};
    }

    // Minimum size: IV(12) + Tag(16) = 28 bytes (no actual ciphertext)
    if (encryptedData.size() < 28) {
        qWarning() << "CryptoUtils::aes256GcmDecrypt: encrypted data too short";
        return {};
    }

    // Extract IV, ciphertext, and tag
    QByteArray iv = encryptedData.mid(0, 12);
    QByteArray tag = encryptedData.right(16);
    QByteArray ciphertext = encryptedData.mid(12, encryptedData.size() - 12 - 16);

    UniqueCipherCtx ctx(EVP_CIPHER_CTX_new());
    if (!ctx) {
        qWarning() << "CryptoUtils::aes256GcmDecrypt: failed to create cipher context";
        return {};
    }

    const EVP_CIPHER* cipher = EVP_aes_256_gcm();
    if (!cipher) {
        qWarning() << "CryptoUtils::aes256GcmDecrypt: failed to get AES-256-GCM cipher";
        return {};
    }

    // Initialize decryption
    if (EVP_DecryptInit_ex(ctx.get(), cipher, nullptr,
                           reinterpret_cast<const unsigned char*>(key.constData()),
                           reinterpret_cast<const unsigned char*>(iv.constData())) != 1) {
        qWarning() << "CryptoUtils::aes256GcmDecrypt: DecryptInit failed";
        return {};
    }

    // Allocate output buffer
    int maxOutLen = ciphertext.size() + EVP_MAX_BLOCK_LENGTH;
    QByteArray plaintext;
    plaintext.resize(maxOutLen);

    int outLen = 0;
    int plainLen = 0;

    // Decrypt ciphertext
    if (EVP_DecryptUpdate(ctx.get(),
                          reinterpret_cast<unsigned char*>(plaintext.data()),
                          &outLen,
                          reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                          ciphertext.size()) != 1) {
        qWarning() << "CryptoUtils::aes256GcmDecrypt: DecryptUpdate failed";
        return {};
    }
    plainLen = outLen;

    // Set the expected authentication tag
    if (EVP_CIPHER_CTX_ctrl(ctx.get(), EVP_CTRL_GCM_SET_TAG, 16,
                            const_cast<char*>(tag.constData())) != 1) {
        qWarning() << "CryptoUtils::aes256GcmDecrypt: failed to set GCM tag";
        return {};
    }

    // Verify tag and finalize
    if (EVP_DecryptFinal_ex(ctx.get(),
                            reinterpret_cast<unsigned char*>(plaintext.data()) + outLen,
                            &outLen) != 1) {
        qWarning() << "CryptoUtils::aes256GcmDecrypt: DecryptFinal failed (tag mismatch?)";
        return {};
    }
    plainLen += outLen;

    plaintext.resize(plainLen);
    return plaintext;
}

// ---------------------------------------------------------------------------
// RSA Key Pair Generation (RSA-2048)
// ---------------------------------------------------------------------------
std::pair<QByteArray, QByteArray> CryptoUtils::generateRsaKeyPair()
{
    // Create EVP_PKEY context for RSA key generation
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!pctx) {
        qWarning() << "CryptoUtils::generateRsaKeyPair: failed to create PKEY_CTX";
        return {};
    }

    if (EVP_PKEY_keygen_init(pctx) <= 0) {
        qWarning() << "CryptoUtils::generateRsaKeyPair: keygen_init failed";
        EVP_PKEY_CTX_free(pctx);
        return {};
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048) <= 0) {
        qWarning() << "CryptoUtils::generateRsaKeyPair: set_rsa_keygen_bits failed";
        EVP_PKEY_CTX_free(pctx);
        return {};
    }

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(pctx, &pkey) <= 0) {
        qWarning() << "CryptoUtils::generateRsaKeyPair: keygen failed";
        EVP_PKEY_CTX_free(pctx);
        return {};
    }
    EVP_PKEY_CTX_free(pctx);

    UniquePkey key(pkey);

    // Write public key to PEM
    UniqueBio pubBio(BIO_new(BIO_s_mem()));
    if (!pubBio) {
        qWarning() << "CryptoUtils::generateRsaKeyPair: failed to create pub BIO";
        return {};
    }
    if (PEM_write_bio_PUBKEY(pubBio.get(), key.get()) != 1) {
        qWarning() << "CryptoUtils::generateRsaKeyPair: failed to write public key";
        return {};
    }

    char* pubData = nullptr;
    long pubLen = BIO_get_mem_data(pubBio.get(), &pubData);
    QByteArray pubKeyPem(pubData, static_cast<int>(pubLen));

    // Write private key to PEM
    UniqueBio privBio(BIO_new(BIO_s_mem()));
    if (!privBio) {
        qWarning() << "CryptoUtils::generateRsaKeyPair: failed to create priv BIO";
        return {};
    }
    if (PEM_write_bio_PrivateKey(privBio.get(), key.get(), nullptr, nullptr, 0, nullptr, nullptr) != 1) {
        qWarning() << "CryptoUtils::generateRsaKeyPair: failed to write private key";
        return {};
    }

    char* privData = nullptr;
    long privLen = BIO_get_mem_data(privBio.get(), &privData);
    QByteArray privKeyPem(privData, static_cast<int>(privLen));

    return {pubKeyPem, privKeyPem};
}

// ---------------------------------------------------------------------------
// RSA Encrypt (OAEP padding)
// ---------------------------------------------------------------------------
QByteArray CryptoUtils::rsaEncrypt(const QByteArray& data,
                                    const QByteArray& publicKeyPem)
{
    // Load public key from PEM
    UniqueBio bio(BIO_new_mem_buf(publicKeyPem.constData(), publicKeyPem.size()));
    if (!bio) {
        qWarning() << "CryptoUtils::rsaEncrypt: failed to create BIO";
        return {};
    }

    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr);
    if (!pkey) {
        qWarning() << "CryptoUtils::rsaEncrypt: failed to read public key from PEM";
        return {};
    }
    UniquePkey key(pkey);

    // Create encryption context
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new(key.get(), nullptr);
    if (!pctx) {
        qWarning() << "CryptoUtils::rsaEncrypt: failed to create PKEY_CTX";
        return {};
    }

    if (EVP_PKEY_encrypt_init(pctx) <= 0) {
        qWarning() << "CryptoUtils::rsaEncrypt: encrypt_init failed";
        EVP_PKEY_CTX_free(pctx);
        return {};
    }

    if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        qWarning() << "CryptoUtils::rsaEncrypt: set_rsa_padding failed";
        EVP_PKEY_CTX_free(pctx);
        return {};
    }

    // Determine output buffer size
    size_t outLen = 0;
    if (EVP_PKEY_encrypt(pctx, nullptr, &outLen,
                         reinterpret_cast<const unsigned char*>(data.constData()),
                         static_cast<size_t>(data.size())) <= 0) {
        qWarning() << "CryptoUtils::rsaEncrypt: failed to determine output size";
        EVP_PKEY_CTX_free(pctx);
        return {};
    }

    QByteArray encrypted;
    encrypted.resize(static_cast<int>(outLen));

    // Perform encryption
    if (EVP_PKEY_encrypt(pctx,
                         reinterpret_cast<unsigned char*>(encrypted.data()),
                         &outLen,
                         reinterpret_cast<const unsigned char*>(data.constData()),
                         static_cast<size_t>(data.size())) <= 0) {
        qWarning() << "CryptoUtils::rsaEncrypt: encryption failed";
        EVP_PKEY_CTX_free(pctx);
        return {};
    }

    EVP_PKEY_CTX_free(pctx);
    encrypted.resize(static_cast<int>(outLen));

    return encrypted;
}

// ---------------------------------------------------------------------------
// RSA Decrypt (OAEP padding)
// ---------------------------------------------------------------------------
QByteArray CryptoUtils::rsaDecrypt(const QByteArray& encryptedData,
                                    const QByteArray& privateKeyPem)
{
    // Load private key from PEM
    UniqueBio bio(BIO_new_mem_buf(privateKeyPem.constData(), privateKeyPem.size()));
    if (!bio) {
        qWarning() << "CryptoUtils::rsaDecrypt: failed to create BIO";
        return {};
    }

    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr);
    if (!pkey) {
        qWarning() << "CryptoUtils::rsaDecrypt: failed to read private key from PEM";
        return {};
    }
    UniquePkey key(pkey);

    // Create decryption context
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new(key.get(), nullptr);
    if (!pctx) {
        qWarning() << "CryptoUtils::rsaDecrypt: failed to create PKEY_CTX";
        return {};
    }

    if (EVP_PKEY_decrypt_init(pctx) <= 0) {
        qWarning() << "CryptoUtils::rsaDecrypt: decrypt_init failed";
        EVP_PKEY_CTX_free(pctx);
        return {};
    }

    if (EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        qWarning() << "CryptoUtils::rsaDecrypt: set_rsa_padding failed";
        EVP_PKEY_CTX_free(pctx);
        return {};
    }

    // Determine output buffer size
    size_t outLen = 0;
    if (EVP_PKEY_decrypt(pctx, nullptr, &outLen,
                         reinterpret_cast<const unsigned char*>(encryptedData.constData()),
                         static_cast<size_t>(encryptedData.size())) <= 0) {
        qWarning() << "CryptoUtils::rsaDecrypt: failed to determine output size";
        EVP_PKEY_CTX_free(pctx);
        return {};
    }

    QByteArray decrypted;
    decrypted.resize(static_cast<int>(outLen));

    // Perform decryption
    if (EVP_PKEY_decrypt(pctx,
                         reinterpret_cast<unsigned char*>(decrypted.data()),
                         &outLen,
                         reinterpret_cast<const unsigned char*>(encryptedData.constData()),
                         static_cast<size_t>(encryptedData.size())) <= 0) {
        qWarning() << "CryptoUtils::rsaDecrypt: decryption failed";
        EVP_PKEY_CTX_free(pctx);
        return {};
    }

    EVP_PKEY_CTX_free(pctx);
    decrypted.resize(static_cast<int>(outLen));

    return decrypted;
}

// ---------------------------------------------------------------------------
// Random Bytes
// ---------------------------------------------------------------------------
QByteArray CryptoUtils::randomBytes(int size)
{
    if (size <= 0) {
        return {};
    }

    QByteArray result;
    result.resize(size);

    if (RAND_bytes(reinterpret_cast<unsigned char*>(result.data()),
                   static_cast<int>(size)) != 1) {
        qWarning() << "CryptoUtils::randomBytes: RAND_bytes failed";
        return {};
    }

    return result;
}

// ---------------------------------------------------------------------------
// Generate AES-256 Key (32 bytes)
// ---------------------------------------------------------------------------
QByteArray CryptoUtils::generateAesKey()
{
    return randomBytes(32);
}

// ---------------------------------------------------------------------------
// Generate IV for AES-GCM (12 bytes)
// ---------------------------------------------------------------------------
QByteArray CryptoUtils::generateIv()
{
    return randomBytes(12);
}

} // namespace crypto
} // namespace chatroom
