import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// 个人详情弹窗组件
// 功能：显示用户的详细个人信息，类似个人博客页面
Popup {
    id: profilePopup  // 弹窗唯一标识符

    // 弹窗尺寸：占父容器的80%宽度，最大500px
    width: Math.min(parent.width * 0.8, 500)
    height: Math.min(parent.height * 0.8, 600)

    // 弹窗居中显示
    anchors.centerIn: parent

    // 使用主题背景色
    background: Rectangle {
        color: themeManager.currentTheme.surface
        radius: 12  // 圆角12像素
        border.color: themeManager.currentTheme.border
        border.width: 1
    }

    // 弹窗内容
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20  // 内边距20像素
        spacing: 15  // 元素间距15像素

        // ==================== 关闭按钮 ====================
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 30  // 关闭按钮高度30像素

            // 关闭按钮
            Button {
                id: closeButton  // 关闭按钮
                anchors.right: parent.right  // 靠右对齐
                width: 30  // 宽度30像素
                height: 30  // 高度30像素
                text: "✕"  // 关闭符号

                background: Rectangle {
                    color: "transparent"  // 透明背景
                    radius: 15  // 圆角15像素
                }

                contentItem: Text {
                    text: parent.text
                    color: themeManager.currentTheme.text
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    profilePopup.close()  // 关闭弹窗
                }
            }
        }

        // ==================== 头部区域 ====================
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 150  // 头部区域高度150像素
            color: themeManager.currentTheme.primary
            radius: 8  // 圆角8像素

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                // 头像
                Rectangle {
                    id: popupAvatar  // 弹窗头像
                    Layout.alignment: Qt.AlignHCenter  // 水平居中
                    width: 80  // 头像宽度80像素
                    height: 80  // 头像高度80像素
                    radius: 40  // 圆角40像素，变成圆形
                    color: themeManager.currentTheme.surface  // 使用表面色作为背景

                    // 头像中的用户图标或首字母
                    Text {
                        anchors.centerIn: parent
                        text: UserController.loggedInUser !== "" ? UserController.loggedInUser.charAt(0).toUpperCase() : "U"
                        color: themeManager.currentTheme.primary
                        font.pixelSize: 36
                        font.bold: true
                    }
                }

                // 用户名
                Text {
                    Layout.alignment: Qt.AlignHCenter  // 水平居中
                    text: UserController.loggedInUser !== "" ? UserController.loggedInUser : "未登录用户"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                }

                // 在线状态
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter  // 水平居中
                    spacing: 5  // 元素间距5像素

                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: AppManager.isConnected ? "#4CAF50" : "#9E9E9E"  // 在线：绿色，离线：灰色
                    }

                    Text {
                        text: AppManager.isConnected ? "在线" : "离线"
                        color: "white"
                        font.pixelSize: 12
                        opacity: 0.9
                    }
                }
            }
        }

        // ==================== 分隔线 ====================
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: themeManager.currentTheme.border
        }

        // ==================== 详细信息区域 ====================
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true  // 填充剩余空间

            ColumnLayout {
                width: parent.width  // 宽度与父容器相同
                spacing: 12  // 元素间距12像素

                // 信息项模板
                function createInfoItem(title, content, icon) {
                    return infoColumn.children
                }

                // ==================== 个人简介 ====================
                Rectangle {
                    Layout.fillWidth: true
                    color: themeManager.currentTheme.background
                    radius: 8
                    padding: 12

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        RowLayout {
                            spacing: 8
                            Text {
                                text: "📝"  // 笔记图标
                                font.pixelSize: 16
                            }
                            Text {
                                text: "个人简介"
                                color: themeManager.currentTheme.text
                                font.pixelSize: 14
                                font.bold: true
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "这个人很懒，什么都没有留下..."
                            color: themeManager.currentTheme.text
                            font.pixelSize: 13
                            opacity: 0.7
                            wrapMode: Text.Wrap
                        }
                    }
                }

                // ==================== 统计信息 ====================
                Rectangle {
                    Layout.fillWidth: true
                    color: themeManager.currentTheme.background
                    radius: 8
                    padding: 12

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        RowLayout {
                            spacing: 8
                            Text {
                                text: "📊"  // 统计图标
                                font.pixelSize: 16
                            }
                            Text {
                                text: "数据统计"
                                color: themeManager.currentTheme.text
                                font.pixelSize: 14
                                font.bold: true
                            }
                        }

                        // 统计数据行
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 20

                            // 好友数量
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "0"  // TODO: 后续从FriendController获取
                                    color: themeManager.currentTheme.primary
                                    font.pixelSize: 24
                                    font.bold: true
                                }

                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "好友"
                                    color: themeManager.currentTheme.text
                                    font.pixelSize: 12
                                    opacity: 0.7
                                }
                            }

                            // 群组数量
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "0"  // TODO: 后续从GroupController获取
                                    color: themeManager.currentTheme.primary
                                    font.pixelSize: 24
                                    font.bold: true
                                }

                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "群组"
                                    color: themeManager.currentTheme.text
                                    font.pixelSize: 12
                                    opacity: 0.7
                                }
                            }

                            // 消息数量
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4

                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "0"  // TODO: 后续从ChatController获取
                                    color: themeManager.currentTheme.primary
                                    font.pixelSize: 24
                                    font.bold: true
                                }

                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "消息"
                                    color: themeManager.currentTheme.text
                                    font.pixelSize: 12
                                    opacity: 0.7
                                }
                            }
                        }
                    }
                }

                // ==================== 联系方式 ====================
                Rectangle {
                    Layout.fillWidth: true
                    color: themeManager.currentTheme.background
                    radius: 8
                    padding: 12

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        RowLayout {
                            spacing: 8
                            Text {
                                text: "📧"  // 邮件图标
                                font.pixelSize: 16
                            }
                            Text {
                                text: "联系方式"
                                color: themeManager.currentTheme.text
                                font.pixelSize: 14
                                font.bold: true
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "email@example.com"  // TODO: 后续从UserController获取
                            color: themeManager.currentTheme.text
                            font.pixelSize: 13
                            opacity: 0.7
                        }
                    }
                }

                // ==================== 注册时间 ====================
                Rectangle {
                    Layout.fillWidth: true
                    color: themeManager.currentTheme.background
                    radius: 8
                    padding: 12

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        RowLayout {
                            spacing: 8
                            Text {
                                text: "📅"  // 日历图标
                                font.pixelSize: 16
                            }
                            Text {
                                text: "注册时间"
                                color: themeManager.currentTheme.text
                                font.pixelSize: 14
                                font.bold: true
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            text: "2024-01-01"  // TODO: 后续从UserController获取
                            color: themeManager.currentTheme.text
                            font.pixelSize: 13
                            opacity: 0.7
                        }
                    }
                }

                // ==================== 个人主页按钮 ====================
                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 45  // 高度45像素
                    text: "编辑个人资料"

                    background: Rectangle {
                        color: themeManager.currentTheme.primary
                        radius: 8
                    }

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        console.log("编辑个人资料")
                        // TODO: 后续打开编辑资料页面
                    }
                }
            }
        }
    }

    // 关闭弹窗的方式
    modal: true  // 模态弹窗
    dim: true  // 背景变暗
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside  // 按ESC或点击外部关闭
}