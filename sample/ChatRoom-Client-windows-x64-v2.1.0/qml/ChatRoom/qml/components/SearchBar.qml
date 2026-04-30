import QtQuick
import QtQuick.Controls

Rectangle {
    id: searchBar

    property alias text: input.text

    signal search(string keyword)

    height: 36
    radius: 18
    color: "#f5f5f5"
    border.width: input.activeFocus ? 1 : 0
    border.color: "#1890ff"

    Row {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 8

        Text {
            text: "\u{1F50D}"  // Search icon
            font.pixelSize: 16
            anchors.verticalCenter: parent.verticalCenter
            color: "#999"
        }

        TextInput {
            id: input
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - 40
            font.pixelSize: 13
            color: "#333"
            selectByMouse: true
            clip: true

            onTextChanged: {
                searchBar.search(text)
            }
        }
    }

    // Clear button
    Rectangle {
        visible: input.text.length > 0
        width: 16
        height: 16
        radius: 8
        color: "#ccc"
        anchors {
            right: parent.right
            rightMargin: 10
            verticalCenter: parent.verticalCenter
        }

        Text {
            text: "\u00D7"
            font.pixelSize: 12
            color: "white"
            anchors.centerIn: parent
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                input.text = ""
                input.forceActiveFocus()
            }
        }
    }
}
