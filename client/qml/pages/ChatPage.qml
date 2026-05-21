import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Page {
    id: root
    title: qsTr("Chat")

    property var conversations: [
        { title: "Alice", lastMessage: "See you soon", lastTime: "09:30", unreadCount: 2 },
        { title: "Team", lastMessage: "Build passed", lastTime: "10:12", unreadCount: 0 }
    ]
    property var messages: [
        { senderName: "Alice", content: "Hello", mine: false, timeText: "09:29" },
        { senderName: "Me", content: "Hi, I am online.", mine: true, timeText: "09:30" }
    ]

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            color: "#f8fafc"
            border.color: "#e2e8f0"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                SearchBar {
                    Layout.fillWidth: true
                    placeholderText: qsTr("Search conversations")
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: root.conversations
                    delegate: ConversationItem {
                        width: ListView.view.width
                        title: modelData.title
                        lastMessage: modelData.lastMessage
                        lastTime: modelData.lastTime
                        unreadCount: modelData.unreadCount
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            ToolBar {
                Layout.fillWidth: true
                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    text: qsTr("Alice")
                    font.bold: true
                }
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: root.messages
                delegate: MessageBubble {
                    width: ListView.view.width
                    senderName: modelData.senderName
                    content: modelData.content
                    mine: modelData.mine
                    timeText: modelData.timeText
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#e2e8f0"
            }

            InputBar {
                Layout.fillWidth: true
                onSendRequested: function(text) {
                    root.messages = root.messages.concat([{
                        senderName: "Me",
                        content: text,
                        mine: true,
                        timeText: Qt.formatTime(new Date(), "HH:mm")
                    }])
                }
            }
        }
    }
}
