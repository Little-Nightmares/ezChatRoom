#ifndef AESENCRYPTOR_H
#define AESENCRYPTOR_H

#include <QObject>


namespace client {

class AESEncryptor : public QObject
{
    Q_OBJECT

public:
    explicit AESEncryptor(QObject *parent = nullptr);
    ~AESEncryptor() override;

signals:

public slots:

private:

};

} // namespace client

#endif // AESENCRYPTOR_H
