import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root
    title: qsTr("设置")

    // 背景颜色
    background: Rectangle {
        color: themeManager.currentTheme.background
    }

    // 顶部工具栏
    header: ToolBar {
        background: Rectangle {
            color: themeManager.currentTheme.surface
        }
        
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: qsTr("← 返回")
                font.pointSize: Math.max(Math.min(root.width * 0.025, 12), 10) // 使用相对字体大小
                background: Rectangle {
                    color: "transparent" // 透明背景
                }
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: themeManager.currentTheme.text
                }
                onClicked: stackView.pop()  // 返回上一页
            }
            Text {
                text: qsTr("设置")
                font.pointSize: Math.max(Math.min(root.width * 0.035, 18), 12) // 使用相对字体大小
                font.bold: true
                color: themeManager.currentTheme.text
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
            Item { width: Math.max(root.width * 0.05, 60) } // 占位，保持标题居中，使用相对宽度
        }
    }

    // 设置内容区域
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: parent.width * 0.05 // 使用相对边距
        spacing: parent.height * 0.02 // 使用相对间距

        // 应用配置区域
        GroupBox {
            title: qsTr("应用配置")
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height * 0.25 // 使用相对高度
            background: Rectangle {
                color: themeManager.currentTheme.surface
                border.color: themeManager.currentTheme.border
                radius: Math.max(parent.width * 0.02, 8) // 使用相对圆角
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: parent.width * 0.02 // 使用相对边距
                spacing: parent.height * 0.015 // 使用相对间距

                // 通知设置
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("启用通知")
                        Layout.fillWidth: true
                        color: themeManager.currentTheme.text
                        font.pointSize: Math.max(Math.min(parent.width * 0.03, 12), 10) // 使用相对字体大小
                    }
                    Switch {
                        id: notificationSwitch
                        // 从AppManager获取通知状态
                        checked: AppManager.notificationsEnabled
                        onCheckedChanged: {
                            console.log("通知设置: " + checked)
                            // 更新AppManager的通知状态
                            AppManager.notificationsEnabled = checked
                        }
                    }
                }

                // 声音设置
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("消息声音")
                        Layout.fillWidth: true
                        color: themeManager.currentTheme.text
                        font.pointSize: Math.max(Math.min(parent.width * 0.03, 12), 10) // 使用相对字体大小
                    }
                    Switch {
                        id: soundSwitch
                        checked: true
                        onCheckedChanged: {
                            console.log("声音设置: " + checked)
                        }
                    }
                }
            }
        }

        // 主题切换区域
        GroupBox {
            title: qsTr("主题设置")
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height * 0.2 // 使用相对高度
            background: Rectangle {
                color: themeManager.currentTheme.surface
                border.color: themeManager.currentTheme.border
                radius: Math.max(parent.width * 0.02, 8) // 使用相对圆角
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: parent.width * 0.02 // 使用相对边距
                spacing: parent.height * 0.015 // 使用相对间距

                // 主题选择
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("主题模式")
                        Layout.fillWidth: true
                        color: themeManager.currentTheme.text
                        font.pointSize: Math.max(Math.min(parent.width * 0.03, 12), 10) // 使用相对字体大小
                    }
                    ComboBox {
                        id: themeComboBox
                        model: [qsTr("浅色主题"), qsTr("深色主题"), qsTr("樱花主题"), qsTr("海洋主题"), qsTr("森林主题"), qsTr("跟随系统")]
                        currentIndex: themeManager.currentThemeIndex // 显示当前主题
                        onCurrentIndexChanged: {
                            console.log("主题选择: " + currentIndex)
                            // 应用主题
                            themeManager.applyTheme(currentIndex)
                        }
                        width: 150
                        height: 40
                    }
                }
            }
        }

        // 账号管理区域
        GroupBox {
            title: qsTr("账号管理")
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height * 0.3 // 使用相对高度
            background: Rectangle {
                color: themeManager.currentTheme.surface
                border.color: themeManager.currentTheme.border
                radius: Math.max(parent.width * 0.02, 8) // 使用相对圆角
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: parent.width * 0.02 // 使用相对边距
                spacing: parent.height * 0.015 // 使用相对间距

                // 用户信息
                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: qsTr("当前用户:")
                        color: themeManager.currentTheme.text
                        font.pointSize: Math.max(Math.min(parent.width * 0.03, 12), 10) // 使用相对字体大小
                    }
                    Text {
                        text: qsTr("用户名")
                        font.bold: true
                        color: themeManager.currentTheme.primary
                        font.pointSize: Math.max(Math.min(parent.width * 0.03, 12), 10) // 使用相对字体大小
                    }
                }

                // 修改密码按钮
                Button {
                    text: qsTr("修改密码")
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.2 // 使用相对高度
                    font.pointSize: Math.max(Math.min(parent.width * 0.03, 12), 10) // 使用相对字体大小
                    background: Rectangle {
                        color: themeManager.currentTheme.button
                        radius: Math.max(parent.width * 0.015, 6)
                    }
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: themeManager.currentTheme.buttonText
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        console.log("打开修改密码对话框")
                    }
                }

                // 退出登录按钮
                Button {
                    text: qsTr("退出登录")
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.2 // 使用相对高度
                    font.pointSize: Math.max(Math.min(parent.width * 0.03, 12), 10) // 使用相对字体大小
                    background: Rectangle {
                        color: themeManager.currentTheme.button
                        radius: Math.max(parent.width * 0.015, 6)
                    }
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: themeManager.currentTheme.buttonText
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        console.log("退出登录")
                        // 返回到登录页面
                        stackView.pop(null)
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true // 占位，将内容推到顶部
        }
    }
}
