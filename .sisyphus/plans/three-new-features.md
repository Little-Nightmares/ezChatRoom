# 三大新功能详细实施计划

## 目录
1. [聊天机器人](#1-聊天机器人)
2. [聊天背景](#2-聊天背景)
3. [数据看板 (Web Admin)](#3-数据看板-web-admin)

---

## 执行策略

```
Wave 1 (并行):
├── T1: 聊天机器人 — 服务端 LLM API 调用
├── T2: 聊天背景 — 客户端 QML 背景图设置
└── T3: 数据看板 — Web Admin API 扩展

Wave 2 (并行):
├── T4: 聊天机器人 — 客户端 @bot 输入 + 消息展示
├── T5: 聊天背景 — 设置页背景选择 UI + 持久化
└── T6: 数据看板 — Admin HTML 前端页面更新

Wave 3 (集成测试):
├── T7: @bot 端到端测试
├── T8: 背景切换测试
└── T9: 管理页面功能验证
```

---

## 1. 聊天机器人

### 概述
在群聊中通过 `@bot 问题` 调用 LLM API（DeepSeek/OpenAI），机器人以系统账号回复。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `server/CMakeLists.txt` | 修改 | 添加 HTTP 库依赖（Qt Network 已有） |
| `server/src/core/BotService.h` | **新建** | 机器人服务类声明 |
| `server/src/core/BotService.cpp` | **新建** | LLM API 调用 + 回复注入 |
| `server/src/core/MessageRouter.h` | 修改 | 添加 bot 初始化 |
| `server/src/core/MessageRouter.cpp` | 修改 | 检测 @bot 并转发到 BotService |
| `server/main.cpp` | 修改 | 创建 BotService 实例 |
| `client/qml/components/InputBar.qml` | 修改 | @ 弹出提示 |
| `client/qml/components/GroupMessageBubble.qml` | 修改 | 机器人消息特殊标记 |

### 详细 TODO

- [ ] 1.1. 创建 BotService（服务端）

  **What to do**: 新建 `server/src/core/BotService.h` 和 `server/src/core/BotService.cpp`：

  ```cpp
  class BotService : public QObject {
      Q_OBJECT
  public:
      BotService(ServerDatabase* db, TcpServer* server, QObject* parent = nullptr);
      
      // 处理用户 @bot 消息
      void handleBotMention(uint64_t groupId, uint64_t senderId, 
                            const QString& senderName, const QString& question);
      
      // 配置
      void setApiKey(const QString& key);
      void setApiUrl(const QString& url);
      void setBotUserId(uint64_t userId) { m_botUserId = userId; }
      uint64_t botUserId() const { return m_botUserId; }

  private:
      QString callLLM(const QString& prompt);  // 调用 LLM API
      
      ServerDatabase* m_db;
      TcpServer* m_server;
      QString m_apiKey;
      QString m_apiUrl = "https://api.deepseek.com/v1/chat/completions";
      QString m_model = "deepseek-chat";
      uint64_t m_botUserId = 0;
  };
  ```

- [ ] 1.2. LLM API 调用实现

  **What to do**: 在 `BotService.cpp` 中实现 `callLLM`：
  ```cpp
  QString BotService::callLLM(const QString& prompt) {
      QNetworkAccessManager mgr;
      QNetworkRequest req(QUrl(m_apiUrl));
      req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
      req.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());

      QJsonObject body;
      body["model"] = m_model;
      
      QJsonArray messages;
      QJsonObject sysMsg;
      sysMsg["role"] = "system";
      sysMsg["content"] = "你是群聊机器人，用中文简洁回答。当前时间: " + 
                          QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
      messages.append(sysMsg);
      
      QJsonObject userMsg;
      userMsg["role"] = "user";
      userMsg["content"] = prompt;
      messages.append(userMsg);
      
      body["messages"] = messages;
      body["stream"] = false;

      QNetworkReply* reply = mgr.post(req, QJsonDocument(body).toJson());
      QEventLoop loop;
      QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::Quit);
      loop.exec();

      if (reply->error() != QNetworkReply::NoError) {
          qWarning() << "BotService LLM error:" << reply->errorString();
          return "机器人暂时无法回答（" + reply->errorString() + "）";
      }

      QJsonDocument resp = QJsonDocument::fromJson(reply->readAll());
      return resp.object()["choices"].toArray()[0].toObject()["message"]
             .toObject()["content"].toString();
  }
  ```

- [ ] 1.3. 消息路由检测 @bot

  **What to do**: 在 `MessageRouter::handleGroupMessage()` 中添加检测：
  ```cpp
  // 在 handleGroupMessage 中，转发消息给所有成员后
  if (content.contains("@bot") && m_botService) {
      QString question = content;
      question.replace("@bot", "").trimmed();
      if (!question.isEmpty()) {
          m_botService->handleBotMention(groupId, senderId, senderNickname, question);
      }
  }
  ```

- [ ] 1.4. 机器人回复注入

  **What to do**: `BotService::handleBotMention` 获取 LLM 回复后，作为系统消息注入群聊：
  ```cpp
  void BotService::handleBotMention(uint64_t groupId, uint64_t senderId, 
                                     const QString& senderName, const QString& question) {
      QString answer = callLLM(question);
      
      // 存储为群消息
      qint64 now = QDateTime::currentSecsSinceEpoch();
      uint64_t msgId = m_db->storeGroupMessage(groupId, m_botUserId, answer, 0, now);
      
      // 推送给所有在线群成员
      QJsonObject push;
      push["groupId"] = static_cast<qint64>(groupId);
      push["senderId"] = static_cast<qint64>(m_botUserId);
      push["senderNickname"] = "Bot";
      push["content"] = answer;
      push["timestamp"] = now;
      push["type"] = 3;  // System type
      
      QByteArray data = QJsonDocument(push).toJson(QJsonDocument::Compact);
      auto members = m_db->getGroupMemberIds(groupId);
      for (auto uid : members) {
          auto* session = m_server->getSession(uid);
          if (session) {
              session->sendPacket(MessageType::GroupMessageNotify, 0, 
                                  ChatProtocol::nextSequence(), data);
          }
      }
  }
  ```

- [ ] 1.5. CMakeLists.txt 添加 BotService

  ```cmake
  src/core/BotService.h
  src/core/BotService.cpp
  ```

- [ ] 1.6. 客户端输入 @bot 提示

  **What to do**: 在 `InputBar.qml` 中检测 `@` 输入，弹出机器人提示：
  ```qml
  // 当输入 @ 时
  onTextChanged: {
      var text = editArea.text
      if (text.endsWith("@")) {
          // 显示 @bot 提示
          atBotHint.visible = true
      } else {
          atBotHint.visible = false
      }
  }
  
  // @bot 快速插入按钮
  StyledText {
      id: atBotHint
      visible: false
      text: "输入 @bot 问题 召唤AI助手"
      color: appCore.themeManager.textTertiary
      font.pixelSize: 11
  }
  ```

### 验收标准
- [ ] 服务端启动时创建系统机器人账号（userId=0 或专用 ID）
- [ ] 用户在群聊中输入 `@bot 今天天气怎么样` → 服务端调用 LLM API
- [ ] 机器人回复以系统消息形式推送到群聊
- [ ] 机器人消息在客户端有特殊标记（🤖 图标）
- [ ] API 调用失败时有友好提示
- [ ] 支持配置 API Key（通过环境变量或配置文件）

---

## 2. 聊天背景

### 概述
允许用户设置聊天区域的自定义背景图片/颜色。设置持久化到本地数据库。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `client/src/database/DatabaseManager.h` | 修改 | 添加背景设置存储 |
| `client/src/database/DatabaseManager.cpp` | 修改 | 实现背景设置 CRUD |
| `client/qml/pages/ChatPage.qml` | 修改 | 聊天区域背景图 |
| `client/qml/pages/SettingsPage.qml` | 修改 | 背景选择 UI |
| `client/src/qmlbridge/AppCore.h` | 修改 | 添加背景属性 |
| `client/src/qmlbridge/AppCore.cpp` | 修改 | 加载/保存背景设置 |

### 详细 TODO

- [ ] 2.1. 数据库存储背景设置

  **What to do**: 在 `DatabaseManager::createTables()` 添加：
  ```sql
  CREATE TABLE IF NOT EXISTS user_settings (
      key   TEXT PRIMARY KEY,
      value TEXT NOT NULL
  );
  ```

  添加方法：
  ```cpp
  void saveSetting(const QString& key, const QString& value);
  QString loadSetting(const QString& key, const QString& defaultValue = "");
  ```

- [ ] 2.2. 聊天区域背景图

  **What to do**: 在 `ChatPage.qml` 中，聊天消息区域的背景改为可配置：
  ```qml
  // 聊天区域背景 (在 messageListView 后面)
  Rectangle {
      anchors.fill: messageListView
      color: appCore.chatBackgroundColor  // 纯色背景
      
      // 可选图片背景
      Image {
          source: appCore.chatBackgroundImage
          fillMode: Image.Tile  // 平铺
          opacity: 0.3
          visible: appCore.chatBackgroundImage !== ""
      }
  }
  ```

- [ ] 2.3. 设置页背景选择 UI

  **What to do**: 在 `SettingsPage.qml` 中添加背景选择区域：
  ```qml
  GroupBox {
      title: "聊天背景"
      
      GridLayout {
          columns: 4
          
          // 预设纯色背景
          Repeater {
              model: ["#ffffff", "#f5f5f5", "#e8f4fd", "#f0faf0",
                      "#fff7e6", "#fce4ec", "#e8eaf6", "#e0f2f1"]
              Rectangle {
                  width: 40; height: 40; radius: 6
                  color: modelData
                  border.width: appCore.chatBackgroundColor === modelData ? 2 : 0
                  border.color: appCore.themeManager.primary
                  MouseArea {
                      anchors.fill: parent
                      onClicked: appCore.setChatBackground(modelData)
                  }
              }
          }
          
          // 自定义图片按钮
          Button {
              text: "📷"
              onClicked: bgFileDialog.open()
          }
          
          // 清除背景按钮
          Button {
              text: "✕ 重置"
              onClicked: appCore.clearChatBackground()
          }
      }
  }
  ```

- [ ] 2.4. 图片背景文件选择

  ```qml
  FileDialog {
      id: bgFileDialog
      title: "选择背景图片"
      nameFilters: ["Image files (*.png *.jpg *.jpeg)"]
      onAccepted: {
          var path = selectedFile.toString()
          // 复制到本地缓存目录并设置
          appCore.setChatBackgroundImage(path)
      }
  }
  ```

- [ ] 2.5. AppCore 公开背景属性

  ```cpp
  Q_PROPERTY(QString chatBackgroundColor READ chatBackgroundColor NOTIFY chatBackgroundChanged)
  Q_PROPERTY(QString chatBackgroundImage READ chatBackgroundImage NOTIFY chatBackgroundChanged)
  
  Q_INVOKABLE void setChatBackground(const QString& color);
  Q_INVOKABLE void setChatBackgroundImage(const QString& imagePath);
  Q_INVOKABLE void clearChatBackground();
  ```

### 验收标准
- [ ] 设置页显示 8 种预设背景色
- [ ] 点击颜色 → 聊天区域背景立即变化
- [ ] 可选择本地图片作为平铺背景
- [ ] 重启应用后背景设置保留
- [ ] 有清除/重置按钮恢复默认

---

## 3. 数据看板 (Web Admin)

### 概述
扩展现有的 Admin HTTP 服务器，添加数据可视化看板：消息统计、在线用户趋势、群组活跃度。

### 涉及文件

| 文件 | 改动 | 说明 |
|---|---|---|
| `server/src/admin/AdminHttpServer.h` | 修改 | 添加统计 API 端点 |
| `server/src/admin/AdminHttpServer.cpp` | 修改 | 实现统计 API |
| `server/src/database/ServerDatabase.h` | 修改 | 添加统计查询方法 |
| `server/src/database/ServerDatabase.cpp` | 修改 | 实现统计 SQL |
| `server/src/admin/index.html` | 修改 | 嵌入 Chart.js 可视化 |

### 详细 TODO

- [ ] 3.1. 服务端统计 API

  **What to do**: 在 `AdminHttpServer::handleApiRequest` 中添加端点：
  ```
  GET  /api/stats/daily-messages    → 近30天每日消息数
  GET  /api/stats/online-trend      → 近24小时在线用户趋势
  GET  /api/stats/top-groups        → 最活跃群聊 TOP10
  GET  /api/stats/user-growth       → 每日新增用户
  ```

  ```cpp
  // AdminHttpServer.cpp 添加
  QJsonObject getDailyMessages() {
      auto data = m_db->getDailyMessageStats(30);
      QJsonObject result;
      QJsonArray dates, counts;
      for (const auto& [date, count] : data) {
          dates.append(date);
          counts.append(count);
      }
      result["dates"] = dates;
      result["counts"] = counts;
      return result;
  }
  ```

- [ ] 3.2. 数据库统计查询

  **What to do**: 在 `ServerDatabase.cpp` 添加：
  ```cpp
  QVector<QPair<QString, int>> getDailyMessageStats(int days) {
      QVector<QPair<QString, int>> result;
      QSqlQuery query(m_db);
      query.prepare(R"(
          SELECT date(timestamp, 'unixepoch') as day, COUNT(*) 
          FROM (
              SELECT timestamp FROM messages
              UNION ALL
              SELECT timestamp FROM group_messages
          )
          WHERE timestamp >= :since
          GROUP BY day ORDER BY day
      )");
      query.bindValue(":since", QDateTime::currentSecsSinceEpoch() - days * 86400);
      // ...
      return result;
  }
  ```

- [ ] 3.3. Admin 前端 Chart.js 看板

  **What to do**: 在 `index.html` 中添加 Chart.js 可视化：
  ```html
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  
  <div class="dashboard">
      <div class="card">
          <h3>近30天消息趋势</h3>
          <canvas id="msgChart"></canvas>
      </div>
      <div class="card">
          <h3>最活跃群聊</h3>
          <canvas id="groupChart"></canvas>
      </div>
      <div class="card">
          <h3>在线用户趋势 (24h)</h3>
          <canvas id="onlineChart"></canvas>
      </div>
      <div class="card">
          <h3>用户增长</h3>
          <canvas id="userChart"></canvas>
      </div>
  </div>
  
  <script>
  async function loadStats() {
      const msgData = await apiGet('/api/stats/daily-messages');
      new Chart('msgChart', {
          type: 'bar',
          data: {
              labels: msgData.dates,
              datasets: [{
                  label: '消息数',
                  data: msgData.counts,
                  backgroundColor: '#1890ff'
              }]
          }
      });
  }
  loadStats();
  </script>
  ```

- [ ] 3.4. Admin 页面样式更新

  **What to do**: 添加看板卡片样式：
  ```css
  .dashboard { 
      display: grid; 
      grid-template-columns: 1fr 1fr; 
      gap: 20px; 
      padding: 20px; 
  }
  .card { 
      background: white; 
      border-radius: 12px; 
      padding: 16px; 
      box-shadow: 0 2px 8px rgba(0,0,0,0.08); 
  }
  .card h3 { 
      margin: 0 0 12px 0; 
      font-size: 14px; 
      color: #666; 
  }
  ```

### 验收标准
- [ ] 访问 `http://localhost:19527` 看到数据看板页面
- [ ] 近30天消息趋势图正常显示
- [ ] 最活跃群聊 TOP10 排行
- [ ] 在线用户趋势（24小时粒度）
- [ ] 图表数据实时或定期刷新
- [ ] 看板页面响应式布局

---

## 总体依赖与工时

| 功能 | 预估工时 | 并行度 |
|---|---|---|
| 🤖 聊天机器人 | ~6h | Wave 1 |
| 🖼️ 聊天背景 | ~3h | Wave 1 |
| 📊 数据看板 | ~4h | Wave 1 |
| 集成测试 | ~2h | Wave 3 |
| **总计** | **~15h** | |
