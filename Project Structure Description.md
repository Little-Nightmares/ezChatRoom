# 局域网即时通讯 - 项目结构说明

## 项目概述

这是一个基于 **Qt + C++** 构建的局域网即时通讯系统，采用 **客户端-服务器（C/S）架构**，支持账号登录、文字聊天、好友管理、消息加密和离线留言功能。

### 技术栈

| 技术    | 版本       | 用途                  |
| ------- | ---------- | --------------------- |
| Qt/QML  | 6.x / 5.15 | GUI 框架、网络通信    |
| C++     | C++17      | 核心业务逻辑          |
| OpenSSL | 3.x        | 消息加密（AES / RSA） |
| SQLite  | 3.x        | 本地聊天记录存储      |
| JSON    | -          | 数据交换格式          |



---

## 目录结构

```
ChatRoom/
├── client/                     # 客户端代码目录
│   ├── src/                    # C++ 核心代码
│   │   ├── main.cpp
│   │   ├── core/
│   │   ├── network/
│   │   ├── crypto/
│   │   ├── database/
│   │   ├── models/            # C++ Model
│   │   └── qmlbridge/         # QML 桥接层
│   ├── qml/                    # QML 界面
│   │   ├── main.qml
│   │   ├── components/
│   │   └── pages/
│   ├── resources/
│   │   ├── qml.qrc            # QML资源打包
│   │   ├── images/
│   │   └── fonts/
│   ├── conf/
│   └── client.pro        # Qt 项目文件
├── server/                     # 服务端代码目录
│   ├── src/                   # 服务端源代码
│   │   ├── main.cpp           # 服务端入口
│   │   ├── core/              # 核心逻辑（连接、消息路由）
│   │   ├── database/          # 服务端数据库（用户、离线消息）
│   │   ├── crypto/            # 服务端加密模块
│   │   └── models/            # 数据模型
│   ├── server.pro             # Qt 项目文件
├── docs/                       # 项目文档
│   ├── 项目结构说明.md
│   ├── 协议文档.md
│   └── 部署说明.md
├── README.md                   # 项目说明
└── .gitignore                  # Git 配置
```

---

# 项目结构说明

[TOC]

------

## ==客户端代码目录==

### client/src/

客户端 C++ 源代码目录

- **main.cpp**：客户端入口，初始化 QML 引擎、注册 C++ 类型、加载主界面

### client/src/core/

客户端核心逻辑模块：

- **AppManager**：应用管理器，控制应用生命周期、全局状态
- **SessionManager**：会话管理器，管理登录状态、用户会话、Token 维护
- **NavigationController**：页面导航控制器，管理 QML 页面跳转逻辑
- **SettingsManager**：配置管理器，读写本地配置文件（服务器地址、自动登录等）

### client/src/network/

网络通信模块：

- **TcpClient**：管理与服务端的 TCP 连接，负责数据收发、重连机制
- **MessageHandler**：解析和分发服务端消息，根据消息类型调用相应处理函数
- **PacketBuilder**：协议包构建器，按照自定义协议打包发送数据
- **HeartbeatManager**：心跳管理器，定时发送心跳包维持连接

### client/src/crypto/

加密模块：

- **AESEncryptor**：消息内容加密，使用 AES-256-GCM 模式
- **RSAEncryptor**：密钥交换，使用 RSA 加密传输 AES 密钥
- **HashHelper**：密码哈希、消息摘要生成（SHA-256）
- **KeyManager**：密钥管理器，存储和管理会话密钥

### client/src/database/

本地数据库操作：

- **DatabaseManager**：SQLite 数据库管理器，负责数据库初始化、连接管理
- **MessageDAO**：消息数据访问对象，增删改查本地聊天记录
- **UserDAO**：用户数据访问对象，管理联系人信息、用户资料
- **ConversationDAO**：会话数据访问对象，管理会话列表、未读计数

### client/src/models/

C++ 数据模型（继承 QAbstractListModel 或 QAbstractItemModel），暴露给 QML 使用：

