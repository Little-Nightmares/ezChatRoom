import QtQuick

Rectangle {
    id: toast

    property string message: ""
    property string type: "info"  // "success", "error", "warning", "info"
    property int duration: 3000

    signal showToast(string msg, string msgType)

    width: messageText.implicitWidth + 32
    height: 40
    radius: 8
    color: {
        switch (type) {
        case "success": return "#f6ffed"
        case "error":   return "#fff2f0"
        case "warning": return "#fffbe6"
        default:        return "#e6f7ff"
        }
    }
    border.width: 1
    border.color: {
        switch (type) {
        case "success": return "#b7eb8f"
        case "error":   return "#ffa39e"
        case "warning": return "#ffe58f"
        default:        return "#91d5ff"
        }
    }
    visible: false
    opacity: 0

    Text {
        id: messageText
        anchors.centerIn: parent
        text: toast.message
        font.pixelSize: 13
        color: {
            switch (toast.type) {
            case "success": return "#52c41a"
            case "error":   return "#ff4d4f"
            case "warning": return "#faad14"
            default:        return "#1890ff"
            }
        }
    }

    function show(msg, msgType) {
        toast.message = msg
        toast.type = msgType || "info"
        toast.visible = true
        toast.opacity = 1

        hideTimer.restart()
    }

    Timer {
        id: hideTimer
        interval: toast.duration
        repeat: false
        onTriggered: {
            fadeOutAnimation.start()
        }
    }

    NumberAnimation {
        id: fadeOutAnimation
        target: toast
        property: "opacity"
        from: 1
        to: 0
        duration: 300
        onFinished: {
            toast.visible = false
        }
    }
}
