import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root
    title: qsTr("Register")

    signal registerRequested(string username, string password, string displayName)

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 360)
        spacing: 14

        Label {
            text: qsTr("Create Account")
            font.pixelSize: 24
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        TextField {
            id: usernameField
            placeholderText: qsTr("Username")
            Layout.fillWidth: true
        }

        TextField {
            id: displayNameField
            placeholderText: qsTr("Display name")
            Layout.fillWidth: true
        }

        TextField {
            id: passwordField
            placeholderText: qsTr("Password")
            echoMode: TextInput.Password
            Layout.fillWidth: true
        }

        TextField {
            id: confirmPasswordField
            placeholderText: qsTr("Confirm password")
            echoMode: TextInput.Password
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("Register")
            enabled: usernameField.text.trim().length > 0
                     && passwordField.text.length > 0
                     && passwordField.text === confirmPasswordField.text
            Layout.fillWidth: true
            onClicked: root.registerRequested(usernameField.text.trim(),
                                              passwordField.text,
                                              displayNameField.text.trim())
        }
    }
}