- **UserModel**：用户/联系人列表模型，支持搜索、排序、分组
- **MessageModel**：聊天消息模型，管理单个会话的消息列表
- **ConversationModel**：会话列表模型，管理所有聊天会话
- **FriendRequestModel**：好友请求列表模型，管理待处理的好友申请

### client/src/qmlbridge/

C++ 与 QML 桥接层，暴露业务逻辑给 QML 界面：

- **AppCore**：应用核心单例，管理整体状态、页面导航、全局事件
- **ChatController**：聊天控制器，处理消息发送、接收、历史记录加载、消息撤回
- **UserController**：用户控制器，处理登录、登出、用户信息更新、头像上传
- **FriendController**：好友控制器，处理添加好友、删除好友、好友请求响应
- **FileTransferController**：文件传输控制器，处理文件发送、接收、进度更新
- **NotificationController**：通知控制器，管理系统通知、消息提醒

### client/qml/

QML 界面文件，负责 UI 呈现：

- **main.qml**：应用入口，加载初始页面，管理全局状态

#### client/qml/pages/

页面级组件：

- **LoginPage.qml**：登录页面，包含账号密码输入、服务器地址配置
- **RegisterPage.qml**：注册页面，新用户注册、信息填写
- **ChatPage.qml**：主聊天页面，包含会话列表、聊天区域、用户信息面板
- **FriendRequestPage.qml**：好友请求处理页面，展示待处理的好友申请
- **ProfilePage.qml**：个人资料页面，查看和编辑个人信息
- **GroupChatCreatePage.qml**：创建群聊页面，选择群成员、设置群信息
- **SettingsPage.qml**：设置页面，应用配置、主题切换、账号管理

#### client/qml/components/

可复用 UI 组件：

- **MessageBubble.qml**：消息气泡组件，支持文字、图片、文件类型
- **ContactItem.qml**：联系人列表项组件，显示头像、昵称、状态
- **ConversationItem.qml**：会话列表项组件，显示最后一条消息、未读计数
- **InputBar.qml**：消息输入栏组件，支持文本输入、表情、文件发送
- **Avatar.qml**：头像组件，支持圆形显示、默认头像、加载状态
- **SearchBar.qml**：搜索栏组件，支持实时搜索、清空输入
- **TabBar.qml**：底部标签栏组件，切换消息、联系人、个人中心页面
- **Toast.qml**：提示框组件，轻提示消息显示
- **DialogHelper.qml**：对话框辅助组件，统一管理确认框、输入框弹窗
- **ImagePreviewer.qml**：图片预览器，支持缩放、保存、转发

#### client/qml/dialogs/

对话框组件：

- **AddFriendDialog.qml**：添加好友对话框，输入好友账号、验证信息
- **GroupChatDialog.qml**：创建群聊对话框，设置群名称、头像、邀请成员
- **FileTransferDialog.qml**：文件传输对话框，显示传输进度、历史记录
- **ConfirmDialog.qml**：通用确认对话框，自定义标题、内容、按钮

### client/resources/

资源文件目录：

