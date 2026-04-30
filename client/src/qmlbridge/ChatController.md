# ChatController 实现指南

## 所属模块
client/src/qmlbridge/

## 职责
聊天控制器，处理消息发送、接收、历史记录加载、消息撤回。

## 类说明
- 类名: `ChatController`
- 继承: `QObject`
- 命名空间: `client`

## 实现指南

### 1. ChatController.h 设计

```cpp
#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QObject>
#include <QtQml/qqmlregistration.h>
#include "network/TcpClient.h"
#include "models/MessageModel.h"

namespace client {

class ChatController : public QObject
{
    Q_OBJECT
    // QML注册宏，使前端可以访问此类
    QML_ELEMENT

public:
    explicit ChatController(QObject *parent = nullptr);
    ~ChatController() override;

    // 发送消息方法，供QML调用
    Q_INVOKABLE void sendMessage(const QString &receiverId, const QString &message);
    
    // 加载历史消息方法，供QML调用
    Q_INVOKABLE void loadMessages(const QString &conversationId, int limit, int offset);

signals:
    // 消息发送成功信号
    void messageSent(const QString &messageId, const QString &receiverId);
    
    // 消息发送失败信号
    void messageFailed(const QString &errorMessage);
    
    // 收到新消息信号
    void messageReceived(const QVariantMap &message);
    
    // 历史消息加载完成信号
    void messagesLoaded(const QVariantList &messages);

private slots:
    // 处理TcpClient的消息接收信号
    void onMessageReceived(const QByteArray &data);
    
    // 处理TcpClient的连接状态变化信号
    void onConnectionStatusChanged(bool connected);

private:
    // TcpClient指针，用于网络通信
    TcpClient *m_tcpClient;
    
    // 消息模型指针，用于管理消息数据
    MessageModel *m_messageModel;
    
    // 生成唯一消息ID
    QString generateMessageId();
};

} // namespace client

#endif // CHATCONTROLLER_H
```

### 2. ChatController.cpp 实现

```cpp
#include "ChatController.h"
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

namespace client {

ChatController::ChatController(QObject *parent) : QObject(parent)
{
    // 初始化TcpClient
    m_tcpClient = new TcpClient(this);
    
    // 初始化MessageModel
    m_messageModel = new MessageModel(this);
    
    // 建立信号槽连接
    connect(m_tcpClient, &TcpClient::messageReceived, this, &ChatController::onMessageReceived);
    connect(m_tcpClient, &TcpClient::connectionStatusChanged, this, &ChatController::onConnectionStatusChanged);
    
    // 连接到服务器
    m_tcpClient->connectToServer("127.0.0.1", 8080);
}

ChatController::~ChatController()
{
    // 清理资源
    if (m_tcpClient) {
        delete m_tcpClient;
    }
    if (m_messageModel) {
        delete m_messageModel;
    }
}

void ChatController::sendMessage(const QString &receiverId, const QString &message)
{
    // 生成消息ID
    QString messageId = generateMessageId();
    
    // 构建消息对象
    QJsonObject messageObj;
    messageObj["id"] = messageId;
    messageObj["type"] = "chat";
    messageObj["action"] = "send";
    messageObj["senderId"] = "currentUser";
    messageObj["receiverId"] = receiverId;
    messageObj["content"] = message;
    messageObj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // 转换为JSON字符串
    QJsonDocument doc(messageObj);
    QByteArray data = doc.toJson();
    
    // 通过TcpClient发送消息
    bool success = m_tcpClient->sendData(data);
    
    if (success) {
        // 发送成功，发出信号
        emit messageSent(messageId, receiverId);
    } else {
        // 发送失败，发出信号
        emit messageFailed("Failed to send message");
    }
}

void ChatController::loadMessages(const QString &conversationId, int limit, int offset)
{
    // 构建加载消息请求
    QJsonObject requestObj;
    requestObj["type"] = "chat";
    requestObj["action"] = "load";
    requestObj["conversationId"] = conversationId;
    requestObj["limit"] = limit;
    requestObj["offset"] = offset;
    
    // 转换为JSON字符串
    QJsonDocument doc(requestObj);
    QByteArray data = doc.toJson();
    
    // 通过TcpClient发送请求
    m_tcpClient->sendData(data);
    
    // 注意：响应将通过onMessageReceived槽处理
}

void ChatController::onMessageReceived(const QByteArray &data)
{
    // 解析收到的数据
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }
    
    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    QString action = obj["action"].toString();
    
    if (type == "chat") {
        if (action == "receive") {
            // 收到新消息
            QVariantMap message;
            message["id"] = obj["id"].toString();
            message["sender"] = obj["senderId"].toString();
            message["message"] = obj["content"].toString();
            message["timestamp"] = obj["timestamp"].toString();
            message["isOwn"] = (obj["senderId"].toString() == "currentUser");
            
            // 发出消息接收信号
            emit messageReceived(message);
        } else if (action == "load") {
            // 加载历史消息响应
            QVariantList messages;
            QJsonArray messageArray = obj["messages"].toArray();
            
            for (const QJsonValue &value : messageArray) {
                QJsonObject msgObj = value.toObject();
                QVariantMap message;
                message["id"] = msgObj["id"].toString();
                message["sender"] = msgObj["senderId"].toString();
                message["message"] = msgObj["content"].toString();
                message["timestamp"] = msgObj["timestamp"].toString();
                message["isOwn"] = (msgObj["senderId"].toString() == "currentUser");
                messages.append(message);
            }
            
            // 发出消息加载完成信号
            emit messagesLoaded(messages);
        }
    }
}

void ChatController::onConnectionStatusChanged(bool connected)
{
    // 处理连接状态变化
    if (connected) {
        qDebug() << "ChatController: Connected to server";
    } else {
        qDebug() << "ChatController: Disconnected from server";
    }
}

QString ChatController::generateMessageId()
{
    // 生成唯一消息ID
    return "msg_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + "_" + QString::number(qrand() % 1000);
}

} // namespace client
```

