import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import ChatRoom

Rectangle {
    id: conversationItem

    property var groupId: 0
    property string groupName: ""
    property string groupAvatar: ""
    property string lastMessage: ""
    property var lastTime: new Date()
    property int unreadCount: 0
    property int memberCount: 0
    property bool isDefault: false
    property bool selected: false

    signal groupClicked(var groupId)

    width: parent ? parent.width : 280
    height: 64
    color: selected ? appCore.themeManager.tabHoverBg : (mouseArea.containsMouse ? appCore.themeManager.surfaceAlt : "transparent")

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 10

        Avatar {
            id: avatarComp
            Layout.preferredWidth: 44
            Layout.preferredHeight: 44
            Layout.alignment: Qt.AlignVCenter
            name: conversationItem.groupName
            imageUrl: conversationItem.groupAvatar
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 4

            // Name row
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                StyledText {
                    text: conversationItem.groupName || "未知群组"
                    font.pixelSize: 14
                    font.bold: true
                    color: appCore.themeManager.textPrimary
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                // Default group badge
                Rectangle {
                    visible: conversationItem.isDefault
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 16
                    radius: 3
                    color: appCore.themeManager.primary
                    Layout.alignment: Qt.AlignVCenter

                    StyledText {
                        text: "默认"
                        font.pixelSize: 9
                        color: "white"
                        anchors.centerIn: parent
                    }
                }

                StyledText {
                    text: {
                        var d = new Date(conversationItem.lastTime)
                        var now = new Date()
                        if (d.toDateString() === now.toDateString()) {
                            return Qt.formatDateTime(d, "HH:mm")
                        }
                        return Qt.formatDateTime(d, "MM/dd")
                    }
                    font.pixelSize: 11
                    color: appCore.themeManager.textTertiary
                    Layout.alignment: Qt.AlignTop
                }
            }

            // Message preview row
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                StyledText {
                    text: conversationItem.lastMessage
                    font.pixelSize: 12
                    color: appCore.themeManager.textTertiary
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                // Unread badge
                Rectangle {
                    id: badge
                    visible: conversationItem.unreadCount > 0
                    Layout.preferredWidth: {
                        var c = conversationItem.unreadCount
                        return c > 99 ? 36 : (c > 9 ? 28 : 20)
                    }
                    Layout.preferredHeight: 18
                    radius: 9
                    color: appCore.themeManager.danger
                    Layout.alignment: Qt.AlignVCenter

                    StyledText {
                        text: {
                            var c = conversationItem.unreadCount
                            return c > 99 ? "99+" : c
                        }
                        font.pixelSize: 10
                        color: "white"
                        anchors.centerIn: parent
                    }
                }
            }
        }
    }

    // Bottom divider
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 66
        height: 1
        color: appCore.themeManager.divider
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: conversationItem.groupClicked(conversationItem.groupId)
    }
}
