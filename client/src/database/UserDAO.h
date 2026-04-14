#ifndef USERDAO_H
#define USERDAO_H

#include <QObject>


namespace client {

class UserDAO : public QObject
{
    Q_OBJECT

public:
    explicit UserDAO(QObject *parent = nullptr);
    ~UserDAO() override;

signals:

public slots:

private:

};

} // namespace client

#endif // USERDAO_H
