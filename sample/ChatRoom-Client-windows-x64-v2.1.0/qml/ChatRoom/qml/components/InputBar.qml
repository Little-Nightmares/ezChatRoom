import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: inputBar

    signal sendMessage(string text)

    // Fixed height like QQ
    property int barHeight: 56
    height: barHeight
    color: "white"

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        spacing: 8

        // Input area with border - fixed size, scrollable inside
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: "#f5f5f5"
            border.color: "#d9d9d9"
            border.width: 1
            clip: true

            Flickable {
                id: flickable
                anchors.fill: parent
                anchors.margins: 8
                contentWidth: width
                contentHeight: editArea.implicitHeight
                boundsBehavior: Flickable.StopAtBounds

                TextEdit {
                    id: editArea
                    width: flickable.width
                    wrapMode: TextEdit.Wrap
                    font.pixelSize: 14
                    color: "#333"
                    selectByMouse: true
                    textFormat: TextEdit.PlainText

                    PlaceholderText {
                        text: "输入消息..."
                        color: "#aaa"
                        font.pixelSize: 14
                        visible: editArea.text.length === 0 && !editArea.activeFocus
                    }

                    Keys.onReturnPressed: function(event) {
                        if (!(event.modifiers & Qt.ShiftModifier)) {
                            event.accepted = true
                            if (text.trim().length > 0) {
                                inputBar.sendMessage(text.trim())
                                text = ""
                            }
                        }
                    }

                    onTextChanged: {
                        // Auto-scroll to bottom when typing
                        flickable.contentY = Math.max(0, contentHeight - flickable.height)
                    }
                }
            }

            // Fade indicator at top when scrolled
            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 12
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#f5f5f5" }
                    GradientStop { position: 1.0; color: "transparent" }
                }
                visible: flickable.contentY > 2
            }
        }

        // Send button - fixed square with text
        Button {
            id: sendButton
            Layout.preferredWidth: 72
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignVCenter
            enabled: editArea.text.trim().length > 0

            background: Rectangle {
                radius: 6
                color: sendButton.hovered
                    ? (sendButton.enabled ? "#40a9ff" : "#d9d9d9")
                    : (sendButton.enabled ? "#1890ff" : "#d9d9d9")
                Behavior on color { ColorAnimation { duration: 150 } }
            }

            contentItem: Text {
                text: "发送~biu"
                font.pixelSize: 13
                font.bold: true
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            ToolTip.visible: hovered
            ToolTip.text: "发送 (Enter)"
            ToolTip.delay: 500

            onClicked: {
                if (editArea.text.trim().length > 0) {
                    inputBar.sendMessage(editArea.text.trim())
                    editArea.text = ""
                }
            }
        }
    }

    // Top border
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: "#e0e0e0"
    }
}
