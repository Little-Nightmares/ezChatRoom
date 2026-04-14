#ifndef FRIENDREQUEST_H
#define FRIENDREQUEST_H

#include <QString>

namespace server {

struct FriendRequest
{
    int id = 0;
    int senderId = 0;
    int receiverId = 0;
    QString message;
    QString status;
};

} // namespace server

#endif // FRIENDREQUEST_H
