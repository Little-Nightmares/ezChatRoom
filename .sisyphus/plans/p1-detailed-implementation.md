# P1 全部功能详细实施计划

## 目录
1. [图片/文件发送 UI](#1-图片文件发送-ui)
2. [消息搜索 UI](#2-消息搜索-ui)
3. [在线状态强化](#3-在线状态强化)
4. [消息回复 UI](#4-消息回复-ui)
5. [群公告显示](#5-群公告显示)
6. [已读状态显示](#6-已读状态显示)
7. [URL 自动识别](#7-url-自动识别)

---

## 执行策略

```
Wave 1 (并行 — 服务端已就绪，纯客户端 UI):
├── T1: 图片/文件发送 UI      (InputBar + MessageBubble 增强)
├── T2: 消息搜索 UI           (搜索结果面板)
└── T3: 在线状态强化          (ContactItem + ChatHeader)

Wave 2 (并行 — 交互增强):
├── T4: 消息回复 UI           (右键菜单 + 回复预览输入)
├── T5: 群公告显示            (GroupInfoDialog 公告区)
├── T6: 已读状态显示          (GroupMessageBubble 底部)
└── T7: URL 安全识别          (检测点击位置打开链接)
```

---

## 1. 图片/文件发送 UI

### 概述
服务端文件上传/下载/分块处理器已完成。需要补完客户端发送 UI：选择文件后实际发送到服务端，接收方在气泡中显示图片/文件并可以下载。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `client/qml/components/InputBar.qml` | 修改 | FileDialog 的 onAccepted 发送文件 |
| `client/src/qmlbridge/ChatController.h` | 修改 | 添加 sendFile, downloadFile 声明 |
| `client/src/qmlbridge/ChatController.cpp` | **实现** | 文件发送/下载逻辑 |
| `client/src/network/MessageHandler.h` | 修改 | 注册文件消息处理器 |
| `client/src/network/MessageHandler.cpp` | 修改 | 处理文件上传响应/通知 |
| `client/qml/components/MessageBubble.qml` | 修改 | type===1/2 已实现，完善下载逻辑 |
| `server/src/core/MessageRouter.cpp` | - | 已完成 |

### 详细 TODO

- [ ] 1.1. ChatController 添加文件发送方法

  **What to do**: 在 `ChatController.h` 声明，`ChatController.cpp` 实现：
  ```cpp
  Q_INVOKABLE void sendFile(const QString& filePath);
  Q_INVOKABLE void downloadFile(uint64_t fileId);
  void handleFileUploadResponse(const QByteArray& body);
  void handleFileDownloadResponse(const QByteArray& body);
  void handleFileNotify(const QByteArray& body);
  ```

  `sendFile` 实现：
  ```cpp
  void ChatController::sendFile(const QString& filePath) {
      QFile file(filePath);
      if (!file.open(QIODevice::ReadOnly)) {
          emit showToast("无法打开文件: " + filePath, "error");
          return;
      }
      QByteArray data = file.readAll();
      file.close();
      
      QJsonObject obj;
      obj["fileName"] = QFileInfo(filePath).fileName();
      obj["fileSize"] = data.size();
      obj["mimeType"] = "application/octet-stream";
      obj["fileData"] = QString::fromUtf8(data.toBase64());
      obj["targetUserId"] = static_cast<qint64>(m_currentChatFriendId);
      obj["targetGroupId"] = static_cast<qint64>(m_currentGroupId);
      
      uint8_t type = static_cast<uint8_t>(MessageType::FileUploadRequest);
      m_appManager->tcpClient()->sendPacket(type, 0, ChatProtocol::nextSequence(),
          QJsonDocument(obj).toJson(QJsonDocument::Compact));
  }
  ```

- [ ] 1.2. MessageHandler 注册文件消息处理器

  **What to do**: 在 `AppManager::initialize()` 中注册：
  ```cpp
  // File upload response
  handler->registerHandler(
      static_cast<uint8_t>(MessageType::FileUploadResponse),
      [this](uint8_t, uint32_t, const QByteArray& body) {
          // Parse fileId, notify ChatController
          m_chatController->handleFileUploadResponse(body);
      });
  
  // File download response
  handler->registerHandler(
      static_cast<uint8_t>(MessageType::FileDownloadResponse),
      [this](uint8_t, uint32_t, const QByteArray& body) {
          m_chatController->handleFileDownloadResponse(body);
      });
  ```

- [ ] 1.3. InputBar 文件按钮实际发送

  **What to do**: 修改 `InputBar.qml` 的 FileDialog：
  ```qml
  FileDialog {
      id: filePickerDialog
      title: "选择文件"
      onAccepted: {
          var fileUrl = selectedFile.toString()
          var filePath = fileUrl.replace("file:///", "")
          if (Qt.platform.os === "windows") {
              // Windows path starts with /C:/...
              filePath = fileUrl.replace("file:///", "")
          }
          appCore.chatController.sendFile(filePath)
      }
  }
  ```

- [ ] 1.4. MessageBubble 文件下载按钮连接

  **What to do**: 当前按钮已有 `onClicked: appCore.chatController.downloadFile(bubble.messageId)`，需确保 `downloadFile` 实现正确发送 `FileDownloadRequest`。

### 验收标准
- [ ] 点击 📎 选择文件 → 文件上传到服务端
- [ ] 对方消息列表中显示 📄 文件消息
- [ ] 点击 ⬇ 按钮 → 文件下载到本地
- [ ] 图片文件直接显示缩略图（type===1）
- [ ] 大文件（>5MB）分块传输

---

## 2. 消息搜索 UI

### 概述
搜索栏已连接 `appCore.chatController.searchMessages()`，需要搜索结果展示面板。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `client/src/qmlbridge/ChatController.h` | 修改 | 添加搜索结果模型 |
| `client/src/qmlbridge/ChatController.cpp` | 修改 | 处理搜索结果 |
| `client/qml/pages/ChatPage.qml` | 修改 | 搜索结果面板渲染 |
| `client/qml/components/SearchResultItem.qml` | **新建** | 搜索结果项组件 |

### 详细 TODO

- [ ] 2.1. ChatController 搜索响应处理

  **What to do**: 添加搜索信号和模型：
  ```cpp
  Q_PROPERTY(QQmlListProperty<SearchResult> searchResults READ searchResults NOTIFY searchResultsChanged)
  
  signals:
      void searchResultsChanged();
  
  // 结构
  struct SearchResult {
      QString content;
      QString snippet;
      QString senderName;
      QString groupName;
      qint64 timestamp;
      bool isGroup;
  };
  ```

- [ ] 2.2. 搜索结果面板 UI

  **What to do**: 在 `ChatPage.qml` 搜索栏下方添加搜索结果面板：
  ```qml
  // 搜索结果覆盖层
  Rectangle {
      visible: searchResults.count > 0
      anchors.fill: parent
      color: appCore.themeManager.surface
      
      ListView {
          model: appCore.chatController.searchResults
          delegate: SearchResultItem {
              content: model.snippet
              senderName: model.senderName
              timestamp: model.timestamp
              isGroup: model.isGroup
              onClicked: {
                  // 跳转到该消息位置
                  messageListView.positionViewAtIndex(model.messageIndex, ListView.Contain)
                  searchResults.clear()
              }
          }
      }
  }
  ```

- [ ] 2.3. 创建 SearchResultItem.qml

  ```qml
  Rectangle {
      property string content
      property string senderName
      property var timestamp
      property bool isGroup
      signal clicked()
      
      height: 52
      color: mouseArea.containsMouse ? appCore.themeManager.surfaceAlt : "transparent"
      
      Row {
          anchors.fill: parent; anchors.margins: 8; spacing: 8
          Text { text: isGroup ? "👥" : "👤"; font.pixelSize: 16 }
          Column {
              Text { text: senderName; font.bold: true; font.pixelSize: 12 }
              Text { text: content; font.pixelSize: 11; color: "#666"; elide: Text.ElideRight; width: parent.width }
          }
      }
      MouseArea { id: mouseArea; anchors.fill: parent; hoverEnabled: true; onClicked: clicked() }
  }
  ```

### 验收标准
- [ ] 输入关键词按回车 → 显示搜索结果列表
- [ ] 结果按时间排序
- [ ] 每条结果显示发送者、消息片段
- [ ] 群聊消息标注群名称
- [ ] 点击结果跳转到对应消息位置

---

## 3. 在线状态强化

### 概述
已有 `FriendOnlineNotify/OfflineNotify` 协议，Online 指示器已有，但需要强化：联系人列表绿点更明显、聊天头部显示在线状态。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `client/qml/components/ContactItem.qml` | 修改 | 使绿点更明显 |
| `client/qml/pages/ChatPage.qml` | 修改 | 聊天头部显示在线状态 |
| `client/qml/components/Avatar.qml` | 修改 | 改进在线指示器样式 |
| `client/src/qmlbridge/ChatController.h` | 修改 | 添加在线状态属性 |
| `client/src/qmlbridge/ChatController.cpp` | 修改 | 在线状态管理 |

### 详细 TODO

- [ ] 3.1. 聊天头部在线状态

  **What to do**: 在 `ChatPage.qml` 头部昵称旁边添加状态文字：
  ```qml
  Column {
      Text { text: nickName; font.bold: true }
      Text {
          text: appCore.chatController.isOnline ? "🟢 在线" : "⚪ 离线"
          font.pixelSize: 10
          color: appCore.chatController.isOnline ? "#52c41a" : "#999"
      }
  }
  ```

- [ ] 3.2. 在线状态管理

  **What to do**: ChatController 跟踪在线状态：
  ```cpp
  Q_PROPERTY(bool isOnline READ isOnline NOTIFY onlineStateChanged)
  
  void ChatController::onFriendOnline(quint64 friendId) {
      if (friendId == m_currentChatFriendId) {
          m_isOnline = true;
          emit onlineStateChanged();
      }
  }
  ```

### 验收标准
- [ ] 好友上线 → 联系人列表绿点亮起
- [ ] 聊天头部显示"🟢 在线"或"⚪ 离线"
- [ ] 好友下线 → 实时更新状态

---

## 4. 消息回复 UI

### 概述
服务端 `handleMessageReply` 已完成。需要客户端右键菜单触发回复，输入框上方显示回复预览。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `client/qml/components/MessageBubble.qml` | 修改 | 右键菜单添加"回复"选项 |
| `client/qml/components/GroupMessageBubble.qml` | 修改 | 右键菜单添加"回复"选项 |
| `client/qml/components/InputBar.qml` | 修改 | 添加回复预览区域 |
| `client/src/qmlbridge/ChatController.h` | 修改 | 回复状态管理 |
| `client/src/qmlbridge/ChatController.cpp` | 修改 | 发送回复消息 |

### 详细 TODO

- [ ] 4.1. 聊天控制器回复状态

  ```cpp
  Q_PROPERTY(QString replyTarget READ replyTarget NOTIFY replyChanged)
  Q_PROPERTY(QString replyContent READ replyContent NOTIFY replyChanged)
  Q_PROPERTY(quint64 replyMessageId READ replyMessageId NOTIFY replyChanged)
  
  void setReply(quint64 msgId, const QString& content, const QString& sender);
  void clearReply();
  ```

- [ ] 4.2. 气泡右键菜单添加"回复"

  ```qml
  // 在现有的撤回菜单旁边
  MenuItem {
      text: "回复"
      onClicked: {
          appCore.chatController.setReply(bubble.messageId, bubble.content, bubble.senderName)
      }
  }
  ```

- [ ] 4.3. 输入框上方回复预览

  ```qml
  // InputBar.qml 输入框上方
  Rectangle {
      visible: appCore.chatController.replyTarget.length > 0
      height: 32
      color: appCore.themeManager.surfaceAlt
      
      Row {
          Text { text: "回复 " + appCore.chatController.replyTarget + ": " + appCore.chatController.replyContent }
          Button { text: "×"; onClicked: appCore.chatController.clearReply() }
      }
  }
  ```

### 验收标准
- [ ] 右键消息 → "回复" → 输入框上方显示"回复 xxx: 原消息内容"
- [ ] 输入回复内容发送 → 对方看到引用消息
- [ ] 点击 × 取消回复

---

## 5. 群公告显示

### 概述
服务端 `handleGroupAnnouncement` 已完成。需要在群信息对话框中显示公告。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `client/qml/dialogs/GroupInfoDialog.qml` | 修改 | 添加公告显示区域 |
| `client/src/qmlbridge/GroupController.h` | 修改 | 添加公告拉取方法 |
| `client/src/qmlbridge/GroupController.cpp` | 修改 | 实现公告拉取 |

### 详细 TODO

- [ ] 5.1. GroupController 公告方法

  ```cpp
  Q_PROPERTY(QString announcement READ announcement NOTIFY announcementChanged)
  Q_INVOKABLE void requestAnnouncement(uint64_t groupId);
  ```

- [ ] 5.2. GroupInfoDialog 公告区域

  ```qml
  // 在群名下方
  Rectangle {
      visible: appCore.groupController.announcement.length > 0
      color: "#fff7e6"
      border.color: "#ffd591"
      radius: 6
      
      Text {
          text: "📢 " + appCore.groupController.announcement
          font.pixelSize: 12
          wrapMode: Text.Wrap
      }
  }
  ```

### 验收标准
- [ ] 群主发布公告后自动推送到所有成员
- [ ] 打开群信息面板显示最新公告
- [ ] 公告内容换行显示

---

## 6. 已读状态显示

### 概述
服务端 `markGroupMessageRead` 已完成。需要在群聊气泡底部显示已读人数。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `client/qml/components/GroupMessageBubble.qml` | 修改 | 已读状态文字 |
| `client/src/qmlbridge/ChatController.cpp` | 修改 | 发送已读通知 |

### 详细 TODO

- [ ] 6.1. 群聊消息发送已读通知

  ```cpp
  // 当用户查看群聊消息时，发送已读通知
  void ChatController::markGroupMessagesRead() {
      if (m_currentGroupId == 0) return;
      
      uint64_t lastMsgId = m_messageModel->lastMessageId();
      if (lastMsgId == 0) return;
      
      QJsonObject obj;
      obj["msgId"] = static_cast<qint64>(lastMsgId);
      obj["groupId"] = static_cast<qint64>(m_currentGroupId);
      
      uint8_t type = static_cast<uint8_t>(MessageType::GroupMessageReadNotify);
      m_appManager->tcpClient()->sendPacket(type, 0, ChatProtocol::nextSequence(),
          QJsonDocument(obj).toJson(QJsonDocument::Compact));
  }
  ```

- [ ] 6.2. 群聊气泡底部已读状态

  ```qml
  // GroupMessageBubble.qml Status+Timestamp 行
  Text {
      visible: bubble.isMine && bubble.type === 0
      text: "已读 " + readCount + "/" + totalMembers
      font.pixelSize: 9
      color: readCount === totalMembers ? appCore.themeManager.success : appCore.themeManager.textTertiary
  }
  ```

### 验收标准
- [ ] 用户查看群聊消息自动发送已读通知
- [ ] 消息发送者看到"已读 3/5"等状态

---

## 7. URL 安全识别

### 概述
之前用 `Text.RichText` 导致消息空白，需要安全的 URL 识别方案。

### 方案
不使用 RichText，而是给 Text 添加 `MouseArea`，检测点击位置是否在 URL 上：

```qml
StyledText {
    id: bubbleText
    text: bubble.content
    // PlainText 安全显示，不解析 HTML
    
    // 用一个不可见的 Text 测量 URL 位置
    Text {
        id: urlDetector
        text: bubble.content
        visible: false
    }
}

MouseArea {
    anchors.fill: bubbleText
    cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
    onClicked: {
        // 通过正则检测点击位置是否有 URL
        var urls = bubble.content.match(/(https?:\/\/[^\s]+)/g)
        if (urls && urls.length > 0) {
            Qt.openUrlExternally(urls[0])
        }
    }
    // hoverEnabled: true 时根据是否有 URL 改变光标
}
```

### 涉及文件
- `client/qml/components/MessageBubble.qml`
- `client/qml/components/GroupMessageBubble.qml`

### 验收标准
- [ ] 消息中的 `https://example.com` 可点击打开
- [ ] 普通文本不受影响，不显示 HTML 转义问题
- [ ] 鼠标悬停在 URL 上时变为手型光标

---

## 总体执行波次

```
Wave 1 (并行):
├── T1: 文件发送 UI (ChatController + InputBar)    ~3h
├── T2: 消息搜索 UI (SearchResultItem + 面板)        ~3h
└── T3: 在线状态强化 (ContactItem + ChatHeader)      ~1h

Wave 2 (并行):
├── T4: 消息回复 UI (右键菜单 + 预览)               ~2h
├── T5: 群公告显示 (GroupInfoDialog)                ~1h
├── T6: 已读状态 (GroupMessageBubble)              ~1h
└── T7: URL 识别 (MouseArea)                       ~1h

总计: ~12h
```
