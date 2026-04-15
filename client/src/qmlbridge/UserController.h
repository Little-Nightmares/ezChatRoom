#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include <QObject>


namespace client {

class UserController : public QObject
{
    Q_OBJECT

public:
    explicit UserController(QObject *parent = nullptr);
    ~UserController() override;

signals:

public slots:

private:

};

} // namespace client

#endif // USERCONTROLLER_H
