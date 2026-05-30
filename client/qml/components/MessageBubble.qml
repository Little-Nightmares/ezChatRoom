import QtQuick
import QtQuick.Controls
import ChatRoom

Item {
    id: bubble

    property string content: ""
    property bool isMine: false
    property string senderName: ""
    property var timestamp: new Date()
    property int type: 0
    property int status: 1
    property var messageId: 0
    property bool showRecallMenu: false

    width: ListView.view ? ListView.view.width : 400
    height: bubbleRow.y + bubbleRow.height + 4

    MouseArea {
        id: bubbleMouse; anchors.fill: parent; acceptedButtons: Qt.RightButton
        onClicked: { if (bubble.isMine) bubble.showRecallMenu = true }
    }

    Popup {
        id: recallPopup; anchors.centerIn: parent; width: 120; height: 36
        visible: bubble.showRecallMenu
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle { color: "white"; border.color: "#e0e0e0"; radius: 4 }
        contentItem: StyledText { text: "\u64A4\u56DE\u6D88\u606F"; font.pixelSize: 13; color: "#333"; anchors.centerIn: parent }
        MouseArea { anchors.fill: parent; onClicked: { appCore.chatController.recallMessage(bubble.messageId); recallPopup.close() } }
        onClosed: bubble.showRecallMenu = false
    }

    Row {
        id: bubbleRow; spacing: 0; leftPadding: 8; rightPadding: 8
        anchors.left: parent.left; anchors.right: parent.right; y: 4

        Item { width: isMine ? (bubbleRow.width - 16 - bubbleCol.width - 36 - 8) : 0; height: 1 }

        Row {
            spacing: 8; layoutDirection: isMine ? Qt.RightToLeft : Qt.LeftToRight
            Avatar { width: 36; height: 36; name: bubble.senderName }

            Column {
                id: bubbleCol; spacing: 2

                Rectangle {
                    id: bubbleRect; radius: 8
                    color: isMine ? appCore.themeManager.bubbleMine : appCore.themeManager.bubbleOther
                    border.width: isMine ? 0 : 1; border.color: appCore.themeManager.borderColor
                    width: Math.min(320, Math.max(bubbleText.implicitWidth + 12, 60))
                    height: bubbleText.implicitHeight + 12

                    TextEdit {
                        id: bubbleText
                        anchors.centerIn: parent
                        width: parent.width - 12
                        font.pixelSize: 14
                        font.family: appCore.themeManager.fontFamily
                        color: isMine ? appCore.themeManager.bubbleMineText : appCore.themeManager.bubbleOtherText
                        wrapMode: Text.WrapAnywhere
                        readOnly: true
                        selectByMouse: false
                        textFormat: TextEdit.RichText

                        function highlightMentions(plainText) {
                            var html = plainText.replace(/@([\w\u4e00-\u9fff\-]+)/g,
                                '<span style="color: #1890ff; font-weight: bold;">@$1</span>')
                            html = html.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
                            html = html.replace(/&lt;span style=&quot;color: #1890ff; font-weight: bold;&quot;&gt;@/g,
                                '<span style="color: #1890ff; font-weight: bold;">@')
                            html = html.replace(/&lt;\/span&gt;/g, '</span>')
                            return html
                        }

                        text: highlightMentions(bubble.content)
                    }
                }

                Row {
                    spacing: 4; layoutDirection: isMine ? Qt.RightToLeft : Qt.LeftToRight
                    StyledText {
                        text: { if (bubble.status === 0) return "\u23F3"; if (bubble.status === 4) return "\u2717"; if (bubble.status === 3) return "\u2713\u2713"; if (bubble.status === 2) return "\u2713\u2713"; return "\u2713" }
                        font.pixelSize: 10
                        color: { if (bubble.status === 4) return "#ff4d4f"; if (bubble.status === 3) return "#1890ff"; return appCore.themeManager.textTertiary }
                        visible: bubble.isMine
                    }
                    StyledText { text: Qt.formatDateTime(new Date(bubble.timestamp), "HH:mm"); font.pixelSize: 10; color: appCore.themeManager.textTertiary }
                }
            }
        }
    }
}
