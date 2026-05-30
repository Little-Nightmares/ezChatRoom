#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>

namespace server {

struct Message
{
    int id = 0;
    int senderId = 0;
    int receiverId = 0;
    QString content;
    QString timestamp;
    QString type;
};

} // namespace server

#endif // MESSAGE_H
