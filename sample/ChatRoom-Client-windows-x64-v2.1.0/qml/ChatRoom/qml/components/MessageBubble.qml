import QtQuick
import QtQuick.Controls
import ChatRoom

Item {
    id: bubble

    property string content: ""
    property bool isMine: false
    property string senderName: ""
    property var timestamp: new Date()
    property int type: 0  // 0=text, 1=image, 2=file
    property int status: 1  // 0=Sending, 1=Sent, 2=Delivered, 3=Read, 4=Failed
    property var messageId: 0
    property bool showRecallMenu: false

    width: ListView.view ? ListView.view.width : 400
    height: bubbleRow.height + 4

    MouseArea {
        id: bubbleMouse
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: {
            if (bubble.isMine) {
                bubble.showRecallMenu = true
            }
        }
    }

    // Recall popup
    Popup {
        id: recallPopup
        anchors.centerIn: parent
        width: 120
        height: 36
        visible: bubble.showRecallMenu
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "white"
            border.color: "#e0e0e0"
            radius: 4
        }

        contentItem: Text {
            text: "撤回消息"
            font.pixelSize: 13
            color: "#333"
            anchors.centerIn: parent
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                appCore.chatController.recallMessage(bubble.messageId)
                recallPopup.close()
            }
        }

        onClosed: bubble.showRecallMenu = false
    }

    // Invisible text for measuring wrapped width
    Text {
        id: bubbleTextWidth
        text: bubble.content
        font.pixelSize: 14
        visible: false
        wrapMode: Text.WrapAnywhere
        width: 360
    }

    Row {
        id: bubbleRow
        spacing: 8
        leftPadding: 8
        rightPadding: 8
        anchors.left: parent.left
        anchors.right: parent.right

        // For own messages: show spacer on left to push content to right
        // For others: no spacer, content stays on left
        Item {
            width: bubble.isMine ? (bubbleRow.width - bubbleCol.implicitWidth - 36 - 16) : 0
            height: 1
        }

        // Avatar
        Avatar {
            width: 36
            height: 36
            name: bubble.senderName
            anchors.verticalCenter: parent.verticalCenter
        }

        // Bubble + timestamp column
        Column {
            id: bubbleCol
            spacing: 2

            // Message bubble
            Rectangle {
                width: bubbleRect.implicitWidth + 20
                height: bubbleRect.implicitHeight + 14
                radius: 8
                color: isMine ? "#1890ff" : "white"
                border.width: isMine ? 0 : 1
                border.color: "#e0e0e0"

                Text {
                    id: bubbleRect
                    anchors.centerIn: parent
                    width: Math.min(360, bubbleTextWidth.implicitWidth)
                    text: bubble.content
                    font.pixelSize: 14
                    color: isMine ? "white" : "#333"
                    wrapMode: Text.WrapAnywhere
                    horizontalAlignment: Text.AlignLeft
                }
            }

            // Status + Timestamp row
            Row {
                spacing: 4

                Text {
                    text: {
                        if (bubble.status === 0) return "\u23F3"  // hourglass = Sending
                        if (bubble.status === 4) return "\u2717"  // cross = Failed
                        if (bubble.status === 3) return "\u2713\u2713"  // double blue check = Read
                        if (bubble.status === 2) return "\u2713\u2713"  // double gray check = Delivered
                        return "\u2713"  // single check = Sent
                    }
                    font.pixelSize: 10
                    color: {
                        if (bubble.status === 4) return "#ff4d4f"  // red for failed
                        if (bubble.status === 3) return "#1890ff"  // blue for read
                        return "#999"  // gray for others
                    }
                    visible: bubble.isMine
                }

                Text {
                    text: {
                        var d = new Date(bubble.timestamp)
                        return Qt.formatDateTime(d, "HH:mm")
                    }
                    font.pixelSize: 10
                    color: "#999"
                }
            }
        }
    }
}
