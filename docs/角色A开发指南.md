# 角色 A：客户端全能王 —— 专属开发指南

> 你负责：`client/qml/`（界面）+ `client/src/qmlbridge/`（Bridge 桥接）+ `client/src/models/`（数据模型）
>
> 本文档按**实际开发顺序**编排，每一步都有可直接复制粘贴的代码。

---

## 你需要学的 3 样东西（按优先级）

| 优先级 | 技术 | 学到什么程度 | 花多长时间 |
|--------|------|-------------|-----------|
| **P0** | **QML 基础语法** | 会写布局、按钮、输入框、列表 | 3-5 天 |
| **P0** | **信号与槽** | 理解 `signal` 发出 → `slot` 接收 这个机制 | 2-3 天 |
| **P1** | **QML 调用 C++** | 知道怎么在 QML 里调用 C++ 函数、接收 C++ 信号 | 3-5 天 |

> **推荐学习顺序**：先学 QML 语法（写出界面）→ 再学信号与槽（让界面能交互）→ 最后学 QML 调用 C++（让界面连上后端逻辑）

---

## 第一步：让你的登录界面显示出来（Day 1）

**目标**：运行程序后看到一个有用户名、密码输入框和登录按钮的界面。

### 1.1 修改 `client/qml/pages/LoginPage.qml`

把整个文件内容替换为：

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root
    title: qsTr("登录")

    // 背景色
    background: Rectangle {
        color: "#f5f5f5"
    }

    // 整体布局：垂直居中
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20
        width: parent.width * 0.8

        // 应用标题
        Text {
            text: qsTr("ChatRoom")
            font.pixelSize: 36
            font.bold: true
            color: "#333333"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: qsTr("局域网即时通讯")
            font.pixelSize: 14
            color: "#999999"
            Layout.alignment: Qt.AlignHCenter
        }

        // ---------- 用户名输入框 ----------
        TextField {
            id: usernameField
            placeholderText: qsTr("请输入用户名")
            font.pixelSize: 16
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            leftPadding: 16
            background: Rectangle {
                radius: 8
                color: "#ffffff"
                border.color: usernameField.activeFocus ? "#4A90D9" : "#dddddd"
                border.width: 1
            }
        }

        // ---------- 密码输入框 ----------
        TextField {
            id: passwordField
            placeholderText: qsTr("请输入密码")
            font.pixelSize: 16
            echoMode: TextInput.Password
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            leftPadding: 16
            background: Rectangle {
                radius: 8
                color: "#ffffff"
                border.color: passwordField.activeFocus ? "#4A90D9" : "#dddddd"
                border.width: 1
            }
        }

        // ---------- 登录按钮 ----------
        Button {
            text: qsTr("登 录")
            font.pixelSize: 18
            font.bold: true
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            background: Rectangle {
                radius: 8
                color: parent.down ? "#3a7bc8" : "#4A90D9"
            }
            contentItem: Text {
                text: parent.text
                color: "#ffffff"
                font: parent.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            // 点击登录按钮时触发
            onClicked: {
                console.log("用户点击了登录，用户名:", usernameField.text)
                // TODO: 这里后续会调用 C++ 的登录逻辑
            }
        }

        // ---------- 注册链接 ----------
        Text {
            text: qsTr("没有账号？立即注册")
            font.pixelSize: 14
            color: "#4A90D9"
            Layout.alignment: Qt.AlignHCenter

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    console.log("跳转到注册页面")
                    // TODO: 这里后续会切换到注册页面
                }
            }
        }
    }
}
```

### 1.2 修改 `client/qml/main.qml`

把整个文件内容替换为：

```qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 400
    height: 700
    visible: true
    title: qsTr("ChatRoom")

    // 直接显示登录页面
    LoginPage {
        anchors.fill: parent
    }
}
```

### 1.3 编译运行

在 Qt Creator 中按 **Ctrl+R**（或点绿色三角 ▶），你应该看到：

```
┌──────────────────────┐
│      ChatRoom        │
│   局域网即时通讯      │
│                      │
│  ┌────────────────┐  │
│  │ 请输入用户名    │  │
│  └────────────────┘  │
│  ┌────────────────┐  │
│  │ 请输入密码      │  │
│  └────────────────┘  │
│  ┌────────────────┐  │
│  │     登 录       │  │
│  └────────────────┘  │
│  没有账号？立即注册   │
└──────────────────────┘
```

**如果你看到了这个界面，恭喜你，第一步完成了！** 🎉

---

## 第二步：理解 QML 的核心概念（Day 1-2）

在继续写代码之前，你需要理解 QML 的几个核心概念。对照你刚写的 LoginPage.qml 来理解：

### 2.1 QML 是什么

QML 是 Qt 的界面描述语言，类似 HTML/CSS，但更强大。它的特点是：
- **声明式**：你描述"界面长什么样"，而不是"怎么画"
- **属性绑定**：一个值变了，依赖它的地方自动更新
- **信号与槽**：用户点击按钮 → 触发一个函数

### 2.2 对照代码理解核心概念

```qml
// ① 导入模块（类似 #include）
import QtQuick           // 基础组件：Text, Rectangle, MouseArea...
import QtQuick.Controls  // UI 控件：Button, TextField, Page...
import QtQuick.Layouts   // 布局：ColumnLayout, RowLayout...

