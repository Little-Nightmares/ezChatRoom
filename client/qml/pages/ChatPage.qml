// ==================== Qt Quick模块导入 ====================
// 导入Qt Quick核心模块，提供QML类型和功能
import QtQuick
// 导入Qt Quick Controls模块，提供UI控件如Button、ListView等
import QtQuick.Controls
// 导入Qt Quick Layouts模块，提供布局管理器如RowLayout、ColumnLayout等
import QtQuick.Layouts
// 导入组件模块，使InputBar等组件可用
import "../components"

// ==================== 页面根元素 ====================
// 页面组件，作为聊天页面的根容器
Page {
    id: root  // 页面根元素唯一标识符，用于在组件内部引用自身

    // 页面背景
    background: Rectangle {
        color: themeManager.currentTheme.background
    }

    // ==================== 个人详情弹窗 ====================
    ProfilePopup {
        id: profilePopup  // 个人详情弹窗
    }

    // ==================== 聊天内容区域 ====================
    // 水平布局容器，左边是侧边栏，右边是聊天区域
    RowLayout {
        anchors.fill: parent  // 填充整个页面内容区域
        spacing: 0  // 子元素之间无间距

        // ==================== 左侧边栏 ====================
        // 侧边栏容器，使用固定比例宽度（占父容器25%）
        Rectangle {
            id: sidebar  // 侧边栏唯一标识符
            Layout.fillHeight: true  // 高度占据所有可用空间
            Layout.preferredWidth: parent.width * 0.25  // 宽度占父容器的25%
            Layout.maximumWidth: 300  // 最大宽度300像素
            Layout.minimumWidth: 200  // 最小宽度200像素
            Layout.leftMargin: 10  // 左边距10像素
            Layout.topMargin: 10  // 上边距10像素
            Layout.bottomMargin: 10  // 下边距10像素
            color: themeManager.currentTheme.surface  // 背景色使用主题表面色

            // 侧边栏内容：垂直布局
            ColumnLayout {
                anchors.fill: parent  // 填充整个侧边栏
                spacing: 10  // 子元素间距10像素

                // ==================== 好友列表区域 ====================
                // 好友列表区域，使用固定比例高度
                Rectangle {
                    Layout.fillWidth: true  // 宽度占据所有可用空间
                    Layout.preferredHeight: parent.height * 0.20  // 高度占父容器的20%
                    color: themeManager.currentTheme.background  // 背景色使用主题背景色
                    radius: 8  // 圆角8像素

                    Text {
                        anchors.centerIn: parent  // 文本居中
                        text: "好友列表"  // 临时文本
                        color: themeManager.currentTheme.text  // 文字颜色
                    }
                }

                // ==================== 聊天室选择区域 ====================
                // 聊天室区域，使用固定比例高度
                Rectangle {
                    Layout.fillWidth: true  // 宽度占据所有可用空间
                    Layout.preferredHeight: parent.height * 0.20  // 高度占父容器的20%
                    color: themeManager.currentTheme.background  // 背景色使用主题背景色
                    radius: 8  // 圆角8像素

                    Text {
                        anchors.centerIn: parent  // 文本居中
                        text: "聊天室"  // 临时文本
                        color: themeManager.currentTheme.text  // 文字颜色
                    }
                }

                // ==================== 个人主页区域 ====================
                // 个人主页区域，填充剩余空间
                Rectangle {
                    id: profilePanel  // 个人主页区域唯一标识符
                    Layout.fillWidth: true  // 宽度占据所有可用空间
                    Layout.fillHeight: true  // 高度占据所有剩余空间
                    color: themeManager.currentTheme.background  // 背景色使用主题背景色
                    radius: 8  // 圆角8像素

                    // 个人主页内容：垂直布局
                    ColumnLayout {
                        anchors.fill: parent  // 填充整个个人主页区域
                        anchors.margins: 15  // 内边距15像素
                        spacing: 12  // 子元素间距12像素

                        // ==================== 用户头像区域（可点击自定义）====================
                        Rectangle {
                            id: avatarContainer  // 头像容器
                            Layout.alignment: Qt.AlignHCenter  // 水平居中
                            width: 80  // 头像宽度80像素
                            height: 80  // 头像高度80像素
                            radius: 40  // 圆角40像素，变成圆形
                            color: themeManager.currentTheme.primary  // 头像背景色使用主题主色

                            // 头像点击区域（用于打开个人详情弹窗）
                            MouseArea {
                                anchors.fill: parent  // 填充整个头像区域
                                onClicked: {
                                    console.log("头像点击：打开个人详情弹窗")
                                    profilePopup.open()  // 打开个人详情弹窗
                                }
                            }

                            // 头像中的用户图标或首字母
                            // text属性绑定到当前用户名的首字母，后续可改为显示用户头像图片
                            Text {
                                id: avatarText  // 头像文字
                                anchors.centerIn: parent  // 文本居中
                                // 后续可以从UserController获取用户首字母
                                text: UserController.loggedInUser !== "" ? UserController.loggedInUser.charAt(0).toUpperCase() : "U"
                                color: "white"  // 文字颜色白色
                                font.pixelSize: 32  // 字体大小32像素
                                font.bold: true  // 字体加粗
                            }

                            // 头像上的相机图标（表示可点击编辑）
                            Rectangle {
                                anchors.right: parent.right  // 靠右
                                anchors.bottom: parent.bottom  // 靠下
                                width: 24  // 宽度24像素
                                height: 24  // 高度24像素
                                radius: 12  // 圆角12像素
                                color: themeManager.currentTheme.secondary  // 使用次要色

                                Text {
                                    anchors.centerIn: parent  // 居中
                                    text: "📷"  // 相机表情符号
                                    font.pixelSize: 12  // 字体大小12像素
                                }
                            }
                        }

                        // ==================== 用户昵称 ====================
                        // 后续绑定到UserController的用户名
                        Text {
                            id: usernameText  // 用户名文本
                            Layout.alignment: Qt.AlignHCenter  // 水平居中
                            // 后续从UserController获取用户名
                            text: UserController.loggedInUser !== "" ? UserController.loggedInUser : "未登录用户"
                            color: themeManager.currentTheme.text  // 文字颜色
                            font.pixelSize: 18  // 字体大小18像素
                            font.bold: true  // 字体加粗
                        }

                        // ==================== 在线状态 ====================
                        RowLayout {
                            id: statusRow  // 状态行
                            Layout.alignment: Qt.AlignHCenter  // 水平居中
                            spacing: 5  // 元素间距5像素

                            // 状态指示点
                            Rectangle {
                                width: 10  // 宽度10像素
                                height: 10  // 高度10像素
                                radius: 5  // 圆角5像素
                                // 根据连接状态显示不同颜色
                                color: AppManager.isConnected ? "#4CAF50" : "#9E9E9E"  // 在线：绿色，离线：灰色
                            }

                            // 状态文本
                            Text {
                                text: AppManager.isConnected ? "在线" : "离线"  // 根据连接状态显示
                                color: themeManager.currentTheme.text  // 文字颜色
                                font.pixelSize: 12  // 字体大小12像素
                                opacity: 0.7  // 略微透明
                            }
                        }

                        // ==================== 分隔线 ====================
                        Rectangle {
                            Layout.fillWidth: true  // 宽度占据所有可用空间
                            height: 1  // 高度1像素
                            color: themeManager.currentTheme.border  // 分隔线颜色
                        }

                        // ==================== 功能按钮区域 ====================
                        ColumnLayout {
                            Layout.fillWidth: true  // 宽度占据所有可用空间
                            spacing: 8  // 按钮间距8像素

                            // 消息通知按钮
                            Button {
                                id: notificationButton  // 通知按钮
                                Layout.fillWidth: true  // 宽度占据所有可用空间
                                height: 40  // 高度40像素
                                // 根据通知设置状态显示不同文本
                                text: AppManager.notificationsEnabled ? "🔔 通知: 开" : "🔕 通知: 关"

                                background: Rectangle {
                                    color: themeManager.currentTheme.secondary
                                    radius: 6
                                }

                                contentItem: Text {
                                    text: parent.text
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: {
                                    // 切换通知状态
                                    AppManager.notificationsEnabled = !AppManager.notificationsEnabled
                                    console.log("通知设置:", AppManager.notificationsEnabled ? "开启" : "关闭")
                                }
                            }

                            // 设置按钮 - 跳转到设置页面
                            Button {
                                id: settingsButton  // 设置按钮
                                Layout.fillWidth: true  // 宽度占据所有可用空间
                                height: 40  // 高度40像素
                                text: "⚙️ 设置"

                                background: Rectangle {
                                    color: themeManager.currentTheme.surface
                                    border.color: themeManager.currentTheme.border
                                    border.width: 1
                                    radius: 6
                                }

                                contentItem: Text {
                                    text: parent.text
                                    color: themeManager.currentTheme.text
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: {
                                    console.log("跳转到设置页面")
                                    // 跳转到设置页面
                                    stackView.push("qrc:/qml/pages/SettingsPage.qml")
                                }
                            }

                            // 退出登录按钮
                            Button {
                                id: logoutButton  // 退出登录按钮
                                Layout.fillWidth: true  // 宽度占据所有可用空间
                                height: 40  // 高度40像素
                                text: "🚪 退出登录"
                                visible: UserController.loggedInUser !== ""  // 已登录时才显示

                                background: Rectangle {
                                    color: "#F44336"  // 红色背景表示危险操作
                                    radius: 6
                                }

                                contentItem: Text {
                                    text: parent.text
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: {
                                    console.log("退出登录")
                                    // TODO: 调用UserController.logout()
                                    // UserController.logout()
                                    // 返回登录页面
                                    stackView.replace("qrc:/qml/pages/LoginPage.qml")
                                }
                            }
                        }
                    }
                }
            }
        }

        // ==================== 右侧聊天区域 ====================
        // 右侧聊天区域的垂直布局容器
        ColumnLayout {
            Layout.fillWidth: true  // 宽度占据剩余所有空间
            Layout.fillHeight: true  // 高度占据所有可用空间
            spacing: 0  // 子元素之间无间距

            // ==================== 消息列表 ====================
            // 用于显示聊天消息列表的视图组件
            ListView {
                id: messageList  // 列表视图唯一标识符

                Layout.fillWidth: true  // 宽度占据所有可用空间
                Layout.fillHeight: true  // 高度占据所有可用空间（除了输入栏）

                model: messageModel  // 绑定消息数据模型，提供列表数据源
                delegate: messageDelegate  // 绑定消息委托组件，定义每条消息的渲染方式

                // 当列表中的项目数量发生变化时触发
                onCountChanged: {
                    if (count > 0) {  // 如果列表中有项目
                        positionViewAtEnd()  // 自动滚动到列表末尾，显示最新消息
                    }
                }
            }

            // ==================== 输入栏 ====================
            // 用户输入消息和发送消息的组件
            InputBar {
                Layout.fillWidth: true  // 宽度填充父容器

                // 处理发送消息事件：当用户点击发送按钮或按回车键时触发
                // 使用JavaScript函数明确声明参数
                onSendMessage: function(text) {
                    // 测试：输出到控制台
                    console.log("=== ChatController Test ===")
                    console.log("onSendMessage called with:", text)

                    // 调用chatController发送消息
                    chatController.sendMessage("receiverId", text)

                    console.log("=== ChatController Test Complete ===")

                    // 在本地消息模型中立即添加消息（乐观更新，提升用户体验）
                    // 实际发送结果会在收到服务器响应后确认
                    messageModel.addMessage({
                        "id": "temp_" + Date.now(),  // 生成临时消息ID，基于当前时间戳
                        "message": text,  // 消息内容，来自用户输入
                        "sender": "me",  // 发送者标记为自己
                        "isOwn": true,  // 标记为已发送的消息
                        "timestamp": new Date().toLocaleTimeString()  // 当前时间作为时间戳
                    })
                }
            }
        }
    }

    // ==================== 消息数据模型 ====================
    // QML列表模型，用于存储和管理聊天消息数据
    ListModel {
        id: messageModel  // 模型唯一标识符

        // 添加消息到模型的函数
        function addMessage(message) {
            append(message)  // 将消息对象追加到模型末尾
        }
    }

    // ==================== 消息委托组件 ====================
    // 消息列表项的委托组件，定义单条消息的显示样式
    Component {
        id: messageDelegate  // 委托组件唯一标识符

        // 委托项的根元素
        Item {
            width: parent.width  // 宽度与父元素（列表项）宽度相同
            height: bubble.height + 30  // 高度根据气泡高度自动调整，额外增加间距

            // ==================== 发送者头像（对方消息显示） ====================
            Rectangle {
                id: avatar  // 头像容器ID
                anchors.left: parent.left  // 头像贴在父元素左侧
                anchors.leftMargin: 10  // 左边距10像素
                anchors.verticalCenter: bubble.verticalCenter  // 与气泡垂直居中
                width: 40  // 头像宽度40像素
                height: 40  // 头像高度40像素
                radius: 20  // 圆角半径20像素，将正方形变为圆形
                color: themeManager.currentTheme.primary  // 头像背景色使用主题色
                visible: !isOwn  // 只有对方消息才显示头像

                // 头像中的首字母文本
                Text {
                    anchors.centerIn: parent  // 文本居中于父元素
                    text: sender.charAt(0).toUpperCase()  // 获取发送者名称的首字母并转为大写
                    color: "white"  // 文字颜色为白色
                    font.bold: true  // 字体加粗
                }
            }

            // ==================== 自己头像（自己消息显示） ====================
            Rectangle {
                id: ownAvatar  // 自己头像容器ID
                anchors.right: parent.right  // 头像贴在父元素右侧
                anchors.rightMargin: 10  // 右边距10像素
                anchors.verticalCenter: bubble.verticalCenter  // 与气泡垂直居中
                width: 40  // 头像宽度40像素
                height: 40  // 头像高度40像素
                radius: 20  // 圆角半径20像素，将正方形变为圆形
                color: themeManager.currentTheme.secondary  // 自己头像使用次要色
                visible: isOwn  // 只有自己消息才显示头像

                // 头像中的首字母文本
                Text {
                    anchors.centerIn: parent  // 文本居中于父元素
                    text: sender.charAt(0).toUpperCase()  // 获取发送者名称的首字母并转为大写
                    color: "white"  // 文字颜色为白色
                    font.bold: true  // 字体加粗
                }
            }

            // ==================== 消息气泡 ====================
            Rectangle {
                id: bubble  // 气泡容器ID，用于引用
                // 对方消息：气泡紧跟头像，不设置右边锚定以实现宽度自适应
                anchors.left: isOwn ? undefined : avatar.right  // 对方消息紧跟头像
                anchors.leftMargin: isOwn ? 0 : 8  // 对方消息左边距8像素
                // 自己消息：气泡在右侧头像左边
                anchors.right: isOwn ? ownAvatar.left : undefined  // 自己消息气泡靠右
                anchors.rightMargin: isOwn ? 8 : undefined  // 自己消息右边距8像素
                anchors.top: parent.top  // 顶部对齐
                anchors.topMargin: 5  // 顶部间距

                // 气泡宽度根据文本内容自适应，最大为父容器宽度的70%
                width: Math.min(messageText.implicitWidth + 40, parent.width * 0.7)
                // 气泡高度根据文本内容自适应
                height: messageText.implicitHeight + 24
                radius: 12  // 圆角半径12像素

                // 气泡背景色根据发送者决定
                // 自己发送的消息使用主题色（主色调）
                // 对方发送的消息使用表面色（浅色背景）
                color: isOwn ? themeManager.currentTheme.primary : themeManager.currentTheme.surface

                // 气泡边框：1像素宽的主题边框色
                border.color: themeManager.currentTheme.border
                border.width: 1

                // ==================== 气泡内消息文本 ====================
                Text {
                    id: messageText  // 文本元素ID，用于获取尺寸
                    anchors.centerIn: parent  // 在气泡内居中显示
                    text: message  // 显示实际的消息内容
                    // 文字颜色根据发送者决定
                    // 自己发送的消息使用白色文字（在主题色背景上）
                    // 对方发送的消息使用主题文字色（在浅色背景上）
                    color: isOwn ? "white" : themeManager.currentTheme.text
                    wrapMode: Text.WordWrap  // 自动换行，确保长文本不会超出气泡宽度
                    // 文本最大宽度为气泡宽度减去40像素的内边距
                    width: bubble.width - 40
                    font.pixelSize: 14  // 字体大小14像素
                }
            }

            // ==================== 时间戳标签（放在 Item 内部，用负 margin 超出边界） ====================
            Text {
                id: timestampText  // 时间戳ID
                anchors.top: bubble.bottom  // 时间戳在气泡下方
                anchors.topMargin: 5  // 与气泡间距5像素
                anchors.right: isOwn ? undefined : bubble.right  // 自己消息时间戳在左侧
                anchors.left: isOwn ? bubble.left : undefined  // 自己消息时间戳在左侧
                text: timestamp  // 显示时间戳文本
                font.pixelSize: 10  // 小号字体（10像素）
                color: themeManager.currentTheme.text  // 使用主题文字色，更清晰
                opacity: 0.7  // 略微透明但仍清晰可见
            }
        }
    }

    // ==================== 页面初始化 ====================
    // 页面加载完成时执行的初始化操作
    Component.onCompleted: {
        // 注意：chatController 的 C++ 方法尚未实现，以下为模拟数据
        // chatController.loadMessages("conversationId", 75, 0)  // 加载历史消息（待实现）

        // 添加模拟消息用于测试UI效果
        // 对方发送的消息
        messageModel.addMessage({
            "id": "1",  // 消息ID
            "message": "你好！我来看看你开发的怎么样了",  // 消息内容
            "sender": "Alice",  // 发送者名称
            "isOwn": false,  // 不是自己的消息
            "timestamp": "10:00"  // 时间戳
        })

        // 自己发送的消息
        messageModel.addMessage({
            "id": "2",  // 消息ID
            "message": "你好，最近怎么样？",  // 消息内容
            "sender": "Me",  // 发送者名称
            "isOwn": true,  // 是自己的消息
            "timestamp": "10:01"  // 时间戳
        })
    }
}
