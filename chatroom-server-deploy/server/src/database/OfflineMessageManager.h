#ifndef OFFLINEMESSAGEMANAGER_H
#define OFFLINEMESSAGEMANAGER_H

#include <QObject>


namespace server {

class OfflineMessageManager : public QObject
{
    Q_OBJECT

public:
    explicit OfflineMessageManager(QObject *parent = nullptr);
    ~OfflineMessageManager() override;

signals:

public slots:

private:
};

} // namespace server

#endif // OFFLINEMESSAGEMANAGER_H