// ② 根元素：每个 QML 文件有且只有一个根元素
Page {
    id: root    // ③ id：给这个元素起个名字，其他地方可以用 root 引用它

    // ④ 布局：ColumnLayout 让子元素垂直排列
    ColumnLayout {
        anchors.centerIn: parent  // ⑤ 锚点布局：居中于父元素
        spacing: 20               // 子元素之间的间距

        // ⑥ 文本
        Text {
            text: qsTr("ChatRoom")   // qsTr() 用于国际化翻译
            font.pixelSize: 36        // 字号
            color: "#333333"          // 颜色
        }

        // ⑦ 输入框
        TextField {
            id: usernameField         // 给输入框起名，后面可以用 usernameField.text 获取内容
            placeholderText: qsTr("请输入用户名")
        }

        // ⑧ 按钮
        Button {
            text: qsTr("登 录")

            // ⑨ 信号处理：用户点击按钮时执行
            onClicked: {
                // 获取输入框的文本
                console.log(usernameField.text)
            }
        }
    }
}
```

### 2.3 你需要记住的 QML 组件速查表

| 你想做什么 | 用什么组件 | 示例 |
|-----------|-----------|------|
| 显示文字 | `Text` | `Text { text: "你好" }` |
| 输入文字 | `TextField` | `TextField { id: input }` |
| 按钮 | `Button` | `Button { onClicked: { ... } }` |
| 密码输入 | `TextField { echoMode: TextInput.Password }` | |
| 图片 | `Image { source: "qrc:/images/avatar.png" }` | |
| 列表 | `ListView` | 后面聊天列表会用到 |
| 垂直排列 | `ColumnLayout` | |
| 水平排列 | `RowLayout` | |
| 滚动区域 | `ScrollView` | 包裹内容使其可滚动 |
| 弹窗 | `Dialog` | |
| 页面 | `Page` | |
| 切换页面 | `StackView` | 后面做页面导航会用到 |

---

## 第三步：实现页面导航 —— 从登录页跳到聊天页（Day 2-3）

**目标**：点击登录按钮后，从 LoginPage 跳转到 ChatPage。

### 3.1 修改 `client/qml/main.qml`（使用 StackView 管理页面）

```qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 400
    height: 700
    visible: true
    title: qsTr("ChatRoom")

    // StackView：页面导航管理器，类似手机的页面栈
    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: "qrc:/qml/pages/LoginPage.qml"  // 初始页面：登录页
    }
}
```

### 3.2 修改 `LoginPage.qml`（登录后跳转）

在 `onClicked` 里添加跳转逻辑：

```qml
onClicked: {
    console.log("用户点击了登录，用户名:", usernameField.text)
    // 跳转到聊天页面
    stackView.push("qrc:/qml/pages/ChatPage.qml")
}
```

同样，注册链接也加上跳转：

```qml
MouseArea {
    anchors.fill: parent
    cursorShape: Qt.PointingHandCursor
    onClicked: {
        stackView.push("qrc:/qml/pages/RegisterPage.qml")
    }
}
```

### 3.3 给 ChatPage.qml 加一个返回按钮

修改 `client/qml/pages/ChatPage.qml`：

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root
    title: qsTr("聊天")

    // 顶部工具栏
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: qsTr("← 返回")
                onClicked: root.StackView.view.pop()  // 返回上一页
            }
            Text {
                text: qsTr("ChatRoom")
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
            ToolButton {
                text: qsTr("设置")
                onClicked: console.log("打开设置")
            }
        }
    }

    // 聊天内容区域（后续实现）
    Text {
        anchors.centerIn: parent
        text: qsTr("聊天页面 - 待实现")
        font.pixelSize: 16
        color: "#999999"
    }
}
```

