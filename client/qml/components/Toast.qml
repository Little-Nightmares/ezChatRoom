import QtQuick
import QtQuick.Controls

Item {
    id: root

    property string message: ""

    width: Math.min(parent ? parent.width - 32 : 320, 320)
    height: toastLabel.implicitHeight + 20
    opacity: 0
    visible: opacity > 0
    z: 200

    function show(text, timeoutMs) {
        message = text
        opacity = 1
        hideTimer.interval = timeoutMs || 2200
        hideTimer.restart()
    }

    Rectangle {
        anchors.fill: parent
        radius: 6
        color: "#eaf2ff"
        border.color: "#bfdbfe"
    }

    Label {
        id: toastLabel
        anchors.fill: parent
        anchors.margins: 10
        text: root.message
        color: "#1e3a8a"
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Timer {
        id: hideTimer
        onTriggered: root.opacity = 0
    }

    Behavior on opacity {
        NumberAnimation { duration: 160 }
    }
}
