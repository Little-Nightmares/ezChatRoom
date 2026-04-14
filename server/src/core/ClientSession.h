#ifndef CLIENTSESSION_H
#define CLIENTSESSION_H

#include <QObject>


namespace server {

class ClientSession : public QObject
{
    Q_OBJECT

public:
    explicit ClientSession(QObject *parent = nullptr);
    ~ClientSession() override;

signals:

public slots:

private:
};

} // namespace server

#endif // CLIENTSESSION_H