**运行效果**：登录页 → 点击登录 → 跳到聊天页 → 点返回 → 回到登录页

---

## 第四步：理解 QML 和 C++ 怎么通信（Day 3-5）

这是你作为角色 A **最核心的技能**。整个 qmlbridge/ 目录的存在就是为了这个。

### 4.1 通信原理（一张图看懂）

```
┌─────────────────────────────────────────────────┐
│                   QML 界面层                      │
│  LoginPage.qml / ChatPage.qml / 组件们           │
│                                                 │
│   用户点击按钮 → 调用 C++ 的函数                  │
│   C++ 发出信号 → QML 界面自动更新                 │
└──────────────┬──────────────┬───────────────────┘
               │ 调用          │ 监听信号
               ▼              ▼
┌─────────────────────────────────────────────────┐
│              qmlbridge 桥接层（你写的）            │
│  AppCore / UserController / ChatController ...   │
│                                                 │
│   暴露函数给 QML（Q_INVOKABLE）                   │
│   发出信号通知 QML（signals）                     │
└──────────────┬──────────────┬───────────────────┘
               │ 调用          │ 监听信号
               ▼              ▼
┌─────────────────────────────────────────────────┐
│           后端模块（队友 B 和 C 写的）              │
│  Core / Network / Database / Crypto              │
└─────────────────────────────────────────────────┘
```

### 4.2 实战：让 QML 调用 C++ 函数

以 UserController 为例，实现"QML 点击登录按钮 → 调用 C++ 的 login 函数"。

#### 4.2.1 修改 `client/src/qmlbridge/UserController.h`

```cpp
#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include <QObject>
#include <QString>

namespace client {

class UserController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // ② 把属性暴露给 QML，QML 可以直接读取和绑定
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY loginStatusChanged)
    Q_PROPERTY(QString username READ username NOTIFY loginStatusChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY loginStatusChanged)

public:
    explicit UserController(QObject *parent = nullptr);
    ~UserController() override;

    bool isLoggedIn() const { return m_isLoggedIn; }
    QString username() const { return m_username; }
    QString errorMessage() const { return m_errorMessage; }

    // ① Q_INVOKABLE：让这个函数可以在 QML 中直接调用
    Q_INVOKABLE void login(const QString &username, const QString &password);
    Q_INVOKABLE void register(const QString &username, const QString &password);
    Q_INVOKABLE void logout();

signals:
    // ③ 信号：C++ 状态变化时通知 QML
    void loginStatusChanged();
    void loginSuccess(const QString &username);
    void loginFailed(const QString &error);

private:
    bool m_isLoggedIn = false;
    QString m_username;
    QString m_errorMessage;
};

} // namespace client

#endif // USERCONTROLLER_H
```

#### 4.2.2 修改 `client/src/qmlbridge/UserController.cpp`

