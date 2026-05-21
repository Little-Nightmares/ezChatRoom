import QtQuick
import QtQuick.Controls

Item {
    id: root

    property string name: ""
    property url imageUrl: ""
    property color fillColor: "#dbeafe"
    property color textColor: "#1d4ed8"

    implicitWidth: 40
    implicitHeight: 40

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: root.fillColor
        clip: true

        Image {
            id: avatarImage
            anchors.fill: parent
            source: root.imageUrl
            fillMode: Image.PreserveAspectCrop
            visible: status === Image.Ready
        }

        Label {
            anchors.centerIn: parent
            visible: !avatarImage.visible
            text: root.name.length > 0 ? root.name.charAt(0).toUpperCase() : "?"
            color: root.textColor
            font.bold: true
            font.pixelSize: Math.max(12, root.width * 0.42)
        }
    }
}
