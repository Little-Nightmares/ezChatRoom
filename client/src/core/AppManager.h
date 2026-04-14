#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>


namespace client {

class AppManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit AppManager(QObject *parent = nullptr);
    ~AppManager() override;

signals:

public slots:

private:

};

} // namespace client

#endif // APPMANAGER_H
