import QtQuick
import QtQuick.Controls

Item {
    id: root

    property url source: ""
    property bool opened: false

    anchors.fill: parent
    visible: opened
    z: 100

    function open(imageSource) {
        source = imageSource
        opened = true
    }

    function close() {
        opened = false
    }

    Rectangle {
        anchors.fill: parent
        color: "#cc000000"
    }

    Image {
        anchors.centerIn: parent
        width: Math.min(parent.width - 48, implicitWidth)
        height: Math.min(parent.height - 96, implicitHeight)
        source: root.source
        fillMode: Image.PreserveAspectFit
    }

    Button {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 16
        text: qsTr("Close")
        onClicked: root.close()
    }

    MouseArea {
        anchors.fill: parent
        z: -1
        onClicked: root.close()
    }
}
