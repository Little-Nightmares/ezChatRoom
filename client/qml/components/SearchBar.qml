import QtQuick
import QtQuick.Controls

Item {
    id: root

    property alias text: field.text
    property string placeholderText: qsTr("Search")
    signal accepted(string text)

    implicitHeight: 42

    TextField {
        id: field
        anchors.fill: parent
        placeholderText: root.placeholderText
        selectByMouse: true
        leftPadding: 12
        rightPadding: clearButton.visible ? 38 : 12
        onAccepted: root.accepted(text)

        background: Rectangle {
            radius: 6
            color: "#f8fafc"
            border.color: field.activeFocus ? "#2563eb" : "#d9e2ec"
        }
    }

    Button {
        id: clearButton
        visible: field.text.length > 0
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: 32
        height: 32
        text: "x"
        flat: true
        onClicked: field.text = ""
    }
}
