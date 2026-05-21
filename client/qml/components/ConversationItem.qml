import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property string title: ""
    property string lastMessage: ""
    property string lastTime: ""
    property string avatarUrl: ""
    property int unreadCount: 0
    property bool selected: false

    signal clicked()

    implicitHeight: 68
    width: parent ? parent.width : 300

    Rectangle {
        anchors.fill: parent
        radius: 6
        color: root.selected ? "#e0ecff" : (mouseArea.containsMouse ? "#f3f6fb" : "transparent")
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 10

        Avatar {
            Layout.preferredWidth: 44
            Layout.preferredHeight: 44
            name: root.title
            imageUrl: root.avatarUrl
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: root.title
                    color: "#111827"
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Label {
                    text: root.lastTime
                    color: "#94a3b8"
                    font.pixelSize: 11
                }
            }

            Label {
                text: root.lastMessage
                color: "#64748b"
                font.pixelSize: 12
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        Label {
            visible: root.unreadCount > 0
            text: root.unreadCount > 99 ? "99+" : String(root.unreadCount)
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            Layout.preferredWidth: Math.max(22, implicitWidth + 10)
            Layout.preferredHeight: 22

            background: Rectangle {
                radius: 11
                color: "#ef4444"
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
