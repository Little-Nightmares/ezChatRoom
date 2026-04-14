#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include <QObject>


namespace server {

class ConnectionPool : public QObject
{
    Q_OBJECT

public:
    explicit ConnectionPool(QObject *parent = nullptr);
    ~ConnectionPool() override;

signals:

public slots:

private:
};

} // namespace server

#endif // CONNECTIONPOOL_H
