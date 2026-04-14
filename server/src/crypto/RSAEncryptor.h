#ifndef RSAENCRYPTOR_H
#define RSAENCRYPTOR_H

#include <QObject>


namespace server {

class RSAEncryptor : public QObject
{
    Q_OBJECT

public:
    explicit RSAEncryptor(QObject *parent = nullptr);
    ~RSAEncryptor() override;

signals:

public slots:

private:
};

} // namespace server

#endif // RSAENCRYPTOR_H
