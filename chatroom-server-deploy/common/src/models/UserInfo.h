#ifndef CHATROOM_MODELS_USER_INFO_H
#define CHATROOM_MODELS_USER_INFO_H

#include <QString>

#include <cstdint>

namespace chatroom {
namespace models {

struct UserInfo {
    uint64_t userId   = 0;
    QString  username;
    QString  nickname;
    QString  avatar;
    bool     isOnline = false;

    UserInfo() = default;

    UserInfo(uint64_t uid, const QString& uname, const QString& nick,
             const QString& avatarUrl, bool online = false)
        : userId(uid)
        , username(uname)
        , nickname(nick)
        , avatar(avatarUrl)
        , isOnline(online)
    {}
};

} // namespace models
} // namespace chatroom

#endif // CHATROOM_MODELS_USER_INFO_H
