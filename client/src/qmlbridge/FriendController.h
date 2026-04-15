#ifndef FRIENDCONTROLLER_H
#define FRIENDCONTROLLER_H

#include <QObject>
#include <QtQml/qqmlregistration.h>


namespace client {

class FriendController : public QObject
{
    Q_OBJECT

public:
    explicit FriendController(QObject *parent = nullptr);
    ~FriendController() override;

signals:

public slots:

private:

};

} // namespace client

#endif // FRIENDCONTROLLER_H
