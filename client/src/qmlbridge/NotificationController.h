#ifndef NOTIFICATIONCONTROLLER_H
#define NOTIFICATIONCONTROLLER_H

#include <QObject>


namespace client {

class NotificationController : public QObject
{
    Q_OBJECT

public:
    explicit NotificationController(QObject *parent = nullptr);
    ~NotificationController() override;

signals:

public slots:

private:

};

} // namespace client

#endif // NOTIFICATIONCONTROLLER_H
