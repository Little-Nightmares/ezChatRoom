#ifndef AUTHHELPER_H
#define AUTHHELPER_H

#include <QObject>


namespace server {

class AuthHelper : public QObject
{
    Q_OBJECT

public:
    explicit AuthHelper(QObject *parent = nullptr);
    ~AuthHelper() override;

signals:

public slots:

private:
};

} // namespace server

#endif // AUTHHELPER_H