### 3. TcpClient 接口设计（需要实现）

为了使ChatController正常工作，TcpClient需要实现以下接口：

```cpp
// TcpClient.h 中需要添加的方法和信号
public:
    // 连接到服务器
    void connectToServer(const QString &host, int port);
    
    // 发送数据
    bool sendData(const QByteArray &data);

signals:
    // 收到消息信号
    void messageReceived(const QByteArray &data);
    
    // 连接状态变化信号
    void connectionStatusChanged(bool connected);
```

### 4. 与前端的集成

在AppCore中注册ChatController：

```cpp
// AppCore.cpp
#include "qmlbridge/ChatController.h"

AppCore::AppCore(QObject *parent) : QObject(parent)
{
    // 初始化ChatController
    m_chatController = new ChatController(this);
    
    // 向QML注册ChatController
    engine->rootContext()->setContextProperty("chatController", m_chatController);
}
```

### 5. 前端使用示例

在ChatPage.qml中使用ChatController：

```qml
// ChatPage.qml 中的使用示例
InputBar {
    Layout.fillWidth: true
    
    onSendMessage: {
        // 发送消息
        chatController.sendMessage("receiverId", text);
        
        // 在本地消息模型中立即添加消息（乐观更新）
        messageModel.addMessage({
            "id": "temp_" + Date.now(),
            "message": text,
            "sender": "me",
            "isOwn": true,
            "timestamp": new Date().toLocaleTimeString()
        })
    }
}

// 监听消息接收信号
Connections {
    target: chatController
    onMessageReceived: {
        // 收到新消息，添加到消息模型
        messageModel.addMessage(message);
    }
    
    onMessageSent: {
        // 消息发送成功，更新临时消息状态
        console.log("Message sent successfully:", messageId);
    }
    
    onMessageFailed: {
        // 消息发送失败，显示错误
        console.log("Message failed:", errorMessage);
    }
}

// 页面加载时加载历史消息
Component.onCompleted: {
    // 加载历史消息
    chatController.loadMessages("conversationId", 75, 0);
    
    // 监听消息加载完成信号
    chatController.messagesLoaded.connect(function(messages) {
        // 清空现有消息
        messageModel.clear();
        // 添加加载的消息
        for (var i = 0; i < messages.length; i++) {
            messageModel.addMessage(messages[i]);
        }
    });
}
```

## 实现要点

1. **消息格式**：使用JSON格式进行消息传输，确保前后端数据结构一致。

2. **错误处理**：实现完善的错误处理机制，包括网络连接失败、消息发送失败等情况。

3. **乐观更新**：在发送消息时，先在本地添加消息，提升用户体验，待服务器响应后再确认状态。

4. **消息ID生成**：确保每条消息都有唯一的ID，用于跟踪消息状态和处理消息撤回。

5. **信号槽连接**：正确建立ChatController与TcpClient之间的信号槽连接，确保消息能够及时传递。

6. **QML交互**：使用Q_INVOKABLE宏标记供QML调用的方法，使用信号通知前端事件。

## 依赖关系

- **Qt Core**：提供基础的QObject、信号槽机制
- **Qt Network**：提供网络通信功能
- **Qt QML**：提供QML集成功能
- **TcpClient**：处理网络通信
- **MessageModel**：管理消息数据

## 测试建议

1. **单元测试**：测试ChatController的核心方法，如sendMessage、loadMessages等。

2. **集成测试**：测试ChatController与TcpClient、前端QML的集成。

3. **模拟测试**：使用模拟的TcpClient实现，测试ChatController在不同网络状态下的行为。

4. **性能测试**：测试ChatController在处理大量消息时的性能。

通过以上实现，ChatController将能够满足聊天应用的核心功能需求，包括消息发送、接收、历史记录加载等。