import QtQuick
import QtQuick.Layouts
import ChatRoom

Rectangle {
    id: conversationItem

    property var friendId: 0
    property string nickname: ""
    property string avatarUrl: ""
    property alias lastMessage: lastMsgText.text
    property var lastTime: new Date()
    property alias unreadCount: badge.count
    property bool selected: false

    signal clicked()

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
            name: conversationItem.nickname
            imageUrl: conversationItem.avatarUrl
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
                    text: conversationItem.nickname || "Unknown"
                    font.pixelSize: 14
                    font.bold: true
                    color: "#333"
                    elide: Text.ElideRight
                    Layout.fillWidth: true
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
                    id: lastMsgText
                    text: ""
                    font.pixelSize: 12
                    color: "#999"
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                // Unread badge
                Rectangle {
                    id: badge
                    property int count: 0
                    visible: count > 0
                    Layout.preferredWidth: count > 99 ? 36 : (count > 9 ? 28 : 20)
                    Layout.preferredHeight: 18
                    radius: 9
                    color: "#ff4d4f"
                    Layout.alignment: Qt.AlignVCenter

                    Text {
                        text: badge.count > 99 ? "99+" : badge.count
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
        onClicked: conversationItem.clicked()
    }
}