```cpp
#include "UserController.h"
#include <QDebug>

namespace client {

UserController::UserController(QObject *parent)
    : QObject(parent)
{
}

UserController::~UserController()
{
}

void UserController::login(const QString &username, const QString &password)
{
    qDebug() << "C++ login called:" << username;

    // TODO: 这里后续会调用角色 C（网络层）的接口发送登录请求到服务端
    // 现在先做模拟登录
    if (username.isEmpty() || password.isEmpty()) {
        m_errorMessage = "用户名和密码不能为空";
        emit loginFailed(m_errorMessage);
        emit loginStatusChanged();
        return;
    }

    // 模拟登录成功
    m_isLoggedIn = true;
    m_username = username;
    m_errorMessage = "";
    emit loginSuccess(username);
    emit loginStatusChanged();
}

void UserController::register(const QString &username, const QString &password)
{
    qDebug() << "C++ register called:" << username;
    // TODO: 调用网络层发送注册请求
}

void UserController::logout()
{
    m_isLoggedIn = false;
    m_username = "";
    emit loginStatusChanged();
}

} // namespace client
```

#### 4.2.3 修改 `client/src/main.cpp`（注册 C++ 到 QML）

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

// 引入你的 Bridge 类
#include "qmlbridge/UserController.h"
#include "qmlbridge/AppCore.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // 创建 C++ 对象
    // 注意：因为用了 QML_SINGLETON，Qt 会自动注册单例
    // 所以这里不需要手动 setContextProperty

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}
```

#### 4.2.4 修改 `LoginPage.qml`（在 QML 中调用 C++）

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root
    title: qsTr("登录")

    background: Rectangle {
        color: "#f5f5f5"
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20
        width: parent.width * 0.8

        Text {
            text: qsTr("ChatRoom")
            font.pixelSize: 36
            font.bold: true
            color: "#333333"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: qsTr("局域网即时通讯")
            font.pixelSize: 14
            color: "#999999"
            Layout.alignment: Qt.AlignHCenter
        }

        TextField {
            id: usernameField
            placeholderText: qsTr("请输入用户名")
            font.pixelSize: 16
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            leftPadding: 16
            background: Rectangle {
                radius: 8
                color: "#ffffff"
                border.color: usernameField.activeFocus ? "#4A90D9" : "#dddddd"
                border.width: 1
            }
        }

        TextField {
            id: passwordField
            placeholderText: qsTr("请输入密码")
            font.pixelSize: 16
            echoMode: TextInput.Password
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            leftPadding: 16
            background: Rectangle {
                radius: 8
                color: "#ffffff"
                border.color: passwordField.activeFocus ? "#4A90D9" : "#dddddd"
                border.width: 1
            }
        }

        // 错误提示（登录失败时显示）
        Text {
            id: errorText
            text: UserController.errorMessage  // 绑定 C++ 属性
            color: "#ff4444"
            font.pixelSize: 13
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            visible: text !== ""  // 有错误信息时才显示
        }

        Button {
            text: qsTr("登 录")
            font.pixelSize: 18
            font.bold: true
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            background: Rectangle {
                radius: 8
                color: parent.down ? "#3a7bc8" : "#4A90D9"
            }
            contentItem: Text {
                text: parent.text
                color: "#ffffff"
                font: parent.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                // ★ 调用 C++ 的 login 函数 ★
                UserController.login(usernameField.text, passwordField.text)
            }
        }

        Text {
            text: qsTr("没有账号？立即注册")
            font.pixelSize: 14
            color: "#4A90D9"
            Layout.alignment: Qt.AlignHCenter

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    stackView.push("qrc:/qml/pages/RegisterPage.qml")
                }
            }
        }
    }

    // ★ 监听 C++ 的信号 ★
    Connections {
        target: UserController

        function onLoginSuccess(username) {
            console.log("登录成功！欢迎", username)
            stackView.push("qrc:/qml/pages/ChatPage.qml")
        }

        function onLoginFailed(error) {
            console.log("登录失败:", error)
        }
    }
}
```

**运行效果**：
- 输入用户名密码 → 点登录 → 控制台打印"C++ login called: xxx" → 自动跳转到聊天页
- 不输入直接点登录 → 显示红色错误提示"用户名和密码不能为空"

---

## 第五步：实现聊天页面的基本布局（Day 5-7）

**目标**：ChatPage 有会话列表 + 聊天区域 + 消息输入栏。

