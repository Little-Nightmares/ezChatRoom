#ifndef MESSAGEROUTER_H
#define MESSAGEROUTER_H

#include <QObject>


namespace server {

class MessageRouter : public QObject
{
    Q_OBJECT

public:
    explicit MessageRouter(QObject *parent = nullptr);
    ~MessageRouter() override;

signals:

public slots:

private:
};

} // namespace server

#endif // MESSAGEROUTER_H
