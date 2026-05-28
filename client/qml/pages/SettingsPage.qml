import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import ChatRoom

Item {
    id: settingsPage

    signal logoutRequested()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: "#1890ff"

            StyledText {
                text: "Settings"
                font.pixelSize: 16
                font.bold: true
                color: "white"
                anchors.centerIn: parent
            }
        }

        // Settings content
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 16
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            ColumnLayout {
                width: parent.width
                spacing: 16

            // User info section
            GroupBox {
                Layout.fillWidth: true
                title: "Account"

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    // Avatar with upload
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Avatar {
                            id: settingsAvatar
                            Layout.preferredWidth: 64
                            Layout.preferredHeight: 64
                            imageUrl: appCore.sessionManager.avatar
                            name: appCore.sessionManager.nickname

                            MouseArea {
                                anchors.fill: parent
                                onClicked: avatarFileDialog.open()
                            }
                        }

                        StyledText {
                            text: "Click avatar to change"
                            font.pixelSize: 12
                            color: "#999"
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        StyledText { text: "Username:"; font.bold: true; color: "#666" }
                        StyledText { text: appCore.sessionManager.username; color: "#333" }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        StyledText { text: "Nickname:"; font.bold: true; color: "#666" }
                        StyledText { text: appCore.sessionManager.nickname; color: "#333" }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        StyledText { text: "User ID:"; font.bold: true; color: "#666" }
                        StyledText { text: appCore.sessionManager.userId; color: "#333" }
                    }
                }
            }

        // --- Theme Section ---
        GroupBox {
            Layout.fillWidth: true
            title: "Theme"

            ColumnLayout {
                width: parent.width
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        model: appCore.themeManager.themeNames()

                        Rectangle {
                            Layout.preferredWidth: 56
                            Layout.preferredHeight: 72
                            radius: 8
                            border.width: index === appCore.themeManager.themeIndex ? 3 : 1
                            border.color: index === appCore.themeManager.themeIndex
                                         ? appCore.themeManager.primary : appCore.themeManager.borderColor

                            color: [
                                "#ff6b81", "#2d8a4e", "#6c5ce7",
                                "#9acd32", "#00b894"
                            ][index]

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 4

                                Item {
                                    Layout.preferredWidth: 24
                                    Layout.preferredHeight: 24
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 20
                                        height: 20
                                        radius: 10
                                        color: "white"
                                        visible: index === appCore.themeManager.themeIndex
                                    }
                                    StyledText {
                                        anchors.centerIn: parent
                                        text: "\u2713"
                                        font.pixelSize: 12
                                        font.bold: true
                                        color: [
                                            "#ff6b81", "#2d8a4e", "#6c5ce7",
                                            "#9acd32", "#00b894"
                                        ][index]
                                        visible: index === appCore.themeManager.themeIndex
                                    }
                                }

                                StyledText {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: modelData.replace(/[^\u4e00-\u9fff]/g, "")
                                    font.pixelSize: 10
                                    color: "white"
                                    style: Text.Outline
                                    styleColor: "#000000"
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: appCore.themeManager.themeIndex = index
                            }
                        }
                    }
                }
            }
        }

        // --- Font Section ---
        GroupBox {
            Layout.fillWidth: true
            title: "Font"

            ColumnLayout {
                width: parent.width
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        model: appCore.themeManager.fontNames()

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 64
                            radius: 8
                            border.width: index === appCore.themeManager.fontIndex ? 3 : 1
                            border.color: index === appCore.themeManager.fontIndex
                                         ? appCore.themeManager.primary : appCore.themeManager.borderColor
                            color: appCore.themeManager.surface

                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 4

                                StyledText {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: modelData
                                    font.pixelSize: 11
                                    font.bold: index === appCore.themeManager.fontIndex
                                    color: appCore.themeManager.textPrimary
                                }

                                StyledText {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "Hello "
                                    font.pixelSize: 13
                                    color: appCore.themeManager.textSecondary
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: appCore.themeManager.fontIndex = index
                            }
                        }
                    }
                }
            }
        }

        // Chat background
        GroupBox {
            Layout.fillWidth: true
            title: "\u804A\u5929\u80CC\u666F"
            ColumnLayout {
                width: parent.width; spacing: 8
                GridLayout {
                    columns: 4; Layout.fillWidth: true; columnSpacing: 6; rowSpacing: 6
                    Repeater {
                        model: ["#ffffff","#f5f5f5","#e8f4fd","#f0faf0","#fff7e6","#fce4ec","#e8eaf6","#e0f2f1"]
                        Rectangle {
                            Layout.preferredWidth: 36; Layout.preferredHeight: 36; radius: 6
                            color: modelData
                            border.width: appCore.chatBackground === modelData ? 2 : 0
                            border.color: appCore.themeManager.primary
                            MouseArea { anchors.fill: parent; onClicked: appCore.setChatBackground(modelData) }
                        }
                    }
                }
                RowLayout {
                    Layout.fillWidth: true; spacing: 8
                    Button {
                        text: "\u81EA\u5B9A\u4E49\u56FE\u7247"; Layout.fillWidth: true
                        onClicked: bgFileDialog.open()
                    }
                    Button {
                        text: "\u91CD\u7F6E"; flat: true
                        onClicked: appCore.clearChatBackground()
                    }
                }
            }
        }

        // Clear history
        Button {
            Layout.fillWidth: true
            text: "Clear All Chat History"
            flat: true

            onClicked: {
                confirmClearDialog.open()
            }
        }

        Item { Layout.fillHeight: true }

        // Logout button
            Button {
                Layout.fillWidth: true
                text: "Logout"
                font.pixelSize: 14

                background: Rectangle {
                    radius: 6
                    color: "#ff4d4f"
                    implicitHeight: 40
                }
                contentItem: StyledText {
                    text: parent.text
                    color: "white"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: logoutRequested()
            }
            }
        }
    }

    ConfirmDialog {
        id: confirmClearDialog
        title: "Clear History"
        message: "Are you sure you want to clear all chat history? This cannot be undone."
        onAccepted: {
            // Clear all messages from database and models
            appCore.chatController.clearAllHistory()
            appCore.conversationModel.clear()
            appCore.showToast("\u804A\u5929\u8BB0\u5F55\u5DF2\u6E05\u9664")
        }
    }

    // Avatar file dialog
    FileDialog {
        id: avatarFileDialog
        title: "\u9009\u62E9\u5934\u50CF"
        nameFilters: ["Image files (*.png *.jpg *.jpeg)"]
        onAccepted: {
            appCore.chatController.uploadAvatar(selectedFile)
        }
    }

    FileDialog {
        id: bgFileDialog; title: "\u9009\u62E9\u80CC\u666F\u56FE\u7247"
        nameFilters: ["Images (*.png *.jpg *.jpeg)"]
        onAccepted: appCore.setChatBackground(selectedFile.toString())
    }
}