### 5.1 修改 `ChatPage.qml`

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root
    title: qsTr("聊天")

    // 整体水平布局：左侧会话列表 + 右侧聊天区域
    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ====== 左侧：会话列表 ======
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 280
            color: "#f0f0f0"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // 搜索栏
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    color: "#ffffff"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 8

                        TextField {
                            id: searchField
                            Layout.fillWidth: true
                            placeholderText: qsTr("搜索")
                            font.pixelSize: 14
                            background: Rectangle {
                                radius: 4
                                color: "#f5f5f5"
                            }
                        }
                    }
                }

                // 会话列表（后续用 ListView + ConversationModel）
                ListView {
                    id: conversationList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: ListModel {  // 临时用 ListModel，后续换成 ConversationModel
                        ListElement { name: "张三"; lastMsg: "你好！"; time: "10:30"; unread: 2 }
                        ListElement { name: "李四"; lastMsg: "明天见"; time: "09:15"; unread: 0 }
                        ListElement { name: "王五"; lastMsg: "收到"; time: "昨天"; unread: 0 }
                    }
                    delegate: ConversationItem {
                        width: conversationList.width
                        name: model.name
                        lastMessage: model.lastMsg
                        time: model.time
                        unreadCount: model.unread
                        onClicked: console.log("打开与", name, "的聊天")
                    }
                }
            }
        }

        // ====== 右侧：聊天区域 ======
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#ebebeb"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // 聊天对象名称栏
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    color: "#ffffff"

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("张三")
                        font.pixelSize: 16
                        font.bold: true
                    }
                }

                // 消息列表区域
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ListView {
                        id: messageList
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 8
                        verticalLayoutDirection: ListView.BottomToTop  // 新消息在底部

                        model: ListModel {  // 临时数据，后续换成 MessageModel
                            ListElement { content: "你好！"; isMine: false; time: "10:30" }
                            ListElement { content: "你好呀，最近怎么样？"; isMine: true; time: "10:31" }
                            ListElement { content: "挺好的，你呢？"; isMine: false; time: "10:32" }
                        }

                        delegate: MessageBubble {
                            width: messageList.width - 20
                            content: model.content
                            isMine: model.isMine
                            time: model.time
                        }
                    }
                }

                // 消息输入栏
                InputBar {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 60
                    onSendMessage: function(text) {
                        console.log("发送消息:", text)
                        // TODO: 调用 ChatController 发送消息
                    }
                }
            }
        }
    }
}
```

### 5.2 修改 `client/qml/components/MessageBubble.qml`

```qml
import QtQuick
import QtQuick.Controls

Item {
    id: root
    height: bubbleColumn.height + 10

    // 暴露给外部的属性
    property string content: ""
    property bool isMine: false
    property string time: ""

    signal clicked()

    Column {
        id: bubbleColumn
        width: parent.width
        spacing: 2

        // 时间标签
        Text {
            text: root.time
            font.pixelSize: 11
            color: "#999999"
            anchors.horizontalCenter: parent.horizontalCenter
            visible: root.time !== ""
        }

        // 消息气泡行
        Row {
            spacing: 8
            layoutDirection: root.isMine ? Qt.RightToLeft : Qt.LeftToRight
            anchors.right: root.isMine ? parent.right : undefined
            anchors.left: root.isMine ? undefined : parent.left
            leftPadding: 4
            rightPadding: 4

            // 头像（后续用 Avatar 组件）
            Rectangle {
                width: 36
                height: 36
                radius: 18
                color: root.isMine ? "#4A90D9" : "#67C23A"
                Text {
                    anchors.centerIn: parent
                    text: root.isMine ? "我" : "他"
                    color: "#ffffff"
                    font.pixelSize: 14
                }
            }

            // 气泡
            Rectangle {
                width: Math.min(bubbleText.implicitWidth + 24, root.width * 0.6)
                height: bubbleText.implicitHeight + 16
                radius: 8
                color: root.isMine ? "#95ec69" : "#ffffff"

                Text {
                    id: bubbleText
                    text: root.content
                    font.pixelSize: 15
                    color: "#333333"
                    wrapMode: Text.Wrap
                    anchors.fill: parent
                    anchors.margins: 8
                }
            }
        }
    }
}
```

### 5.3 修改 `client/qml/components/InputBar.qml`

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    // 信号：用户点击发送时触发
    signal sendMessage(string text)

    Rectangle {
        anchors.fill: parent
        color: "#ffffff"
        border.color: "#e0e0e0"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 8

            TextField {
                id: inputField
                Layout.fillWidth: true
                Layout.fillHeight: true
                placeholderText: qsTr("输入消息...")
                font.pixelSize: 15
                wrapMode: TextArea.Wrap
                background: null

                // 按回车发送
                Keys.onReturnPressed: {
                    if (inputField.text.trim() !== "") {
                        root.sendMessage(inputField.text)
                        inputField.text = ""
                    }
                }
            }

            Button {
                text: qsTr("发送")
                font.pixelSize: 14
                Layout.preferredWidth: 60
                Layout.fillHeight: true
                background: Rectangle {
                    radius: 6
                    color: parent.down ? "#3a7bc8" : "#4A90D9"
                }
                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (inputField.text.trim() !== "") {
                        root.sendMessage(inputField.text)
                        inputField.text = ""
                    }
                }
            }
        }
    }
}
```

