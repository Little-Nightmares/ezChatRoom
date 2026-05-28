import QtQuick
import ChatRoom

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
        case "success": return appCore.themeManager.success
        case "error":   return appCore.themeManager.danger
        case "warning": return appCore.themeManager.warning
        default:        return appCore.themeManager.primary
        }
    }
    visible: false
    opacity: 0

    StyledText {
        id: messageText
        anchors.centerIn: parent
        text: toast.message
        font.pixelSize: 13
        color: {
            switch (toast.type) {
            case "success": return appCore.themeManager.success
            case "error":   return appCore.themeManager.danger
            case "warning": return appCore.themeManager.warning
            default:        return appCore.themeManager.primary
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
