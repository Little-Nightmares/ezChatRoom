import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property string displayName: ""
    property string subtitle: ""
    property string avatarUrl: ""
    property bool online: false
    property bool selected: false

    signal clicked()

    implicitHeight: 64
    width: parent ? parent.width : 280

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
            Layout.preferredWidth: 42
            Layout.preferredHeight: 42
            name: root.displayName
            imageUrl: root.avatarUrl
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Label {
                text: root.displayName
                color: "#1f2937"
                font.bold: true
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            Label {
                text: root.subtitle
                color: "#6b7280"
                font.pixelSize: 12
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        Rectangle {
            Layout.preferredWidth: 10
            Layout.preferredHeight: 10
            radius: 5
            color: root.online ? "#16a34a" : "#cbd5e1"
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
