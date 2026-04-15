import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page // 登录界面的主页面
{
    id: root // 给页面设置ID，便于其他组件引用
    
    // 背景图片 - 全屏显示
    Image {
        id: backgroundImage // 背景图片组件ID
        source: "qrc:///qml/images/zmd2.jpg" // 图片文件路径（资源系统路径）
        anchors.fill: parent // 填充整个父容器（窗口）
        fillMode: Image.PreserveAspectCrop // 裁剪图片以保持比例并填充容器
    }
    
    // 右侧登录区域容器
    Item {
        id: loginPanel // 登录面板ID
        width: parent.width * 0.4 // 宽度为窗口宽度的40%
        height: parent.height // 高度与窗口相同
        anchors.right: parent.right // 锚定在窗口右侧
        
        // 登录表单容器 - 白色卡片样式
        Rectangle {
            id: loginForm // 登录表单ID
            width: Math.min(parent.width * 0.8, 400) // 表单宽度，最大400像素
            height: Math.min(parent.height * 0.8, 550) // 表单高度，最大550像素
            anchors.centerIn: parent // 在登录面板中居中显示
            radius: 20 // 圆角半径20像素
            color: "white" // 白色背景
            
            // Logo图片
            Image {
                id: logoImage // Logo图片ID
                source: "qrc:///qml/images/logo2.png" // Logo图片路径（资源系统路径）
                width: Math.min(parent.width * 0.25, 100) // Logo宽度，最大100像素
                height: width // 保持宽高比
                anchors.horizontalCenter: parent.horizontalCenter // 水平居中
                anchors.top: parent.top // 锚定在表单顶部
                anchors.topMargin: parent.height * 0.05 // 顶部边距，使用相对值
                fillMode: Image.PreserveAspectFit // 保持图片比例
            }
            
            // 标题文本
            Text {
                id: titleText // 标题文本ID
                text: "终末地个人终端登录系统" // 文本内容
                anchors.horizontalCenter: parent.horizontalCenter // 水平居中
                anchors.top: logoImage.bottom // 锚定在Logo下方
                anchors.topMargin: parent.height * 0.03 // 顶部边距，使用相对值
                font.pointSize: Math.max(Math.min(parent.width * 0.06, 24), 12) // 字体大小，最大24点，最小12点
                font.bold: true // 粗体显示
                color: "#1f2937" // 深灰色文字颜色
            }
            
            // 用户名输入框容器
            Rectangle {
                id: usernameContainer // 用户名容器ID
                width: parent.width * 0.8 // 容器宽度为表单宽度的80%
                height: parent.height * 0.08 // 容器高度为表单高度的8%
                anchors.horizontalCenter: parent.horizontalCenter // 水平居中
                anchors.top: titleText.bottom // 锚定在标题下方
                anchors.topMargin: parent.height * 0.08 // 顶部边距，使用相对值
                radius: 8 // 圆角半径8像素
                border.width: 1 // 边框宽度1像素
                border.color: "#d1d5db" // 浅灰色边框颜色
                color: "white" // 白色背景
                
                // 用户名输入框
                TextField {
                    id: username // 用户名输入框ID
                    anchors.fill: parent // 填充容器
                    anchors.margins: 1 // 边距1像素
                    placeholderText: "用户名或邮箱" // 占位符文本
                    placeholderTextColor: '#25000000' // 占位符文字颜色
                    font.bold: true // 粗体显示
                    font.pointSize: Math.max(Math.min(parent.width * 0.04, 16), 10) // 字体大小，最大16点，最小10点
                    horizontalAlignment: TextInput.AlignHCenter // 水平居中对齐
                    background: null // 不设置背景，使用容器背景
                    color: "#1f2937" // 输入深灰色文字颜色
                }
            }
            
            // 密码输入框容器
            Rectangle {
                id: passwordContainer // 密码容器ID
                width: usernameContainer.width // 宽度与用户名容器相同
                height: usernameContainer.height // 高度与用户名容器相同
                anchors.horizontalCenter: parent.horizontalCenter // 水平居中
                anchors.top: usernameContainer.bottom // 锚定在用户名容器下方
                anchors.topMargin: parent.height * 0.03 // 顶部边距，使用相对值
                radius: 8 // 圆角半径8像素
                border.width: 1 // 边框宽度1像素
                border.color: "#d1d5db" // 浅灰色边框颜色
                color: "white" // 白色背景
                
                // 密码输入框
                TextField {
                    id: password // 密码输入框ID
                    anchors.fill: parent // 填充容器
                    anchors.margins: 1 // 边距1像素
                    placeholderText: "请输入密码" // 占位符文本
                    placeholderTextColor: '#25000000' // 占位符文字颜色
                    font.bold: true // 粗体显示
                    font.pointSize: username.font.pointSize // 字体大小与用户名输入框相同
                    echoMode: TextInput.Password // 密码模式，显示为圆点
                    horizontalAlignment: TextInput.AlignHCenter // 水平居中对齐
                    background: null // 不设置背景，使用容器背景
                    color: "#1f2937" // 输入深灰色文字颜色
                }

                //用户图标
                //2.5 动态图标输入

            }
            
            // 登录按钮容器
            Rectangle {
                id: submitContainer // 登录按钮容器ID
                width: passwordContainer.width // 宽度与密码容器相同
                height: parent.height * 0.09 // 容器高度为表单高度的9%
                anchors.horizontalCenter: parent.horizontalCenter // 水平居中
                anchors.top: passwordContainer.bottom // 锚定在密码容器下方
                anchors.topMargin: parent.height * 0.06 // 顶部边距，使用相对值
                radius: 8 // 圆角半径8像素
                color: "#4f46e5" // 靛蓝色背景
                
                // 加载状态指示器
                BusyIndicator {
                    id: loginLoading
                    anchors.centerIn: parent
                    running: false
                    visible: false
                    width: 24
                    height: 24
                }
                
                // 登录定时器
                Timer {
                    id: loginTimer
                    interval: 1000
                    running: false
                    repeat: false
                    onTriggered: {
                        // 隐藏加载状态
                        loginLoading.visible = false
                        loginLoading.running = false
                        submitMouseArea.enabled = true
                        // 跳转到聊天页面
                        stackView.push("qrc:/qml/pages/ChatPage.qml")
                    }
                }
                
                // 鼠标区域，实现按钮点击效果
                MouseArea {
                    id: submitMouseArea // 鼠标区域ID
                    anchors.fill: parent // 填充容器
                    cursorShape: Qt.PointingHandCursor // 鼠标悬停时显示手型光标
                    hoverEnabled: true // 启用悬停检测
                    // 鼠标进入时改变颜色
                    onEntered: parent.color = "#3730a3" // 深蓝色
                    onExited: parent.color = "#4f46e5" // 恢复靛蓝色
                    // 鼠标按下时改变颜色
                    onPressed: parent.color = '#201d63' // 更深的蓝色
                    onReleased: parent.color = "#3730a3" // 恢复深蓝色
                    
                    // 点击事件
                    onClicked: {
                        print("用户登录：" + username.text + " ： " + password.text) // 打印登录信息
                        // 显示加载状态
                        loginLoading.visible = true
                        loginLoading.running = true
                        submitMouseArea.enabled = false
                        
                        // 启动登录定时器
                        loginTimer.running = true
                    }
                }
                
                // 按钮文本
                Text {
                    id: loginButtonText
                    text: "登录" // 按钮文本
                    anchors.centerIn: parent // 在容器中居中
                    font.pointSize: Math.max(Math.min(parent.width * 0.045, 18), 12) // 字体大小，最大18点，最小12点
                    font.bold: true // 粗体显示
                    color: "white" // 白色文字
                    visible: !loginLoading.visible
                }
                
                // 颜色变化动画
                Behavior on color {
                    ColorAnimation { duration: 200 } // 颜色动画持续时间200毫秒
                }
            }
            
            // 底部提示文本
            Item {
                id: registerContainer
                width: parent.width * 0.8
                height: parent.height * 0.05
                anchors.horizontalCenter: parent.horizontalCenter // 水平居中
                anchors.bottom: parent.bottom // 锚定在表单底部
                anchors.bottomMargin: parent.height * 0.05 // 底部边距，使用相对值
                
                // 注册定时器
                Timer {
                    id: registerTimer
                    interval: 500
                    running: false
                    repeat: false
                    onTriggered: {
                        // 隐藏加载状态
                        registerLoading.visible = false
                        registerLoading.running = false
                        // 跳转到注册页面
                        stackView.push("qrc:/qml/pages/RegisterPage.qml")
                    }
                }
                
                // 注册链接文本
                Text {
                    id: registerText
                    text: "忘记密码? | 注册账号" // 提示文本内容
                    anchors.centerIn: parent // 水平居中
                    font.pointSize: Math.max(Math.min(parent.width * 0.035, 14), 10) // 字体大小，最大14点，最小10点
                    color: "#6b7280" // 中灰色文字颜色
                    visible: !registerLoading.visible
                    
                    // 鼠标悬停效果
                    MouseArea {
                        anchors.fill: parent // 填充整个文本区域
                        cursorShape: Qt.PointingHandCursor // 鼠标悬停时显示手型光标
                        hoverEnabled: true
                        onEntered: registerText.color = "#4f46e5" // 鼠标悬停时改变颜色
                        onExited: registerText.color = "#6b7280" // 恢复颜色
                        onClicked: {
                            print("点击了底部注册链接") // 打印点击事件
                            // 显示加载状态
                            registerLoading.visible = true
                            registerLoading.running = true
                            
                            // 启动注册定时器
                            registerTimer.running = true
                        }
                    }
                }
                
                // 加载状态指示器
                BusyIndicator {
                    id: registerLoading
                    anchors.centerIn: parent
                    running: false
                    visible: false
                    width: 16
                    height: 16
                }
            }
        }
    }
}

