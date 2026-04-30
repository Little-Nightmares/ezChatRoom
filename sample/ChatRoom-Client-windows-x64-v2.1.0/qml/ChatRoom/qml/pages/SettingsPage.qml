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

            Text {
                text: "Settings"
                font.pixelSize: 16
                font.bold: true
                color: "white"
                anchors.centerIn: parent
            }
        }

        // Settings content
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 16
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

                        Text {
                            text: "Click avatar to change"
                            font.pixelSize: 12
                            color: "#999"
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Username:"; font.bold: true; color: "#666" }
                        Text { text: appCore.sessionManager.username; color: "#333" }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Nickname:"; font.bold: true; color: "#666" }
                        Text { text: appCore.sessionManager.nickname; color: "#333" }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "User ID:"; font.bold: true; color: "#666" }
                        Text { text: appCore.sessionManager.userId; color: "#333" }
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
                contentItem: Text {
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

    ConfirmDialog {
        id: confirmClearDialog
        title: "Clear History"
        message: "Are you sure you want to clear all chat history? This cannot be undone."
        onAccepted: {
            // Clear all messages from database and models
            appCore.chatController.clearAllHistory()
            appCore.conversationModel.clear()
            appCore.showToast("聊天记录已清除")
        }
    }

    // Avatar file dialog
    FileDialog {
        id: avatarFileDialog
        title: "选择头像"
        nameFilters: ["Image files (*.png *.jpg *.jpeg)"]
        onAccepted: {
            appCore.chatController.uploadAvatar(selectedFile)
        }
    }
}
