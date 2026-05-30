#ifndef FRIENDREPOSITORY_H
#define FRIENDREPOSITORY_H

#include <QObject>


namespace server {

class FriendRepository : public QObject
{
    Q_OBJECT

public:
    explicit FriendRepository(QObject *parent = nullptr);
    ~FriendRepository() override;

signals:

public slots:

private:
};

} // namespace server

#endif // FRIENDREPOSITORY_H
