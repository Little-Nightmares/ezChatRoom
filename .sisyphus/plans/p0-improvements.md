# P0 关键改进计划

> 基于对整个项目的审查（服务端 1289 行 + MessageRouter 1787 行 + 客户端 20+ QML 文件），以下为最关键的缺失功能与改进项。

---

## 1. 图片/文件传输 (Critical Missing Feature)

**现状**: 协议中定义了 `ChatImage`, `ChatFile`, `AvatarUploadRequest/Response` 等消息类型，但客户端 QML 中没有完整的文件发送/接收 UI 和流程。

**需要实现**:

### 服务端
- `MessageRouter.cpp`: 添加 `handleFileUpload`, `handleFileDownload` 处理器
- 文件存储：专用文件目录 + 数据库记录文件元信息
- 添加消息类型 `FileUploadRequest/Response`, `FileDownloadRequest/Response`

### 客户端
- `InputBar.qml`: 添加文件/图片发送按钮（📎 📷）
- 图片预览：`Image` 元素渲染收到的图片
- 文件下载：点击文件消息下载到本地
- `MessageBubble.qml`: 支持图片和文件类型渲染（`type: 1`, `type: 2`）

**参考**: `ChatProtocol.h` 中已有的消息类型枚举。

**验收标准**:
- [ ] 用户 A 能发送图片给用户 B
- [ ] 用户 B 收到图片并显示在聊天中
- [ ] 用户 A 能发送文件给用户 B
- [ ] 用户 B 能下载收到的文件
- [ ] 大文件分段传输（>10MB）

---

## 2. 消息历史持久化与搜索 (Core Usability)

**现状**: 数据库有 `messages` 表存储聊天记录，`ConversationModel` 也加载历史。但消息搜索功能尚未完善，消息加载存在分页问题。

### 服务端
- `ServerDatabase.cpp`: 添加 `searchMessages(keyword, userId, limit, offset)` 方法
- `MessageRouter.cpp`: 添加 `handleMessageSearch` 处理器
- 添加消息类型 `MessageSearchRequest/Response`

### 客户端
- `ChatPage.qml` 已有一个 `searchVisible` 搜索栏（第 507-541 行），但搜索功能未实际连接到服务端
- 需要实现消息搜索 API 调用和结果展示
- 搜索结果高亮显示

### 数据库
- 为 `messages` 表的 `content` 列添加 FTS（全文搜索）索引：
```sql
CREATE VIRTUAL TABLE messages_fts USING fts5(content, content=messages, content_rowid=rowid);
```

**验收标准**:
- [ ] 用户能搜索历史消息关键词
- [ ] 搜索结果在聊天列表中高亮
- [ ] 搜索支持中英文
- [ ] 搜索结果分页加载

---

## 3. 群聊功能完善 (Group Chat)

**现状**: 已有群聊基本框架（创建、列表、发送消息）。但缺少：

### 缺失功能
1. **群聊消息已读状态** — 谁读了消息？
2. **群公告** — @所有人 功能
3. **群文件共享** — 群内文件库
4. **成员列表实时更新** — 成员加入/离开通知
5. **群聊置顶** — 重要群聊置顶

### 客户端
- `GroupInfoDialog.qml`: 已有成员列表（第 100-195 行），但缺少：
  - 成员昵称显示
  - 成员角色标识（群主/管理员/成员）
  - 成员列表排序（群主置顶）
- `GroupMessageBubble.qml`: 群聊消息已有发送者名称（第 38-43 行），但缺少 @提及 支持

**验收标准**:
- [ ] 创建群聊时可选群头像
- [ ] 群聊成员列表实时刷新
- [ ] 群主可设置管理员
- [ ] 群聊消息已读/未读状态

---

## 4. 连接稳定性与重连 (Reliability)

**现状**: `HeartbeatManager` 已实现心跳（30 秒间隔）。服务端有 `reconnectSuccess` 信号。重连逻辑存在缺陷。

### 问题
- 之前的崩溃日志显示 `TcpClient: disconnected from server` 后进程退出
- 重连时不会重新订阅消息处理器
- 断线后 UI 状态未恢复（会话列表清空）

### 客户端改进
```cpp
// TcpClient.cpp 需要改进：
// 1. 指数退避重连（1s, 2s, 4s, 8s, 16s, 30s max）
// 2. 重连成功后重新登录
// 3. 重连成功后重新请求好友列表/群列表
// 4. 重连状态 UI 提示（已有 reconnectBar 但需改进）
```

### 服务端改进
- 会话过期时间：超过 60 秒无心跳自动断开
- 重复登录处理：同一账号新登录时踢掉旧会话（已有 `ServerKickNotify` 但需完善）

**验收标准**:
- [ ] 服务器重启后客户端自动重连
- [ ] 重连后聊天历史不丢失
- [ ] 重连过程中有 UI 提示
- [ ] 网络断开后聊天输入不丢失（暂存草稿）

---

## 5. 表情与富文本消息 (User Experience)

**现状**: 消息只支持纯文本。

### 需要实现
- 表情选择器：QQ/微信风格的表情面板
- 支持 Unicode Emoji 快捷输入（`:smile:` → 😊）
- 消息中的 URL 自动识别为可点击链接
- 消息引用/回复：右键消息 → "回复"
- 消息撤回完善：`MessageBubble.qml` 已有撤回菜单（第 20-62 行），但撤回后其他客户端未同步

### 客户端
- `InputBar.qml`: 添加表情按钮，点击弹出表情面板
- `MessageBubble.qml`: URL 自动识别为链接

**验收标准**:
- [ ] 发送表情（至少 50 个常用表情）
- [ ] 消息中的链接可点击打开
- [ ] 可回复/引用指定消息
- [ ] 消息撤回后所有客户端同步删除

---

## 6. 端到端加密完善 (Security)

**现状**: 代码中有完整的 RSA + AES-GCM 加密实现（`CryptoUtils`, `KeyExchangeRequest/Response`），但：

### 问题
- 加密仅在私聊中可选启用
- 群聊消息未加密
- 密钥交换 UI 不可见
- 数据库存储的聊天记录未加密

### 需要改进
- 群聊会话密钥：群主生成 AES 密钥，用每个成员的 RSA 公钥加密分发
- 本地数据库加密：使用 `SQLCipher` 或 Qt 的加密存储
- 端到端加密状态指示器：锁图标显示加密状态

**验收标准**:
- [ ] 私聊默认启用端到端加密
- [ ] 群聊消息使用群共享密钥加密
- [ ] UI 上清晰显示加密状态（🔒 / 🔓）
- [ ] 本地数据库加密存储

---

## 实施优先级

```
P0-紧急:  1. 图片/文件传输  ← 最明显的功能缺失
P0-紧急:  2. 消息历史搜索   ← 日常使用必需
P0-紧急:  3. 群聊功能完善   ← 默认群已有，但功能残缺
P0-紧急:  4. 连接稳定性     ← 影响所有用户
P1-重要:  5. 表情与富文本   ← 提升聊天体验
P1-重要:  6. 端到端加密     ← 隐私保护
```

## 技术债务

- `AppCore.h` 成员初始化顺序（已修复）
- 字体主题系统使用 `QFontDatabase` 替代 QML `FontLoader`（已修复）
- 设置页布局添加 `ScrollView`（已修复）
- 管理页面 `admin.qrc` 未编译进 exe（已修复）
- 数据库迁移机制缺失：后续 schema 变更需要版本管理
- 日志系统不完善：无日志轮转，调试日志在生产环境仍输出
