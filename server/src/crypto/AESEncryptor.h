#ifndef AESENCRYPTOR_H
#define AESENCRYPTOR_H

#include <QObject>


namespace server {

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

} // namespace server

#endif // AESENCRYPTOR_H
