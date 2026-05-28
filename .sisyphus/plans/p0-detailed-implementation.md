# P0 全部功能详细实施计划

## 目录
1. [图片/文件传输](#1-图片文件传输)
2. [消息历史搜索](#2-消息历史搜索)
3. [群聊功能完善](#3-群聊功能完善)
4. [连接稳定性与重连](#4-连接稳定性与重连)
5. [表情与富文本消息](#5-表情与富文本消息)
6. [端到端加密完善](#6-端到端加密完善)

---

## 总体执行策略

```
Wave 1 (并行基础 — 协议扩展 + 数据库):
├── T1: 协议 MessageTypes 枚举扩展
├── T2: 数据库 schema 扩展 (FTS + file_metadata)
└── T3: 服务端文件存储目录结构

Wave 2 (并行核心逻辑):
├── T4: 服务端文件上传/下载处理器
├── T5: 服务端消息搜索处理器
├── T6: 服务端群聊改进（公告/已读）
├── T7: 客户端 TcpClient 重连逻辑
└── T8: 加密密钥管理改进

Wave 3 (并行客户端 UI):
├── T9: 客户端图片/文件发送 UI
├── T10: 客户端图片/文件接收展示
├── T11: 客户端消息搜索 UI
├── T12: 客户端群聊 UI 改进
├── T13: 表情选择器
├── T14: 端到端加密 UI 指示器
└── T15: 断线重连 UI

Wave 4 (集成测试 + 验收):
├── T16: 端到端测试 (图片/文件)
├── T17: 端到端测试 (搜索/群聊/重连)
└── T18: 性能测试 + Bug 修复
```

---

## 1. 图片/文件传输

### 概述
在纯文本聊天基础上增加图片和文件发送/接收功能。服务端负责中转文件数据，客户端提供发送和展示 UI。

### 涉及文件

| 文件 | 改动类型 | 说明 |
|---|---|---|
| `common/src/protocol/MessageTypes.h` | 修改 | 添加 `FileUploadReq/Rsp`, `FileDownloadReq/Rsp` |
| `server/src/core/MessageRouter.h` | 修改 | 添加 `handleFileUpload`, `handleFileDownload` |
| `server/src/core/MessageRouter.cpp` | 修改 | 实现文件上传/下载处理器 |
| `server/src/database/ServerDatabase.h` | 修改 | 添加 `saveFileMeta`, `getFileMeta`, `deleteFile` |
| `server/src/database/ServerDatabase.cpp` | 修改 | 实现文件元数据存储 |
| `server/CMakeLists.txt` | 修改 | 创建文件存储目录 |
| `client/src/core/AppManager.h` | 修改 | 添加文件传输接口 |
| `client/src/qmlbridge/ChatController.h` | 修改 | 添加 `sendFile`, `downloadFile` |
| `client/src/qmlbridge/ChatController.cpp` | 修改 | 实现文件发送/下载 |
| `client/qml/components/MessageBubble.qml` | 修改 | 添加图片/文件渲染 |
| `client/qml/components/GroupMessageBubble.qml` | 修改 | 添加图片/文件渲染 |
| `client/qml/components/InputBar.qml` | 修改 | 添加文件/图片选择按钮 |
| `client/qml/components/FileTransferProgress.qml` | **新建** | 文件传输进度条组件 |

### 详细 TODO

- [ ] 1.1. 扩展消息类型枚举

  **What to do**: 在 `common/src/protocol/MessageTypes.h` 中添加：
  ```cpp
  FileUploadRequest     = 0x50,
  FileUploadResponse    = 0x51,
  FileDownloadRequest   = 0x52,
  FileDownloadResponse  = 0x53,
  GroupFileUploadNotify = 0x54,
  ```

  **Acceptance Criteria**:
  - [ ] 枚举值不与其他类型冲突
  - [ ] 编译通过

- [ ] 1.2. 服务端文件元数据表

  **What to do**: 在 `ServerDatabase::createTables()` 中添加：
  ```sql
  CREATE TABLE IF NOT EXISTS file_metadata (
      file_id     INTEGER PRIMARY KEY AUTOINCREMENT,
      sender_id   INTEGER NOT NULL,
      receiver_id INTEGER,
      group_id    INTEGER DEFAULT 0,
      file_name   TEXT NOT NULL,
      file_size   INTEGER NOT NULL,
      file_path   TEXT NOT NULL,
      mime_type   TEXT DEFAULT '',
      sha256_hash TEXT DEFAULT '',
      created_at  INTEGER NOT NULL,
      FOREIGN KEY (sender_id) REFERENCES users(user_id)
  );
  ```

  添加方法：
  - `uint64_t saveFileMeta(...)` → 返回 file_id
  - `FileMeta getFileMeta(uint64_t fileId)` → 返回文件信息
  - `QString getFilePath(uint64_t fileId)` → 返回存储路径

- [ ] 1.3. 服务端文件存储目录

  **What to do**: 服务器启动时创建 `uploads/` 目录，按日期分子目录：
  ```
  uploads/
  ├── 2026-05/
  │   ├── images/
  │   └── files/
  └── avatars/
  ```

  在 `server/main.cpp` 中添加：
  ```cpp
  QDir().mkpath("uploads/images");
  QDir().mkpath("uploads/files");
  QDir().mkpath("uploads/avatars");
  ```

- [ ] 1.4. 服务端文件上传处理器

  **What to do**: 在 `MessageRouter.cpp` 添加 `handleFileUpload`：
  ```cpp
  void MessageRouter::handleFileUpload(ClientSession* session, uint32_t seq, const QByteArray& body) {
      // 1. 解析 JSON：{fileName, fileSize, mimeType, fileData(base64), targetUserId/groupId}
      // 2. 验证文件大小（限制 50MB）
      // 3. 保存文件到 uploads/files/ 目录
      // 4. 计算 SHA-256
      // 5. 保存文件元数据到 file_metadata 表
      // 6. 如果是私聊：发送 FileUploadResponse 给发送者 + FileUploadNotify 给接收者
      // 7. 如果是群聊：发送 FileUploadResponse 给发送者 + GroupFileUploadNotify 给所有成员
  }
  ```

  **大文件处理**：Base64 编码会使大小增加 33%。对于 >5MB 的文件，分块传输：
  ```cpp
  struct FileChunk {
      uint64_t fileId;
      uint32_t chunkIndex;
      uint32_t totalChunks;
      QByteArray data;  // 每个 chunk 最大 1MB
  };
  ```

- [ ] 1.5. 服务端文件下载处理器

  **What to do**: 在 `MessageRouter.cpp` 添加 `handleFileDownload`：
  ```cpp
  void MessageRouter::handleFileDownload(ClientSession* session, uint32_t seq, const QByteArray& body) {
      // 1. 解析 JSON：{fileId}
      // 2. 查询 file_metadata 获取文件路径
      // 3. 验证接收者有权限访问
      // 4. 读取文件，Base64 编码
      // 5. 发送 FileDownloadResponse
  }
  ```

- [ ] 1.6. 客户端文件发送 UI

  **What to do**: 修改 `InputBar.qml`，在输入框左侧添加两个按钮：
  ```qml
  // 图片按钮
  Button {
      text: "📷"
      onClicked: imagePickerDialog.open()
  }
  // 文件按钮
  Button {
      text: "📎"
      onClicked: filePickerDialog.open()
  }
  ```

  使用 Qt Quick Dialogs 的 `FileDialog`：
  ```qml
  FileDialog {
      id: filePickerDialog
      title: "选择文件"
      onAccepted: {
          var filePath = selectedFile.toString().replace("file:///", "");
          appCore.chatController.sendFile(filePath, currentTargetId);
      }
  }
  ```

- [ ] 1.7. 客户端消息气泡图片/文件渲染

  **What to do**: 在 `MessageBubble.qml` 的 `type` 属性基础上，添加：
  ```qml
  // type === 1 (图片)
  Image {
      id: msgImage
      source: "file:///" + appCore.chatController.getCachedImage(bubble.messageId)
      fillMode: Image.PreserveAspectFit
      width: Math.min(300, implicitWidth)
      height: Math.min(300, implicitHeight)
  }
  
  // type === 2 (文件)
  Rectangle {
      Row {
          Text { text: "📄" }
          Column {
              Text { text: fileName }
              Text { text: fileSize }
          }
          Button { text: "下载" }
      }
  }
  ```

- [ ] 1.8. 文件传输进度组件

  **What to do**: 新建 `FileTransferProgress.qml`：
  ```qml
  Item {
      property string fileName
      property real progress  // 0.0 - 1.0
      property bool isUpload: true
      
      ProgressBar { value: progress }
      Text { text: parent.fileName }
      Text { text: Math.round(progress * 100) + "%" }
  }
  ```

### 验收标准
- [ ] 发送图片（<10MB）→ 对方实时收到并显示
- [ ] 发送文件（任意类型）→ 对方可下载到本地
- [ ] 群聊中发送文件 → 所有成员可见可下载
- [ ] 大文件分块传输（>10MB 自动分块）
- [ ] 文件传输显示进度条
- [ ] 图片支持缩略图预览（<300KB 自动生成缩略图）

### Agent-Executed QA 场景
```
Scenario: 发送并接收图片
  Tool: interactive_bash (启动 server + 两个 client)
  Preconditions: 两个用户已登录且互为好友
  Steps:
    1. 用户 A 点击 📷 按钮，选择 test.png
    2. 用户 B 的聊天窗口中看到图片预览
    3. 点击图片放大查看
  Expected: 图片成功传输并显示

Scenario: 发送并下载文件
  Tool: interactive_bash
  Steps:
    1. 用户 A 点击 📎 按钮，选择 document.pdf
    2. 用户 B 看到文件消息，显示文件名和大小
    3. 用户 B 点击下载，文件保存到本地
  Expected: 文件完整传输，SHA-256 一致
```

---

## 2. 消息历史搜索

### 概述
为聊天消息添加全文搜索功能。用户可在聊天界面搜索历史消息关键词。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `common/src/protocol/MessageTypes.h` | 修改 | 添加 `MsgSearchReq/Rsp` |
| `server/src/core/MessageRouter.h` | 修改 | 添加 `handleMessageSearch` |
| `server/src/core/MessageRouter.cpp` | 修改 | 实现搜索处理器 |
| `server/src/database/ServerDatabase.h` | 修改 | 添加 `searchMessages` |
| `server/src/database/ServerDatabase.cpp` | 修改 | 实现 FTS 搜索 |
| `client/src/qmlbridge/ChatController.h` | 修改 | 添加 `searchMessages` |
| `client/src/qmlbridge/ChatController.cpp` | 修改 | 实现搜索请求/响应处理 |
| `client/qml/pages/ChatPage.qml` | 修改 | 完善搜索栏 + 搜索结果展示 |

### 详细 TODO

- [ ] 2.1. 数据库添加 FTS 索引

  **What to do**: 在 `ServerDatabase::createTables()` 中添加：
  ```sql
  -- 消息全文搜索索引
  CREATE VIRTUAL TABLE IF NOT EXISTS messages_fts USING fts5(
      content,
      content=messages,
      content_rowid=message_id,
      tokenize='unicode61'
  );
  
  -- 触发器：插入消息时同步到 FTS
  CREATE TRIGGER IF NOT EXISTS messages_fts_insert AFTER INSERT ON messages
  BEGIN
      INSERT INTO messages_fts(rowid, content) VALUES (new.message_id, new.content);
  END;
  
  -- 同样的触发器用于 group_messages
  CREATE VIRTUAL TABLE IF NOT EXISTS group_messages_fts USING fts5(
      content,
      content=group_messages,
      content_rowid=msg_id,
      tokenize='unicode61'
  );
  ```

- [ ] 2.2. 服务端消息搜索方法

  **What to do**: 在 `ServerDatabase.cpp` 添加：
  ```cpp
  struct SearchResult {
      uint64_t messageId;
      uint64_t senderId;
      QString senderName;
      QString content;
      QString snippet;  // 匹配上下文片段
      QDateTime timestamp;
      bool isGroup;
      uint64_t groupId;
      QString groupName;
  };
  
  QVector<SearchResult> searchMessages(uint64_t userId, const QString& keyword,
                                       int limit = 50, int offset = 0) {
      // 私聊搜索：搜索用户参与的所有私聊消息
      // 使用 FTS5: SELECT ... FROM messages_fts WHERE content MATCH :keyword
      // JOIN messages + 会话权限检查
      
      // 群聊搜索：搜索用户所在群聊的消息
      // 同样使用 FTS5
  }
  ```

- [ ] 2.3. 服务端搜索处理器

  **What to do**: 在 `MessageRouter.cpp` 添加：
  ```cpp
  void MessageRouter::handleMessageSearch(ClientSession* session, uint32_t seq, const QByteArray& body) {
      // 1. 解析：{keyword, scope: "private"|"group"|"all", limit, offset}
      // 2. 调用 m_db->searchMessages(userId, keyword, limit, offset)
      // 3. 返回搜索结果数组
  }
  ```

- [ ] 2.4. 客户端搜索 UI 完善

  **What to do**: 修改 `ChatPage.qml` 第 507-541 行的搜索栏：
  ```qml
  // 搜索输入
  TextField {
      id: searchInput
      placeholderText: "搜索消息..."
      onAccepted: appCore.chatController.searchMessages(text)
      onTextChanged: {
          if (text.length === 0) appCore.chatController.clearSearch()
      }
  }
  
  // 搜索结果列表（覆盖在消息列表上方）
  ListView {
      id: searchResultsList
      visible: searchResultsModel.count > 0
      model: appCore.chatController.searchResultsModel
      delegate: SearchResultItem {
          content: model.content
          snippet: model.snippet
          senderName: model.senderName
          timestamp: model.timestamp
          onClicked: {
              // 跳转到该消息所在位置
              messageListView.positionViewAtIndex(model.messageIndex, ListView.Contain)
          }
      }
  }
  ```

- [ ] 2.5. 搜索结果高亮

  **What to do**: 创建 `SearchResultItem.qml` 组件，搜索结果中关键词高亮显示：
  ```qml
  Rectangle {
      Text {
          text: "<b>" + senderName + "</b> " + snippet
          textFormat: Text.RichText
      }
      Rectangle { // 高亮背景
          visible: isHighlighted
          color: "#fff3cd"
      }
  }
  ```

### 验收标准
- [ ] 输入关键词搜索历史消息
- [ ] 搜索结果按时间排序（最近优先）
- [ ] 搜索结果显示消息片段 + 发送者
- [ ] 点击搜索结果跳转到对应消息位置
- [ ] 支持中文分词搜索
- [ ] 搜索时消息列表滚动位置不丢失

---

## 3. 群聊功能完善

### 概述
完善群聊功能：群公告、成员角色管理、@提及、消息已读状态。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `server/src/database/ServerDatabase.h` | 修改 | 添加公告/已读/角色方法 |
| `server/src/database/ServerDatabase.cpp` | 修改 | 实现群聊增强功能 |
| `server/src/core/MessageRouter.h` | 修改 | 添加群公告/@处理器 |
| `server/src/core/MessageRouter.cpp` | 修改 | 实现群聊增强处理器 |
| `client/qml/dialogs/GroupInfoDialog.qml` | 修改 | 添加公告/角色管理 UI |
| `client/qml/components/GroupMessageBubble.qml` | 修改 | 添加 @提及 支持 |
| `client/qml/pages/ChatPage.qml` | 修改 | 群聊信息面板 |
| `common/src/protocol/MessageTypes.h` | 修改 | 添加群公告/@消息类型 |

### 详细 TODO

- [ ] 3.1. 数据库群聊增强表

  **What to do**: 添加表：
  ```sql
  -- 群公告
  CREATE TABLE IF NOT EXISTS group_announcements (
      announcement_id INTEGER PRIMARY KEY AUTOINCREMENT,
      group_id        INTEGER NOT NULL,
      sender_id       INTEGER NOT NULL,
      content         TEXT NOT NULL,
      created_at      INTEGER NOT NULL,
      is_pinned       INTEGER DEFAULT 0,
      FOREIGN KEY (group_id) REFERENCES groups(group_id)
  );
  
  -- 群消息已读记录
  CREATE TABLE IF NOT EXISTS group_message_reads (
      msg_id    INTEGER NOT NULL,
      group_id  INTEGER NOT NULL,
      user_id   INTEGER NOT NULL,
      read_at   INTEGER NOT NULL,
      PRIMARY KEY (msg_id, user_id)
  );
  
  -- 群成员角色（扩展 role 字段：0=成员, 1=群主, 2=管理员）
  -- 已有 role 字段，只需在应用层面扩展
  ```

- [ ] 3.2. 服务端群公告处理器

  **What to do**: 添加消息类型：
  ```cpp
  GroupAnnouncementPush   = 0x73,
  GroupAnnouncementReq    = 0x74,
  GroupAnnouncementResp   = 0x75,
  GroupMessageReadNotify  = 0x76,
  ```

  在 `MessageRouter.cpp` 添加：
  ```cpp
  void handleGroupAnnouncement(ClientSession* session, uint32_t seq, const QByteArray& body);
  void handleGroupMessageRead(ClientSession* session, uint32_t seq, const QByteArray& body);
  ```

- [ ] 3.3. @提及功能

  **What to do**: 
  - 在 `GroupMessageBubble.qml` 中解析 `@username` 格式
  - 被 @ 的用户收到特殊通知
  - 输入框中输入 `@` 弹出成员选择列表
  
  ```qml
  // 输入框 @ 弹出
  Popup {
      id: mentionPopup
      model: groupMemberModel
      delegate: ItemDelegate {
          text: model.nickname
          onClicked: {
              editArea.insert(editArea.cursorPosition, "@" + model.nickname + " ")
              mentionPopup.close()
          }
      }
  }
  ```

- [ ] 3.4. 群聊已读状态

  **What to do**: 
  - 发送 `GroupMessageReadNotify` 到服务端
  - 服务端记录已读状态
  - 群聊消息气泡底部显示已读/未读状态
  ```qml
  // GroupMessageBubble.qml 底部
  Text {
      text: "已读 " + readCount + "/" + totalCount
      font.pixelSize: 10
      color: readCount === totalCount ? appCore.themeManager.success : appCore.themeManager.textTertiary
  }
  ```

- [ ] 3.5. 群聊信息面板增强

  **What to do**: 修改 `GroupInfoDialog.qml`：
  - 添加"群公告"区域
  - 群主可编辑公告
  - 成员列表显示角色标签
  - 群主可设置/取消管理员
  - 显示成员总数

### 验收标准
- [ ] 群主可发布群公告
- [ ] 新成员加入时看到历史公告
- [ ] 输入 @ 弹出成员选择
- [ ] 被 @ 的用户收到通知
- [ ] 消息底部显示已读人数
- [ ] 群主可设置管理员
- [ ] 成员列表显示角色

---

## 4. 连接稳定性与重连

### 概述
改进 TCP 连接稳定性，实现自动重连、断线缓存、状态恢复。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `client/src/network/TcpClient.h` | 修改 | 添加重连逻辑 |
| `client/src/network/TcpClient.cpp` | 修改 | 实现指数退避重连 |
| `client/src/network/HeartbeatManager.h` | 修改 | 添加超时检测 |
| `client/src/network/HeartbeatManager.cpp` | 修改 | 实现心跳超时触发重连 |
| `client/src/core/AppManager.h` | 修改 | 添加重连状态信号 |
| `client/src/core/AppManager.cpp` | 修改 | 实现重连状态管理 |
| `client/src/qmlbridge/ChatController.cpp` | 修改 | 重连后恢复聊天状态 |
| `client/qml/main.qml` | 修改 | 改进重连提示条 |
| `server/src/core/ClientSession.h` | 修改 | 添加会话超时 |
| `server/src/core/ClientSession.cpp` | 修改 | 实现会话超时断开 |

### 详细 TODO

- [ ] 4.1. 客户端指数退避重连

  **What to do**: 在 `TcpClient.cpp` 添加重连逻辑：
  ```cpp
  void TcpClient::startReconnect() {
      if (m_reconnectAttempt >= 5) {
          emit reconnectFailed();
          return;
      }
      
      int delay = 1000 * (1 << m_reconnectAttempt); // 1s, 2s, 4s, 8s, 16s
      delay = qMin(delay, 30000); // max 30s
      
      QTimer::singleShot(delay, this, [this]() {
          m_reconnectAttempt++;
          qDebug() << "Reconnect attempt" << m_reconnectAttempt;
          connectToHost(m_host, m_port);
      });
  }
  
  void TcpClient::onConnected() {
      m_reconnectAttempt = 0;
      emit reconnectSuccess();
  }
  
  void TcpClient::onDisconnected() {
      if (m_reconnectAttempt < 5) {
          startReconnect();
      }
  }
  ```

- [ ] 4.2. 心跳超时检测

  **What to do**: 在 `HeartbeatManager.cpp` 添加：
  ```cpp
  // 添加超时计时器：如果超过 45 秒没收到心跳响应，触发重连
  m_heartbeatTimer->setInterval(30000); // 每 30 秒发一次心跳
  m_timeoutTimer->setInterval(45000);   // 45 秒超时
  
  void HeartbeatManager::onHeartbeatResponse() {
      m_timeoutTimer->start(); // 重置超时计时器
  }
  
  void HeartbeatManager::onTimeout() {
      qWarning() << "Heartbeat timeout, triggering reconnect";
      emit heartbeatTimeout();
      // TcpClient 收到信号后启动重连
  }
  ```

- [ ] 4.3. 断线消息缓存

  **What to do**: 在 `TcpClient.cpp` 添加发送队列：
  ```cpp
  // 断线时，待发送消息入队
  QQueue<PendingMessage> m_pendingQueue;
  
  struct PendingMessage {
      uint8_t type;
      uint8_t flags;
      QByteArray body;
      QDateTime createdAt;
  };
  
  void TcpClient::sendPacket(...) {
      if (m_socket->state() != QAbstractSocket::ConnectedState) {
          // 断线时缓存消息
          m_pendingQueue.enqueue({type, flags, body, QDateTime::currentDateTime()});
          return;
      }
      // 正常发送
  }
  
  void TcpClient::flushPendingQueue() {
      // 重连成功后，发送所有缓存消息
      while (!m_pendingQueue.isEmpty()) {
          auto msg = m_pendingQueue.dequeue();
          sendPacket(msg.type, msg.flags, msg.body);
      }
  }
  ```

- [ ] 4.4. 服务端会话超时

  **What to do**: 在 `ClientSession.cpp` 添加：
  ```cpp
  QTimer* m_heartbeatTimer = new QTimer(this);
  m_heartbeatTimer->setInterval(60000); // 60 秒无活动断开
  m_heartbeatTimer->setSingleShot(true);
  connect(m_heartbeatTimer, &QTimer::timeout, this, [this]() {
      qDebug() << "Session timeout for user" << m_userId;
      m_socket->disconnectFromHost();
  });
  
  void ClientSession::resetHeartbeatTimer() {
      m_heartbeatTimer->start();
  }
  ```

- [ ] 4.5. 重连状态 UI

  **What to do**: 改进 `main.qml` 中的 `reconnectBar`：
  ```qml
  Rectangle {
      id: reconnectBar
      color: appCore.appManager.isReconnecting ? "#fff7e6" : "transparent"
      height: appCore.appManager.isReconnecting ? 32 : 0
      
      Text {
          text: appCore.appManager.reconnectAttempt > 0
              ? "连接断开，正在重连... (第" + appCore.appManager.reconnectAttempt + "次)"
              : "正在连接服务器..."
      }
      
      ProgressBar {
          indeterminate: true
          visible: appCore.appManager.isReconnecting
      }
  }
  ```

### 验收标准
- [ ] 服务器断开后客户端自动检测（心跳超时 < 45 秒）
- [ ] 客户端自动重连（指数退避：1s/2s/4s/8s/16s/30s）
- [ ] 重连成功 UI 恢复（会话列表、好友列表、群列表）
- [ ] 断线期间输入的消息自动补发
- [ ] 服务端 60 秒无活动自动断开僵尸连接
- [ ] 重连过程中有明确的 UI 提示

---

## 5. 表情与富文本消息

### 概述
添加表情发送、URL 自动识别、消息回复功能。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `client/qml/components/InputBar.qml` | 修改 | 添加表情按钮 + 表情面板 |
| `client/qml/components/EmojiPicker.qml` | **新建** | 表情选择器组件 |
| `client/qml/components/MessageBubble.qml` | 修改 | URL 识别 + 回复显示 |
| `client/qml/components/GroupMessageBubble.qml` | 修改 | URL 识别 + 回复显示 |
| `client/qml/components/MessageReplyPreview.qml` | **新建** | 回复预览组件 |
| `common/src/protocol/MessageTypes.h` | 修改 | 添加引用消息类型 |
| `client/src/qmlbridge/ChatController.h` | 修改 | 添加引用消息方法 |
| `client/src/qmlbridge/ChatController.cpp` | 修改 | 实现引用消息发送 |

### 详细 TODO

- [ ] 5.1. 表情选择器

  **What to do**: 新建 `EmojiPicker.qml`：
  ```qml
  Popup {
      id: emojiPicker
      width: 360
      height: 280
      
      GridView {
          model: emojiModel  // 200+ 常用 Unicode 表情
          cellWidth: 36
          cellHeight: 36
          delegate: Item {
              Text {
                  text: model.emoji
                  font.pixelSize: 24
              }
              MouseArea {
                  onClicked: emojiPicked(model.emoji)
              }
          }
      }
      
      // 分类 Tab：😊 表情 / 🎉 庆祝 / 🐱 动物 / 🍔 食物
      Row {
          Repeater {
              model: ["😊", "🎉", "🐱", "🍔"]
              Button { text: modelData }
          }
      }
  }
  ```

- [ ] 5.2. URL 自动识别

  **What to do**: 在 `MessageBubble.qml` 的文本渲染中添加 URL 检测：
  ```qml
  Text {
      id: bubbleText
      text: {
          // 将 URL 转换为可点击链接
          var msg = bubble.content;
          var urlRegex = /(https?:\/\/[^\s]+)/g;
          return msg.replace(urlRegex, '<a href="$1">$1</a>');
      }
      textFormat: Text.RichText  // 启用 HTML
      onLinkActivated: Qt.openUrlExternally(link)
  }
  ```

- [ ] 5.3. 消息回复/引用

  **What to do**: 添加消息类型 `MessageReply`：
  ```cpp
  // 消息格式
  {
      "replyTo": 12345,      // 被回复消息的 ID
      "replyContent": "...", // 被回复消息的内容片段
      "replySender": "...",  // 被回复消息的发送者
      "content": "..."       // 回复内容
  }
  ```

  在 `MessageBubble.qml` 中添加回复预览区域：
  ```qml
  // 被引用的消息预览
  Rectangle {
      visible: replyTo > 0
      color: "#f0f0f0"
      Text { text: replySender + ": " + replyContent }
  }
  ```

### 验收标准
- [ ] 点击 😊 按钮弹出表情面板
- [ ] 选择表情插入到输入框
- [ ] 消息中的 https:// 链接可点击
- [ ] 右键消息 → "回复" → 输入框上方显示回复预览
- [ ] 发送后对方看到"回复 xxx 的消息"

---

## 6. 端到端加密完善

### 概述
默认启用端到端加密，群聊加密，加密状态 UI 指示。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `common/src/crypto/CryptoUtils.h` | 修改 | 添加群密钥方法 |
| `common/src/crypto/CryptoUtils.cpp` | 修改 | 实现群密钥加密/解密 |
| `server/src/core/MessageRouter.cpp` | 修改 | 群聊密钥分发 |
| `client/src/qmlbridge/ChatController.h` | 修改 | 添加加密状态 |
| `client/src/qmlbridge/ChatController.cpp` | 修改 | 实现加密消息处理 |
| `client/qml/components/MessageBubble.qml` | 修改 | 加密状态图标 |
| `client/qml/pages/ChatPage.qml` | 修改 | 加密状态指示器 |

### 详细 TODO

- [ ] 6.1. 群聊密钥管理

  **What to do**: 群主创建群时生成 AES-256 密钥，用每个成员的公钥加密后分发：
  ```cpp
  // 服务器在群创建时
  QByteArray groupKey = CryptoUtils::generateAesKey();
  
  // 为每个成员加密群密钥
  for (auto& member : members) {
      QByteArray memberPublicKey = getPublicKey(member.userId);
      QByteArray encryptedKey = CryptoUtils::rsaEncrypt(groupKey, memberPublicKey);
      // 存储: group_id, user_id, encrypted_key
      storeGroupKey(groupId, member.userId, encryptedKey);
  }
  ```

- [ ] 6.2. 端到端加密默认启用

  **What to do**: 在 `ChatController.cpp` 中，所有私聊消息默认加密：
  ```cpp
  void ChatController::sendPrivateMessage(const QString& content) {
      // 检查是否已有会话密钥
      if (hasSessionKey(currentChatFriendId)) {
          QByteArray encrypted = CryptoUtils::aes256GcmEncrypt(
              content.toUtf8(),
              getSessionKey(currentChatFriendId)
          );
          sendEncryptedMessage(encrypted);
      } else {
          // 触发密钥交换
          requestKeyExchange(currentChatFriendId);
          // 使用临时密钥发送
      }
  }
  ```

- [ ] 6.3. 加密状态 UI 指示器

  **What to do**: 在聊天头部显示加密状态：
  ```qml
  // ChatPage.qml 聊天头部
  Row {
      Text { text: "🔒" }  // 端到端加密
      Text { text: "消息已加密" }
      color: "#52c41a"
  }
  ```

  消息气泡加密图标：
  ```qml
  // MessageBubble.qml
  Text {
      text: isEncrypted ? "🔒" : ""
      font.pixelSize: 10
      color: appCore.themeManager.success
  }
  ```

### 验收标准
- [ ] 私聊默认启用端到端加密
- [ ] 群聊使用群共享密钥加密
- [ ] 新成员加入群聊时自动获取群密钥
- [ ] UI 显示加密状态（🔒 / 🔓）
- [ ] 密钥交换失败时提示用户

---

## 总体依赖矩阵

```
Task                   Depends On         Blocks
─────────────────────────────────────────────────
1.1 协议枚举扩展       -                  1.4, 1.5, 2.3, 3.2
1.2 数据库表           1.1                1.4, 1.5
1.3 文件目录           1.1                1.4
1.4 文件上传处理器     1.1, 1.2, 1.3     1.6, 1.7
1.5 文件下载处理器     1.1, 1.2          1.7
1.6 文件发送 UI        1.4                -
1.7 气泡文件渲染       1.5                -
1.8 传输进度组件       1.6                -

2.1 FTS 索引           1.1                2.2
2.2 搜索方法           2.1                2.3
2.3 搜索处理器         2.2                2.4
2.4 搜索 UI            2.3                2.5
2.5 搜索高亮           2.4                -

3.1 数据库表           1.1                3.2, 3.4
3.2 公告处理器         3.1                3.5
3.3 @提及              3.1                3.5
3.4 已读状态           3.1                3.5
3.5 群聊 UI            3.2, 3.3, 3.4     -

4.1 重连逻辑           -                  4.3, 4.5
4.2 心跳超时           4.1                4.5
4.3 消息缓存           4.1                4.5
4.4 会话超时           4.1                4.5
4.5 重连 UI            4.1, 4.2, 4.3     -

5.1 表情选择器         -                  -
5.2 URL 识别           -                  5.3
5.3 消息回复           1.1                5.3

6.1 群密钥管理         1.1                6.2
6.2 默认加密           6.1                6.3
6.3 加密 UI            6.2                -
```

## 执行波次

```
Wave 1 (Start Immediately - 协议 + 数据库):
├── 1.1 协议枚举扩展
├── 2.1 FTS 索引 (数据库)
├── 3.1 群聊数据库表
├── 1.2 文件元数据表
└── 1.3 文件存储目录

Wave 2 (Wave 1 完成后 - 服务端逻辑):
├── 1.4 文件上传处理器
├── 1.5 文件下载处理器
├── 2.2 搜索方法 + 2.3 搜索处理器
├── 3.2 公告处理器 + 3.4 已读状态
├── 4.1 重连逻辑
├── 4.2 心跳超时
├── 4.4 会话超时
└── 6.1 群密钥管理

Wave 3 (Wave 2 完成后 - 客户端核心):
├── 1.6 文件发送 UI
├── 1.7 气泡文件渲染
├── 1.8 传输进度组件
├── 2.4 搜索 UI + 2.5 搜索高亮
├── 4.3 消息缓存
├── 4.5 重连 UI
├── 6.2 默认加密
└── 6.3 加密 UI

Wave 4 (客户端 UI 完善):
├── 3.3 @提及
├── 3.5 群聊 UI 增强
├── 5.1 表情选择器
├── 5.2 URL 识别
└── 5.3 消息回复

Wave FINAL (集成测试):
├── T16: 端到端测试 (图片/文件)
├── T17: 端到端测试 (搜索/群聊/重连)
└── T18: 性能测试 + Bug 修复
```
