import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Page {
    id: root
    title: qsTr("Profile")

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 380)
        spacing: 14

        Avatar {
            Layout.preferredWidth: 82
            Layout.preferredHeight: 82
            Layout.alignment: Qt.AlignHCenter
            name: displayNameField.text
        }

        TextField {
            id: displayNameField
            text: "ChatRoom User"
            placeholderText: qsTr("Display name")
            Layout.fillWidth: true
        }

        TextField {
            text: "user@example.lan"
            placeholderText: qsTr("Account")
            readOnly: true
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("Save")
            Layout.fillWidth: true
        }
    }
}