### 5.4 修改 `client/qml/components/ConversationItem.qml`

```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    height: 64
    signal clicked()

    // 暴露给外部的属性
    property string name: ""
    property string lastMessage: ""
    property string time: ""
    property int unreadCount: 0

    Rectangle {
        anchors.fill: parent
        color: root.MouseArea.containsMouse ? "#e8e8e8" : "#f0f0f0"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            // 头像
            Rectangle {
                width: 44
                height: 44
                radius: 22
                color: "#4A90D9"
                Text {
                    anchors.centerIn: parent
                    text: root.name.charAt(0)
                    color: "#ffffff"
                    font.pixelSize: 18
                    font.bold: true
                }
            }

            // 名称 + 最后消息
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: root.name
                        font.pixelSize: 15
                        font.bold: true
                        color: "#333333"
                        Layout.fillWidth: true
                    }
                    Text {
                        text: root.time
                        font.pixelSize: 12
                        color: "#999999"
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: root.lastMessage
                        font.pixelSize: 13
                        color: "#999999"
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    // 未读计数气泡
                    Rectangle {
                        visible: root.unreadCount > 0
                        width: 18
                        height: 18
                        radius: 9
                        color: "#ff4444"
                        Text {
                            anchors.centerIn: parent
                            text: root.unreadCount
                            color: "#ffffff"
                            font.pixelSize: 10
                        }
                    }
                }
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
```

**运行效果**：左侧会话列表 + 右侧聊天区域，有头像、消息气泡、输入栏，看起来像一个真正的聊天应用了！

---

## 第六步：后续开发路线图

完成以上 5 步后，你已经掌握了 QML 开发的核心技能。后续按这个顺序继续：

### 你接下来要做的事

| 优先级 | 任务 | 涉及文件 | 依赖队友 |
|--------|------|---------|---------|
| ⭐⭐⭐ | 完善 RegisterPage.qml 注册界面 | `pages/RegisterPage.qml` | 无 |
| ⭐⭐⭐ | 实现 ChatController（消息收发 Bridge） | `qmlbridge/ChatController.h/.cpp` | 角色 C（网络层就绪后） |
| ⭐⭐⭐ | 实现 ConversationModel（会话列表数据） | `models/ConversationModel.h/.cpp` | 角色 B（数据库就绪后） |
| ⭐⭐⭐ | 实现 MessageModel（消息列表数据） | `models/MessageModel.h/.cpp` | 角色 B |
| ⭐⭐ | 实现 FriendController + FriendRequestPage | `qmlbridge/FriendController` + `pages/FriendRequestPage.qml` | 角色 C、D |
| ⭐⭐ | 实现 ProfilePage.qml 个人资料页 | `pages/ProfilePage.qml` | 角色 B |
| ⭐⭐ | 实现 SettingsPage.qml 设置页 | `pages/SettingsPage.qml` | 角色 B |
| ⭐ | 实现 NotificationController 通知 | `qmlbridge/NotificationController.h/.cpp` | 无 |
| ⭐ | 实现 FileTransferController 文件传输 | `qmlbridge/FileTransferController.h/.cpp` | 角色 C |
| ⭐ | 实现 GroupChatCreatePage 群聊 | `pages/GroupChatCreatePage.qml` | 角色 C、D |

