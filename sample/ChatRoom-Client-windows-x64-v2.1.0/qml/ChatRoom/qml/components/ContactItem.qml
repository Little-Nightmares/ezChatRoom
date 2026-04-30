import QtQuick
import ChatRoom

Rectangle {
    id: contactItem

    property int userId: 0
    property alias nickname: nicknameText.text
    property alias username: usernameText.text
    property alias avatar: avatarComp.name
    property bool isOnline: false
    property bool selected: false

    signal clicked()

    width: parent ? parent.width : 200
    height: 60
    color: selected ? "#e6f7ff" : (mouseArea.containsMouse ? "#f5f5f5" : "transparent")

    Row {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 10

        Avatar {
            id: avatarComp
            width: 40
            height: 40
            anchors.verticalCenter: parent.verticalCenter
            showOnlineIndicator: true
            online: contactItem.isOnline
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            Text {
                id: nicknameText
                text: ""
                font.pixelSize: 14
                font.bold: true
                color: "#333"
                elide: Text.ElideRight
                width: contactItem.width - 80
            }

            Text {
                id: usernameText
                text: ""
                font.pixelSize: 12
                color: "#999"
                elide: Text.ElideRight
                width: contactItem.width - 80
            }
        }
    }

    // Bottom divider
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 62
        height: 1
        color: "#f0f0f0"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: contactItem.clicked()
    }
}