- **qml.qrc**：QML 资源文件注册表，将所有 QML 文件打包到资源系统
- **images/**：图标、头像、表情包等图片资源
  - **icons/**：功能图标（发送、添加、设置等）
  - **avatars/**：默认头像、系统头像
  - **emojis/**：表情包资源
  - **placeholders/**：占位图、加载动画
- **fonts/**：自定义字体文件（如有需要）
- **sounds/**：消息提示音、通知音效（可选）

### client/conf/

配置文件目录：

- **config.ini**：客户端配置文件，包含服务器地址、端口、自动登录设置
- **logger.conf**：日志配置文件，日志级别、输出路径

### client/client.pro

Qt 项目文件，包含模块依赖、源文件列表、包含路径、编译选项

## ==服务端代码目录==

### server/src/

服务端 C++ 源代码目录

### server/src/main.cpp

服务端入口，初始化数据库、启动 TCP 服务器、加载配置

### server/src/core/

服务端核心逻辑：

- **TcpServer**：TCP 服务器，监听客户端连接、管理所有客户端会话
- **ClientSession**：客户端会话管理，维护每个连接的客户端状态
- **MessageRouter**：消息路由器，根据消息类型分发到对应的处理器
- **ConnectionPool**：连接池管理，管理数据库连接复用
- **ThreadPool**：线程池，处理并发请求
- **SessionManager**：会话管理器，管理在线用户、心跳检测

### server/src/database/

服务端数据库操作：

- **DatabasePool**：数据库连接池，管理 MySQL/PostgreSQL 连接
- **UserRepository**：用户仓储，用户注册、登录验证、用户信息查询
- **MessageRepository**：消息仓储，存储聊天记录、离线消息管理
- **FriendRepository**：好友仓储，好友关系管理、好友请求处理
- **GroupRepository**：群组仓储，群组信息、群成员管理
- **OfflineMessageManager**：离线消息管理器，存储和分发离线消息

### server/src/crypto/

服务端加密模块：

- **AESEncryptor**：消息内容加密解密
- **RSAEncryptor**：RSA 密钥对生成、私钥解密
- **AuthHelper**：身份验证辅助，Token 生成与验证
- **PasswordHasher**：密码哈希存储，使用 bcrypt 或 Argon2

### server/src/models/

服务端数据模型：

- **User**：用户模型，用户 ID、用户名、密码哈希、头像、状态
- **Message**：消息模型，消息 ID、发送者、接收者、内容、时间、类型
- **FriendRelation**：好友关系模型，用户 ID、好友 ID、关系状态、备注
- **Group**：群组模型，群组 ID、群名、创建者、成员列表
- **FriendRequest**：好友请求模型，请求 ID、发送者、接收者、验证信息、状态

### server/server.pro

Qt 项目文件，包含模块依赖、源文件列表、编译选项

## ==项目文档==

### docs/项目结构说明.md

本文档，详细描述项目目录结构和模块职责

### docs/协议文档.md

自定义通信协议文档，包含：

- 协议格式说明（包头、包体、校验）
- 消息类型定义（登录、聊天、文件传输、心跳等）
- 请求响应流程说明
- 错误码定义

### docs/部署说明.md

部署指南，包含：

- 服务端环境要求（操作系统、数据库版本）
- 编译步骤
- 配置文件说明
- 启动命令
- 常见问题排查

### README.md

项目说明，包含项目简介、功能特性、快速开始、贡献指南

### .gitignore

Git 版本控制忽略文件配置，忽略编译产物、临时文件、配置文件（含敏感信息）





---

## ==功能模块说明==

### 1. 账号密码登录
- 客户端发送登录请求（用户名 + 密码哈希）
- 服务端验证用户信息
- 返回登录结果 + 好友列表

### 2. 正常文字聊天
- 支持点对点消息发送
- 消息内容使用 AES 加密
- 服务端转发消息，不持久化

### 3. 好友添加
- 支持通过用户名搜索好友
- 发送好友请求
- 同意/拒绝好友请求

### 4. 聊天加密
- 使用 RSA 交换 AES 密钥
- 每条消息使用 AES-256-GCM 加密
- 服务端无法解密消息内容

### 5. 离线留言
- 若好友不在线，消息暂存服务端
- 好友上线后自动推送
- 离线消息保留 7 天

---

## ==开发命令==

### 客户端
```bash
cd client
qmake client.pro
make
./client
```

### 服务端
```bash
cd server
qmake server.pro
make
./server
```

---

## ==部署说明==

### 服务端部署
1. 编译服务端程序
2. 配置数据库（SQLite 或 MySQL）
3. 运行服务端，默认监听端口 `6667`

### 客户端部署
1. 编译客户端程序
2. 修改配置文件中的服务器地址
3. 分发可执行文件

---

## ==扩展建议==

- **多人聊天**：可扩展群组功能，服务端维护群组消息分发
- **文件传输**：基于 UDP 或额外 TCP 连接实现 P2P 文件传输
- **云备份**：可选将聊天记录同步到云端

