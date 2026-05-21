# ChatRoom — LAN 局域网聊天室

> 基于 Qt6 + C++17 的局域网即时通讯应用，支持私聊、群聊、文件传输、端到端加密、Bot 集成、Web 管理后台。

---

## 目录

1. [项目概述](#1-项目概述)
2. [技术栈](#2-技术栈)
3. [项目结构](#3-项目结构)
4. [架构设计](#4-架构设计)
5. [功能清单](#5-功能清单)
6. [开发历程](#6-开发历程)
7. [协议说明](#7-协议说明)
8. [数据库设计](#8-数据库设计)
9. [安全机制](#9-安全机制)
10. [构建与部署](#10-构建与部署)
11. [常见问题](#11-常见问题)

---

## 1. 项目概述

ChatRoom 是一个面向局域网（LAN）环境的即时通讯系统，采用 C/S 架构。服务端使用 Qt6 开发基于 TCP 的聊天服务，并附加 HTTP 管理后台；客户端使用 Qt6 QML 提供现代化的桌面 UI。

### 核心设计目标

| 目标 | 说明 |
|---|---|
| 🚀 **零配置** | 局域网内开箱即用，无需公网服务器 |
| 🔒 **隐私优先** | 端到端加密，服务端不存储明文消息 |
| 🤖 **可扩展** | 支持 AstrBot 集成，可自定义 AI 助手 |
| 🖥️ **可管理** | Web 管理后台，运营友好 |

---

## 2. 技术栈

### 客户端

| 技术 | 版本 | 用途 |
|---|---|---|
| Qt 6.11.0 (MinGW 13.1.0) | 6.11 | 应用框架 |
| QML / Qt Quick | 6.11 | UI 界面 |
| Qt Quick Controls 2 | 6.11 | 控件库（Fusion 样式） |
| Qt Quick Layouts | 6.11 | 布局管理 |
| Qt SQL (SQLite 3) | 6.11 | 本地数据持久化 |
| Qt Network | 6.11 | TCP 通信 |
| Qt Multimedia | 6.11 | 音效通知 |

### 服务端

| 技术 | 版本 | 用途 |
|---|---|---|
| Qt 6.11.0 (MinGW 13.1.0) | 6.11 | 应用框架 |
| Qt Core | 6.11 | 事件循环、定时器 |
| Qt Network | 6.11 | TCP 服务 + HTTP 管理 |
| Qt SQL (SQLite 3) | 6.11 | 数据库 |
| OpenSSL 3.x | 3.x | 加密（RSA/AES） |
| Qt Network (HTTP Client) | 6.11 | 调用 AstrBot API |

### 公共库

| 技术 | 用途 |
|---|---|
| OpenSSL EVP | AES-256-GCM 加解密 |
| OpenSSL RSA | RSA-2048 密钥交换 |
| SQLite 3 | 服务端/客户端数据库 |
| Google Fonts | 内嵌 11 款字体（中英文配对） |
| Chart.js (Admin) | 管理后台图表（已规划） |

---

## 3. 项目结构

```
ChatRoom/
├── client/                        # Qt 桌面客户端
│   ├── CMakeLists.txt
│   ├── main.cpp                   # 入口，QFontDatabase 注册字体 + Fusion 样式
│   ├── qml/                       # QML UI
│   │   ├── main.qml               # 主窗口（StackView 页面路由）
│   │   ├── pages/
│   │   │   ├── LoginPage.qml      # 登录页
│   │   │   ├── RegisterPage.qml   # 注册页
│   │   │   ├── ChatPage.qml       # 主聊天页（私聊/群聊）
│   │   │   ├── SettingsPage.qml   # 设置页（主题/字体/背景）
│   │   │   └── FriendRequestPage.qml # 好友请求页
│   │   ├── components/
│   │   │   ├── StyledText.qml     # 带主题字体的 Text
│   │   │   ├── MessageBubble.qml  # 私聊消息气泡
│   │   │   ├── GroupMessageBubble.qml # 群聊消息气泡
│   │   │   ├── InputBar.qml       # 输入栏（含表情/文件按钮）
│   │   │   ├── EmojiPicker.qml    # 表情选择器（内联于 InputBar）
│   │   │   ├── Avatar.qml         # 头像组件
│   │   │   ├── ContactItem.qml    # 联系人列表项
│   │   │   ├── ConversationItem.qml  # 会话列表项
│   │   │   ├── GroupConversationItem.qml # 群聊会话项
│   │   │   ├── SearchBar.qml      # 搜索栏
│   │   │   └── Toast.qml          # 消息提示
│   │   └── dialogs/
│   │       ├── ConfirmDialog.qml
│   │       ├── AddFriendDialog.qml
│   │       ├── CreateGroupDialog.qml
│   │       └── GroupInfoDialog.qml
│   ├── src/
│   │   ├── core/                  # 核心逻辑
│   │   │   ├── AppManager.cpp/h   # 应用管理器（DB/TCP/心跳/重连）
│   │   │   └── SessionManager.cpp/h # 会话管理器
│   │   ├── network/               # 网络层
│   │   │   ├── TcpClient.cpp/h    # TCP 客户端（含自动重连）
│   │   │   ├── MessageHandler.cpp/h # 消息分发
│   │   │   └── HeartbeatManager.cpp/h # 心跳保活
│   │   ├── database/              # 本地数据库
│   │   │   └── DatabaseManager.cpp/h
│   │   ├── models/                # 数据模型
│   │   │   ├── MessageModel.cpp/h
│   │   │   ├── ConversationModel.cpp/h
│   │   │   ├── UserModel.cpp/h
│   │   │   ├── GroupModel.cpp/h
│   │   │   └── FriendRequestModel.cpp/h
│   │   └── qmlbridge/             # C++ ↔ QML 桥接
│   │       ├── AppCore.cpp/h      # 核心控制器（暴露给 QML）
│   │       ├── ChatController.cpp/h  # 聊天逻辑
│   │       ├── UserController.cpp/h  # 用户逻辑
│   │       ├── FriendController.cpp/h # 好友逻辑
│   │       ├── GroupController.cpp/h # 群组逻辑
│   │       └── ThemeManager.cpp/h  # 主题管理
│   └── resources/
│       ├── fonts/                 # 11 款 Google Fonts
│       └── sounds/                # 音效
│
├── server/                        # Qt 服务端
│   ├── CMakeLists.txt
│   ├── main.cpp                   # 入口
│   ├── src/
│   │   ├── core/
│   │   │   ├── TcpServer.cpp/h    # TCP 服务
│   │   │   ├── ClientSession.cpp/h# 客户端会话
│   │   │   ├── MessageRouter.cpp/h# 消息路由（所有消息处理）
│   │   │   └── BotService.cpp/h   # AstrBot 集成
│   │   ├── database/
│   │   │   └── ServerDatabase.cpp/h # 服务端数据库
│   │   ├── crypto/
│   │   │   └── PasswordHasher.cpp/h # 密码哈希
│   │   └── admin/
│   │       ├── AdminHttpServer.cpp/h # HTTP 管理服务
│   │       └── index.html         # 管理后台前端
│   └── admin.qrc                  # 管理页面资源
│
├── common/                        # 公共库（静态链接）
│   ├── CMakeLists.txt
│   ├── src/
│   │   ├── protocol/
│   │   │   ├── ChatProtocol.cpp/h # 协议编解码
│   │   │   └── MessageTypes.h     # 消息类型枚举
│   │   ├── crypto/
│   │   │   └── CryptoUtils.cpp/h  # AES/RSA 加密
│   │   └── models/
│   │       ├── ChatMessage.h
│   │       ├── UserInfo.h
│   │       └── FriendRequest.h
│   └── CMakeLists.txt
│
├── openssl/                       # 预编译 OpenSSL 库
├── build/                         # 构建目录
├── release/                       # 发布包
├── start_client.bat               # 客户端启动脚本
├── start_server.bat               # 服务端启动脚本
├── admin.token                    # 管理后台 Token
├── chatroom_server.db             # 服务端数据库（运行时生成）
└── CMakeLists.txt                 # 顶层 CMake
```

---

## 4. 架构设计

### 4.1 网络架构

```
┌──────────────────┐         TCP (6667)         ┌──────────────────┐
│                  │ ◄─────────────────────────► │                  │
│  ChatRoomClient  │                             │  ChatRoomServer  │
│  (Qt6 QML)       │                             │  (Qt6 Console)   │
│                  │                             │                  │
└──────────────────┘                             └────────┬─────────┘
                                                          │
                                                          │ HTTP (19527)
                                                          │
                                                          ▼
                                                 ┌──────────────────┐
                                                 │  AstrBot Server  │
                                                 │  (Python)         │
                                                 └──────────────────┘
```

### 4.2 通信协议

```
┌─────────────────────────────────────────────────────┐
│                  Packet Header (20B)                 │
├─────────┬──────┬──────┬─────┬──────┬────────┬───────┤
│  Magic  │ Ver  │ Type │Flag │Resv  │  Body  │  Seq  │
│  (4B)   │ (1B) │ (1B) │(1B) │ (1B) │  Len   │  (4B) │
│         │      │      │     │      │  (4B)  │       │
├─────────┴──────┴──────┴─────┴──────┴────────┴───────┤
│  Timestamp (4B)                                      │
├──────────────────────────────────────────────────────┤
│  Body (variable, max 10MB)                           │
└──────────────────────────────────────────────────────┘
```

- Magic: `0x4348524D` ("CHRM")
- Version: `0x01`
- 消息类型: 100+ 种（登录、注册、聊天、群操作、文件等）
- 最大 Body: 10MB

### 4.3 线程模型

```
主线程（Qt Event Loop）
├── QTcpServer（accept 连接）
├── ClientSession × N（每个连接一个对象，非阻塞 I/O）
├── AdminHttpServer（HTTP 管理，SSE 推送）
├── BotService（异步 HTTP 调用 AstrBot）
└── QTimer 定时任务（心跳、重连、统计缓存）
```

**单线程事件驱动模型** — 所有网络 I/O 和业务逻辑在同一个事件循环中处理。无需线程同步，简化开发复杂度。

### 4.4 数据库架构

**服务端** (`chatroom_server.db`)

| 表 | 用途 |
|---|---|
| `users` | 用户账号信息 |
| `friendships` | 好友关系 |
| `friend_requests` | 好友请求 |
| `offline_messages` | 离线消息（登录后清空） |
| `chat_messages` | 私聊消息持久化 |
| `groups` | 群组信息 |
| `group_members` | 群成员 |
| `group_messages` | 群聊消息 |
| `group_message_reads` | 群消息已读记录 |
| `group_announcements` | 群公告 |
| `group_encryption_keys` | 群加密密钥 |
| `file_metadata` | 文件元数据 |
| `server_settings` | 服务器设置 |
| `messages_fts` / `group_messages_fts` / `chat_messages_fts` | FTS5 全文搜索 |

**客户端** (`%APPDATA%/ChatRoom/ChatRoom/chatroom_client.db`)

| 表 | 用途 |
|---|---|
| `messages` | 本地消息缓存 |
| `conversations` | 会话列表 |
| `users` | 用户缓存 |
| `friend_requests` | 好友请求缓存 |
| `session_keys` | 会话密钥 |
| `key_pairs` | RSA 密钥对 |
| `user_settings` | 用户设置（背景等） |

---

## 5. 功能清单

### 5.1 基础功能

| 功能 | 状态 | 说明 |
|---|---|---|
| ✅ 用户注册/登录 | 完成 | 用户名+密码，SHA-256 哈希 |
| ✅ 好友搜索/添加 | 完成 | 模糊搜索用户名 |
| ✅ 好友接受/拒绝 | 完成 | 请求通知 |
| ✅ 好友列表/在线状态 | 完成 | 绿点指示 |
| ✅ 好友删除 | 完成 | 双向解除 |
| ✅ 私聊消息 | 完成 | 纯文本 |
| ✅ 消息撤回 | 完成 | 2 分钟内可撤回 |
| ✅ 离线消息推送 | 完成 | 登录后推送未读消息 |
| ✅ 心跳保活 | 完成 | 30s 间隔，3 次超时 |
| ✅ 自动重连 | 完成 | 指数退避（1s~30s） |

### 5.2 群聊

| 功能 | 状态 | 说明 |
|---|---|---|
| ✅ 群组创建/解散 | 完成 | 群主可解散 |
| ✅ 邀请成员 | 完成 | 选择好友加入 |
| ✅ 群聊消息 | 完成 | 实时广播 |
| ✅ 群公告 | 完成 | 群主发布 |
| ✅ 消息已读 | 完成 | 记录已读 |
| ✅ 转让群主 | 完成 | 群主可转让 |
| ✅ 修改群名 | 完成 | 群主可修改 |
| ✅ 默认大群 | 完成 | 局域网用户自动加入 |

### 5.3 文件传输

| 功能 | 状态 | 说明 |
|---|---|---|
| ✅ 文件上传 | 完成 | Base64 编码传输 |
| ✅ 文件下载 | 完成 | 权限校验 |
| ✅ 大文件分块 | 完成 | 分块传输 |
| ✅ 图片预览 | 完成 | 气泡内直接显示 |
| ✅ 缩略图 | 完成 | PreserveAspectFit |

### 5.4 消息搜索

| 功能 | 状态 | 说明 |
|---|---|---|
| ✅ 私聊搜索 | 完成 | 本地客户端数据库 |
| ✅ 群聊搜索 | 完成 | 服务端 FTS5 |
| ✅ 关键词高亮 | 部分 | LIKE 片段提取 |
| ✅ 搜索结果跳转 | 完成 | 点击定位到消息 |

### 5.5 安全与加密

| 功能 | 状态 | 说明 |
|---|---|---|
| ✅ 端到端加密 | 完成 | AES-256-GCM + RSA-2048 |
| ✅ 密钥交换 | 完成 | RSA 加密传输 AES 密钥 |
| ✅ 群聊加密 | 完成 | 群共享密钥 |
| ✅ 密码哈希 | 完成 | SHA-256 |
| ✅ 管理 Token 认证 | 完成 | Bearer Token |

### 5.6 主题与 UI

| 功能 | 状态 | 说明 |
|---|---|---|
| ✅ 5 套色彩主题 | 完成 | 樱花/森林/极光/抹茶/海洋 |
| ✅ 3 套字体 | 完成 | 手写体/标准体/圆润体 |
| ✅ Fusion 样式 | 完成 | 跨平台统一外观 |
| ✅ 表情选择器 | 完成 | 100+ Unicode 表情 |
| ✅ 聊天背景 | 完成 | 纯色/自定义图片 |
| ✅ 消息气泡自适应 | 完成 | implicitWidth 动态大小 |

### 5.7 Bot 与集成

| 功能 | 状态 | 说明 |
|---|---|---|
| ✅ AstrBot 对接 | 完成 | @bot 调用 AstrBot API |
| ✅ 异步调用 | 完成 | 不阻塞消息处理 |
| ✅ Web 配置 | 完成 | 管理后台设置 AstrBot URL |
| ✅ 使用说明 | 完成 | 内嵌在管理页面 |

### 5.8 管理后台

| 功能 | 状态 | 说明 |
|---|---|---|
| ✅ 概览仪表盘 | 完成 | 用户/在线/群组/消息统计 |
| ✅ 用户管理 | 完成 | 列表/删除 |
| ✅ 群组管理 | 完成 | 列表/解散 |
| ✅ 消息日志 | 完成 | 最近消息查看 |
| ✅ 系统设置 | 完成 | AstrBot 地址配置 |

---

## 6. 开发历程

### v0.1 — 基础架构（2026.03）

- 搭建 CMake 构建系统
- 实现 TCP 协议编解码 (`ChatProtocol`)
- 实现服务端 `TcpServer` + `ClientSession`
- 实现客户端 `TcpClient`
- 实现用户注册/登录
- 实现私聊消息收发

### v0.2 — 好友与群聊（2026.04）

- 好友系统（搜索/添加/接受/拒绝/删除）
- 好友在线状态通知
- 群组 CRUD（创建/加入/退出/解散）
- 群聊消息广播
- 离线消息推送

### v0.3 — 安全与加密（2026.04）

- RSA-2048 密钥对生成
- AES-256-GCM 端到端加密
- 密钥交换协议
- 消息撤回

### v0.4 — 主题系统（2026.05）

- 5 套色彩主题 + 3 套字体
- `ThemeManager` C++ 类
- `StyledText` QML 组件
- 设置页 UI
- Fusion 样式统一
- QFontDatabase 注册字体

### v0.5 — 界面重构（2026.05）

- 设置页布局修复（ScrollView）
- 消息气泡动态大小（implicitWidth/Height）
- 表情选择器（内联 Popup）
- 群聊默认群显示修复
- 管理页面 admin.qrc 修复

### v0.6 — 功能扩展（2026.05）

- 文件传输（上传/下载/分块）
- 消息搜索（FTS5 + LIKE 回退）
- 群公告/已读状态
- 在线状态强化
- 消息回复

### v0.7 — Bot 集成（2026.05）

- BotService（异步 AstrBot 调用）
- Web 管理后台 AstrBot URL 配置
- 使用说明内嵌
- 聊天背景（纯色/图片）
- 数据看板 API 扩展

### v0.8 — 质量修复（2026.05）

- 15 项审查 Bug 修复
- 私聊消息持久化（chat_messages 表）
- 文件下载权限校验
- Bot 用户 ID 初始化
- 响应类型规范化
- 协议 bodyLength 安全检查

---

## 7. 协议说明

### 7.1 消息类型分类

| 范围 | 类别 | 数量 |
|---|---|---|
| `0x01-0x02` | 心跳 | 2 |
| `0x10-0x14` | 认证 | 5 |
| `0x20-0x30` | 好友 | 17 |
| `0x31-0x35` | 聊天 | 5 |
| `0x40-0x43` | 加密/撤回 | 4 |
| `0x50-0x51` | 头像 | 2 |
| `0x60-0x7E` | 群组 | 31 |
| `0x80-0x85` | 文件 | 6 |
| `0x90-0x91` | 搜索 | 2 |
| `0x94-0x99` | 群增强 | 6 |
| `0xA0-0xA1` | 消息特性 | 2 |

### 7.2 消息格式

```json
// 登录请求
{
    "username": "yuan",
    "passwordHash": "a1b2c3d4..."
}

// 登录响应  
{
    "success": true,
    "userId": 1,
    "username": "yuan",
    "nickname": "Yuan",
    "avatar": "avatars/1.png"
}

// 聊天消息
{
    "content": "你好",
    "targetUserId": 2,
    "encrypted": false,
    "timestamp": 1746000000
}

// 文件上传
{
    "fileName": "photo.jpg",
    "fileSize": 1048576,
    "mimeType": "image/jpeg",
    "fileData": "/9j/4AAQ...",
    "targetUserId": 2
}
```

---

## 8. 数据库设计

### 8.1 核心表关系

```
users ──┬── friendships ──── users（好友关系）
        ├── friend_requests ── users（好友请求）
        ├── offline_messages（离线消息）
        ├── chat_messages（私聊消息持久化）
        ├── group_members ─── groups（群组成员）
        ├── file_metadata（文件记录）
        └── group_encryption_keys（群密钥）
```

### 8.2 关键索引

| 表 | 索引 | 用途 |
|---|---|---|
| `messages` (client) | `idx_messages_time` | 按时间排序 |
| `group_messages` | `idx_group_messages_group_id_id` | 群消息查询 |
| `chat_messages` | `idx_chat_messages_sender_receiver` | 搜索 + 时间排序 |
| `group_members` | `UNIQUE(group_id, user_id)` | 防重复加入 |

---

## 9. 安全机制

### 9.1 传输安全

| 措施 | 说明 |
|---|---|
| TCP 直连 | LAN 内通信，不经过公网 |
| 消息加密 | AES-256-GCM 端到端加密 |
| 密钥交换 | RSA-2048 加密传输 AES 密钥 |
| 协议 Magic | 4 字节魔数校验非协议数据 |

### 9.2 身份安全

| 措施 | 说明 |
|---|---|
| 密码存储 | SHA-256 哈希，不存明文 |
| 会话认证 | TCP 连接保持，登录后标记已认证 |
| 管理 Token | Bearer Token 认证所有 API 请求 |

### 9.3 数据安全

| 措施 | 说明 |
|---|---|
| SQLite WAL | 崩溃安全写入 |
| PSK 完整性 | 预共享密钥 + 数据库持久化 |
| bodyLength 上限 | 10MB 防止内存耗尽 |

### 9.4 已知限制

| 限制 | 说明 |
|---|---|
| 无 TLS | LAN 环境，TCP 直连未加密（E2E 加密补偿） |
| 无速率限制 | 未实现请求频率控制 |
| 无文件类型验证 | 头像上传未校验文件魔数 |
| 无消息编辑 | 发送后不可编辑（可撤回） |

---

## 10. 构建与部署

### 10.1 开发环境

| 组件 | 版本 |
|---|---|
| Qt | 6.11.0 (MinGW 13.1.0 64-bit) |
| CMake | 3.30+ |
| MinGW | 13.1.0 64-bit |
| OpenSSL | 3.x (预编译于 `openssl/` 目录) |

### 10.2 构建步骤

```bash
# 1. 创建构建目录
mkdir build/release && cd build/release

# 2. 配置
cmake -G "MinGW Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="C:/Qt/6.11.0/mingw_64" \
  -DOPENSSL_ROOT_DIR="../../openssl" \
  -DOPENSSL_INCLUDE_DIR="C:/Program Files/OpenSSL-Win64/include" \
  ../..

# 3. 构建
mingw32-make -j4 ChatRoomServer ChatRoomClient
```

### 10.3 打包部署

```bash
# 1. 复制 exe 到发布目录
# 2. 运行 windeployqt 收集依赖
windeployqt6 --release release/ChatRoomClient/ChatRoomClient.exe
windeployqt6 --release release/ChatRoomServer/ChatRoomServer.exe
# 3. 复制 OpenSSL DLL
# 4. 打包 ZIP
```

### 10.4 运行

```bash
# 启动服务端
start_server.bat

# 启动客户端
start_client.bat

# 管理后台
# 浏览器打开 http://localhost:19527
# Token 在 admin.token 文件中
```

### 10.5 发布包结构

```
ChatRoom-Package/
├── start_server.bat
├── start_client.bat
├── client/
│   ├── ChatRoomClient.exe
│   ├── Qt6*.dll
│   ├── platform sqlimageformats styles
│   └── translations/
└── server/
    ├── ChatRoomServer.exe
    ├── Qt6*.dll
    ├── libcrypto-3-x64.dll
    ├── libssl-3-x64.dll
    └── admin.token
```

---

## 11. 常见问题

### 11.1 客户端闪退

**可能原因**: 未重新构建。QML 文件编译在 exe 中，修改后需要 `Ctrl+B` 构建。

### 11.2 看不到默认群

**可能原因**: 
1. 旧数据库中没有群成员记录 → 重新登录会自动加入
2. 删库后需要重建 → 服务端启动时自动创建

### 11.3 字体切换无效

**可能原因**: 旧版本使用 `QGuiApplication::setFont()` 无法动态更新。v0.4 后改用 `StyledText` + `fontFamily` 属性绑定。

### 11.4 Bot 不回复

**检查项**:
1. 管理后台 `⚙️ 系统设置` 中是否正确配置了 AstrBot URL
2. AstrBot 服务是否正常运行
3. 群聊消息是否包含 `@bot`

### 11.5 消息空白

**可能原因**: `textFormat: Text.RichText` 导致 HTML 标签被解析。v0.6 后已回退为纯文本。

---

> 文档版本: v0.8 | 最后更新: 2026.05 | 项目状态: 稳定
