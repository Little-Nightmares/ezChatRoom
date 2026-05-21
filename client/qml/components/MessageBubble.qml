import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property string content: ""
    property bool mine: false
    property string senderName: ""
    property string timeText: ""

    implicitHeight: bubbleRow.implicitHeight + 8
    width: parent ? parent.width : 360

    RowLayout {
        id: bubbleRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 8
        layoutDirection: root.mine ? Qt.RightToLeft : Qt.LeftToRight
        spacing: 8

        Avatar {
            Layout.preferredWidth: 32
            Layout.preferredHeight: 32
            name: root.senderName
        }

        ColumnLayout {
            spacing: 3
            Layout.maximumWidth: Math.max(180, root.width * 0.72)

            Rectangle {
                radius: 8
                color: root.mine ? "#2563eb" : "#ffffff"
                border.color: root.mine ? "#2563eb" : "#d9e2ec"
                Layout.fillWidth: true
                implicitHeight: messageText.implicitHeight + 16

                Label {
                    id: messageText
                    anchors.fill: parent
                    anchors.margins: 8
                    text: root.content
                    color: root.mine ? "white" : "#111827"
                    wrapMode: Text.WrapAnywhere
                }
            }

            Label {
                text: root.timeText
                color: "#94a3b8"
                font.pixelSize: 11
                Layout.alignment: root.mine ? Qt.AlignRight : Qt.AlignLeft
            }
        }
    }
}
