# ChatRoom - 局域网即时通讯系统

## 项目简介
基于 Qt 6 + C++17 构建的局域网即时通讯系统，采用 C/S 架构。

## 功能特性
- 账号密码登录/注册
- 实时文字聊天
- 好友管理
- 消息加密 (AES-256 + RSA)
- 离线留言

## 技术栈
- Qt 6.11.0 / QML
- C++17
- CMake 构建系统
- SQLite (客户端) / MySQL (服务端)
- OpenSSL 3.x

## 快速开始
### 编译
```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.11.0/gcc_64
cmake --build .
```

### 运行
```bash
./client/ChatRoomClient
./server/ChatRoomServer
```

## 项目结构
详见 [docs/项目结构说明.md](docs/项目结构说明.md)

## 贡献指南
(待补充)
