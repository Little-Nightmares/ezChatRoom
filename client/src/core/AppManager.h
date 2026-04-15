#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QtQml/qqmlregistration.h>


namespace client {

class AppManager : public QObject
{
    Q_OBJECT

public:
    explicit AppManager(QObject *parent = nullptr);
    ~AppManager() override;

signals:

public slots:

private:

};

} // namespace client

#endif // APPMANAGER_H
