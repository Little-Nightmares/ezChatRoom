# ChatRoom 项目骨架生成计划

## 概述

基于已上传的项目结构说明文档，使用 **Qt 6.11.0 + CMake + C++17** 生成完整的局域网即时通讯系统项目骨架。所有文件仅包含基本文件头、空实现或骨架代码，不编写详细业务逻辑。每个源文件（.h / .qml）旁附带同名 .md 说明文件。

## 当前状态

- 已有完整的项目结构说明文档（已上传）
- 用户确认使用 **CMake** 作为构建系统（替代文档中的 qmake）
- 目标路径：`/workspace/ChatRoom/`

## 文件统计

| 分类 | 文件数 |
|------|--------|
| 项目根目录 (CMake + 文档 + .gitignore) | 6 |
| 客户端 C++ (CMake + 26 对 .h/.cpp + 26 .md) | 79 |
| 客户端 QML (22 .qml + 22 .md) | 44 |
| 客户端资源与配置 (qrc + .gitkeep + conf) | 8 |
| 服务端 C++ (CMake + 21 对 .h/.cpp + 21 .md) | 64 |
| **合计** | **~201** |

## 关键设计决策

1. **命名空间**：客户端 `namespace client {}`，服务端 `namespace server {}`
2. **头文件保护宏**：全大写类名 + `_H`，如 `APPMANAGER_H`
3. **.md 说明文件命名**：`.h.md`（C++ 头文件旁）、`.qml.md`（QML 文件旁）
4. **QML 组件继承**：pages → `Page`，components → `Item`，dialogs → `Dialog`
5. **服务端 models**：纯 `struct` 数据结构；客户端 models：继承 `QAbstractListModel`
6. **CMake Qt 模块**：客户端 `Core/Quick/Network/Sql`，服务端 `Core/Network/Sql/Concurrent`

## 执行步骤

### 步骤 1：创建全部目录结构（约 30 个目录）

```
ChatRoom/
├── docs/
├── client/src/{core,network,crypto,database,models,qmlbridge}/
├── client/qml/{pages,components,dialogs}/
├── client/resources/images/{icons,avatars,emojis,placeholders}/
├── client/resources/{fonts,sounds}/
├── client/conf/
└── server/src/{core,database,crypto,models}/
```

### 步骤 2：生成项目根目录文件

- `CMakeLists.txt` — 顶层 CMake，C++17，add_subdirectory(client/server)
- `README.md` — 项目简介、功能特性、技术栈、快速开始
- `.gitignore` — 构建产物、IDE 文件、OS 文件、Qt 生成文件
- `docs/项目结构说明.md` — 基于已上传文档
- `docs/协议文档.md` — 协议占位文档
- `docs/部署说明.md` — 部署占位文档

### 步骤 3：生成客户端 CMake 与入口

- `client/CMakeLists.txt` — qt_add_executable(ChatRoomClient)，列出所有源文件和 Qt 模块
- `client/src/main.cpp` — QGuiApplication + QQmlApplicationEngine 基本骨架
- `client/src/main.cpp.md`

### 步骤 4：批量生成客户端 C++ 文件（6 个子目录，26 个类 × 3 文件 = 78 个文件）

每个类生成 `.h`（头文件保护宏 + 类声明）、`.cpp`（空构造/析构）、`.h.md`（职责说明）：

| 子目录 | 类名 | 继承 |
|--------|------|------|
| core/ | AppManager, SessionManager, NavigationController, SettingsManager | QObject |
| network/ | TcpClient, MessageHandler, PacketBuilder, HeartbeatManager | QObject |
| crypto/ | AESEncryptor, RSAEncryptor, HashHelper, KeyManager | QObject |
| database/ | DatabaseManager, MessageDAO, UserDAO, ConversationDAO | QObject |
| models/ | UserModel, MessageModel, ConversationModel, FriendRequestModel | QAbstractListModel |
| qmlbridge/ | AppCore, ChatController, UserController, FriendController, FileTransferController, NotificationController | QObject + QML_ELEMENT |

