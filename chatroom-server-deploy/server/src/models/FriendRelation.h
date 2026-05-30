#ifndef FRIENDRELATION_H
#define FRIENDRELATION_H

#include <QString>

namespace server {

struct FriendRelation
{
    int userId = 0;
    int friendId = 0;
    QString status;
    QString remark;
};

} // namespace server

#endif // FRIENDRELATION_H
