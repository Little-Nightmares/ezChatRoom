#ifndef USER_H
#define USER_H

#include <QString>

namespace server {

struct User
{
    int id = 0;
    QString username;
    QString passwordHash;
    QString avatar;
    QString status;
};

} // namespace server

#endif // USER_H
