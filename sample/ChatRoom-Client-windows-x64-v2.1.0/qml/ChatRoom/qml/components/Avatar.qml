import QtQuick

Rectangle {
    id: avatar

    property string name: ""
    property string source: ""
    property string imageUrl: ""
    property bool showOnlineIndicator: false
    property bool online: false

    width: 40
    height: 40
    radius: width / 2
    color: getAvatarColor(name)

    // Image avatar (from uploaded imageUrl)
    Image {
        id: avatarImage
        anchors.fill: parent
        source: avatar.imageUrl !== "" ? avatar.imageUrl : ""
        visible: avatar.imageUrl !== "" && status === Image.Ready
        fillMode: Image.PreserveAspectCrop
    }

    // Initials text (shown when no image)
    Text {
        anchors.centerIn: parent
        text: getInitials(name)
        font.pixelSize: parent.width * 0.4
        font.bold: true
        color: "white"
        visible: avatar.imageUrl === "" || avatarImage.status !== Image.Ready
    }

    // Online indicator
    Rectangle {
        visible: avatar.showOnlineIndicator
        width: 10
        height: 10
        radius: 5
        color: avatar.online ? "#52c41a" : "#d9d9d9"
        anchors {
            bottom: parent.bottom
            right: parent.right
            bottomMargin: -1
            rightMargin: -1
        }
        border.width: 2
        border.color: "white"
    }

    function getInitials(name) {
        if (!name || name.length === 0) return "?"
        // For Chinese names, take the last character; for others, take first letter
        var firstChar = name.charAt(0)
        if (firstChar.charCodeAt(0) >= 0x4e00 && firstChar.charCodeAt(0) <= 0x9fff) {
            return name.length > 1 ? name.charAt(name.length - 1) : firstChar
        }
        return firstChar.toUpperCase()
    }

    function getAvatarColor(name) {
        if (!name || name.length === 0) return "#999"
        var colors = [
            "#1890ff", "#52c41a", "#faad14", "#ff4d4f",
            "#722ed1", "#13c2c2", "#eb2f96", "#fa8c16",
            "#2f54eb", "#a0d911"
        ]
        var hash = 0
        for (var i = 0; i < name.length; i++) {
            hash = name.charCodeAt(i) + ((hash << 5) - hash)
        }
        return colors[Math.abs(hash) % colors.length]
    }
}
