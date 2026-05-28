# Default Group Fix Plan

## Problem
Default group "局域网大群" exists in database but users aren't shown as members. The auto-join code exists in `handleLogin()` but not in `registerUser()`. When a user registers, they are NOT added to the default group.

## Changes Required

### Change 1: ServerDatabase::registerUser()
File: `server/src/database/ServerDatabase.cpp`

After successful registration (after line 202, before `return 0;`), add auto-join to default group:
```cpp
    // Auto-join the new user to the default group (LAN group)
    uint64_t userId = static_cast<uint64_t>(query.lastInsertId().toULongLong());
    if (userId > 0) {
        GroupInfo defaultGroup = getDefaultGroup();
        if (defaultGroup.groupId > 0 && !isGroupMember(defaultGroup.groupId, userId)) {
            addGroupMember(defaultGroup.groupId, userId);
        }
    }
```

### Change 2: MessageRouter::handleRegister()
File: `server/src/core/MessageRouter.cpp`

After successful registration (inside `if (result == 0)` block), add a log message indicating the user was registered. The auto-join now happens inside registerUser() itself.

### Prerequisite
Client must be rebuilt (initialization order fix already applied in AppCore.h).

## Verification
1. Build server
2. Clear database (delete chatroom_server.db)
3. Start server → default group created automatically
4. Register a new user → should be auto-joined to default group
5. Log in → group list should show the default group
