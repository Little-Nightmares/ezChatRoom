#ifndef HEARTBEATMANAGER_H
#define HEARTBEATMANAGER_H

#include <QObject>
#include <QTimer>

namespace client {

class HeartbeatManager : public QObject
{
    Q_OBJECT

public:
    explicit HeartbeatManager(QObject *parent = nullptr);
    ~HeartbeatManager() override;

signals:

public slots:

private:

};

} // namespace client

#endif // HEARTBEATMANAGER_H
