#ifndef GROUP_H
#define GROUP_H

#include <QString>

namespace server {

struct Group
{
    int id = 0;
    QString name;
    int creatorId = 0;
    QString avatar;
};

} // namespace server

#endif // GROUP_H
