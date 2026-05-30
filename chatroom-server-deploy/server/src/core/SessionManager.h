#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>


namespace server {

class SessionManager : public QObject
{
    Q_OBJECT

public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager() override;

signals:

public slots:

private:
};

} // namespace server

#endif // SESSIONMANAGER_H