### 步骤 5：批量生成客户端 QML 文件（22 个 .qml + 22 .md = 44 个文件）

| 子目录 | 文件 | 根元素 |
|--------|------|--------|
| qml/ | main.qml | ApplicationWindow |
| pages/ | LoginPage, RegisterPage, ChatPage, FriendRequestPage, ProfilePage, GroupChatCreatePage, SettingsPage | Page |
| components/ | MessageBubble, ContactItem, ConversationItem, InputBar, Avatar, SearchBar, TabBar, Toast, DialogHelper, ImagePreviewer | Item |
| dialogs/ | AddFriendDialog, GroupChatDialog, FileTransferDialog, ConfirmDialog | Dialog |

### 步骤 6：生成客户端资源与配置文件

- `client/resources/qml.qrc` — 注册所有 QML 文件
- `client/resources/images/{icons,avatars,emojis,placeholders}/.gitkeep` — 占位
- `client/resources/{fonts,sounds}/.gitkeep` — 占位
- `client/conf/config.ini` — 服务器地址、端口、自动登录配置
- `client/conf/logger.conf` — 日志级别、输出路径配置

### 步骤 7：生成服务端 CMake 与入口

- `server/CMakeLists.txt` — qt_add_executable(ChatRoomServer)
- `server/src/main.cpp` — QCoreApplication 基本骨架
- `server/src/main.cpp.md`

### 步骤 8：批量生成服务端 C++ 文件（4 个子目录，21 个类 × 3 文件 = 63 个文件）

| 子目录 | 类名 | 类型 |
|--------|------|------|
| core/ | TcpServer, ClientSession, MessageRouter, ConnectionPool, ThreadPool, SessionManager | class (QTcpServer/QObject) |
| database/ | DatabasePool, UserRepository, MessageRepository, FriendRepository, GroupRepository, OfflineMessageManager | class (QObject) |
| crypto/ | AESEncryptor, RSAEncryptor, AuthHelper, PasswordHasher | class (QObject) |
| models/ | User, Message, FriendRelation, Group, FriendRequest | struct |

## 文件模板规范

### C++ .h 模板（客户端 QObject）
```cpp
#ifndef CLASSNAME_H
#define CLASSNAME_H

#include <QObject>

namespace client {

class ClassName : public QObject
{
    Q_OBJECT
public:
    explicit ClassName(QObject *parent = nullptr);
    ~ClassName() override;
};

} // namespace client
#endif // CLASSNAME_H
```

### C++ .h 模板（服务端 struct）
```cpp
#ifndef STRUCTNAME_H
#define STRUCTNAME_H

#include <QString>

namespace server {

struct StructName {
    int id = 0;
    // fields...
};

} // namespace server
#endif // STRUCTNAME_H
```

### C++ .cpp 模板
```cpp
#include "ClassName.h"

namespace client {

ClassName::ClassName(QObject *parent) : QObject(parent) {}
ClassName::~ClassName() {}

} // namespace client
```

### QML 模板
```qml
import QtQuick
import QtQuick.Controls

Page {  // 或 Item / Dialog / ApplicationWindow
    id: root
    // TODO: 实现具体功能
}
```

### .md 说明文件模板
```markdown
# ClassName
## 所属模块
client/src/core/
## 职责
简短描述该类/文件的职责。
## 主要接口
(待实现)
## 依赖关系
- Qt Core
```

## 验证步骤

1. 检查所有 30 个目录均已创建
2. 检查文件总数约 201 个
3. 验证 3 个 CMakeLists.txt 语法正确
4. 确认每个 .h 文件有头文件保护宏和正确命名空间
5. 确认每个 .h/.qml 文件旁有对应 .md 说明文件
6. 确认 qml.qrc 列出所有 QML 文件
