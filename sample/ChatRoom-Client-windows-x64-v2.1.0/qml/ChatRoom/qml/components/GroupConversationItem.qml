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
    color: selected ? "#e6f7ff" : (mouseArea.containsMouse ? "#f5f5f5" : "transparent")

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

                Text {
                    text: conversationItem.groupName || "未知群组"
                    font.pixelSize: 14
                    font.bold: true
                    color: "#333"
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                // Default group badge
                Rectangle {
                    visible: conversationItem.isDefault
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 16
                    radius: 3
                    color: "#1890ff"
                    Layout.alignment: Qt.AlignVCenter

                    Text {
                        text: "默认"
                        font.pixelSize: 9
                        color: "white"
                        anchors.centerIn: parent
                    }
                }

                Text {
                    text: {
                        var d = new Date(conversationItem.lastTime)
                        var now = new Date()
                        if (d.toDateString() === now.toDateString()) {
                            return Qt.formatDateTime(d, "HH:mm")
                        }
                        return Qt.formatDateTime(d, "MM/dd")
                    }
                    font.pixelSize: 11
                    color: "#999"
                    Layout.alignment: Qt.AlignTop
                }
            }

            // Message preview row
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: conversationItem.lastMessage
                    font.pixelSize: 12
                    color: "#999"
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
                    color: "#ff4d4f"
                    Layout.alignment: Qt.AlignVCenter

                    Text {
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
        color: "#f0f0f0"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: conversationItem.groupClicked(conversationItem.groupId)
    }
}
