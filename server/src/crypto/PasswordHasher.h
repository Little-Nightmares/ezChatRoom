#ifndef PASSWORDHASHER_H
#define PASSWORDHASHER_H

#include <QObject>


namespace server {

class PasswordHasher : public QObject
{
    Q_OBJECT

public:
    explicit PasswordHasher(QObject *parent = nullptr);
    ~PasswordHasher() override;

signals:

public slots:

private:
};

} // namespace server

#endif // PASSWORDHASHER_H
