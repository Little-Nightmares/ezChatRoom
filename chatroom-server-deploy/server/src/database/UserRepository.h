#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include <QObject>


namespace server {

class UserRepository : public QObject
{
    Q_OBJECT

public:
    explicit UserRepository(QObject *parent = nullptr);
    ~UserRepository() override;

signals:

public slots:

private:
};

} // namespace server

#endif // USERREPOSITORY_H