### 你的开发节奏建议

```
第 1 周：完成第一步~第三步（登录界面 + 页面导航）
第 2 周：完成第四步~第五步（QML 调用 C++ + 聊天界面布局）
第 3 周：完善所有页面 UI（注册、好友、个人资料、设置）
第 4 周+：等队友接口就绪后，对接 Bridge 层
```

---

## 常见问题速查

### Q：QML 报错 "ReferenceError: UserController is not defined"
**A**：检查 `UserController.h` 里有没有 `QML_ELEMENT` 和 `QML_SINGLETON` 宏。如果没有，QML 找不到这个 C++ 类。

### Q：修改了 QML 但界面没变化
**A**：需要重新编译（Ctrl+B），因为 QML 文件打包在 .qrc 资源文件里。

### Q：StackView.push 报错
**A**：检查路径是否正确，必须用 `qrc:/` 前缀，如 `"qrc:/qml/pages/ChatPage.qml"`。

### Q：属性绑定不更新
**A**：检查 C++ 里的 `emit xxxChanged()` 信号有没有发出来。QML 属性绑定依赖信号通知。

### Q：怎么调试 QML
**A**：在 QML 里用 `console.log("调试信息", 变量名)`，输出在 Qt Creator 的"应用程序输出"面板。

---

## 你负责的文件清单（打印出来当 checklist）

### qmlbridge/（6 个类，Bridge 桥接层）
- [ ] `AppCore.h/.cpp` — 应用核心单例
- [ ] `UserController.h/.cpp` — 登录注册
- [ ] `ChatController.h/.cpp` — 聊天消息
- [ ] `FriendController.h/.cpp` — 好友管理
- [ ] `FileTransferController.h/.cpp` — 文件传输
- [ ] `NotificationController.h/.cpp` — 通知

### models/（4 个类，数据模型）
- [ ] `UserModel.h/.cpp` — 用户/联系人列表
- [ ] `MessageModel.h/.cpp` — 聊天消息列表
- [ ] `ConversationModel.h/.cpp` — 会话列表
- [ ] `FriendRequestModel.h/.cpp` — 好友请求列表

### qml/pages/（7 个页面）
- [ ] `main.qml` — 应用入口
- [ ] `LoginPage.qml` — 登录页
- [ ] `RegisterPage.qml` — 注册页
- [ ] `ChatPage.qml` — 聊天主页
- [ ] `FriendRequestPage.qml` — 好友请求页
- [ ] `ProfilePage.qml` — 个人资料页
- [ ] `GroupChatCreatePage.qml` — 创建群聊页
- [ ] `SettingsPage.qml` — 设置页

### qml/components/（10 个组件）
- [ ] `MessageBubble.qml` — 消息气泡
- [ ] `ContactItem.qml` — 联系人列表项
- [ ] `ConversationItem.qml` — 会话列表项
- [ ] `InputBar.qml` — 消息输入栏
- [ ] `Avatar.qml` — 头像
- [ ] `SearchBar.qml` — 搜索栏
- [ ] `TabBar.qml` — 底部标签栏
- [ ] `Toast.qml` — 提示框
- [ ] `DialogHelper.qml` — 对话框辅助
- [ ] `ImagePreviewer.qml` — 图片预览器

### qml/dialogs/（4 个对话框）
- [ ] `AddFriendDialog.qml` — 添加好友
- [ ] `GroupChatDialog.qml` — 创建群聊
- [ ] `FileTransferDialog.qml` — 文件传输
- [ ] `ConfirmDialog.qml` — 通用确认框

### 其他
- [ ] `client/CMakeLists.txt` — 构建配置
- [ ] `client/src/main.cpp` — 客户端入口
- [ ] `client/resources/qml.qrc` — QML 资源注册
