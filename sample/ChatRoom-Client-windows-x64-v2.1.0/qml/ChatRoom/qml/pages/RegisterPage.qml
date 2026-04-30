import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: registerPage

    signal registerSuccess()
    signal goBack()

    // Server connection fields passed from LoginPage
    property string serverHost: "127.0.0.1"
    property int serverPort: 6667
    property bool pendingRegister: false
    property bool registering: false

    background: Rectangle {
        color: "#f0f2f5"
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 16
        width: Math.min(parent.width * 0.8, 400)

        // Title
        Text {
            text: "Create Account"
            font.pixelSize: 28
            font.bold: true
            color: "#333"
            Layout.alignment: Qt.AlignHCenter
        }

        // Server address
        TextField {
            id: serverHostField
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            placeholderText: "Server Address (e.g. 192.168.1.100)"
            text: registerPage.serverHost
            font.pixelSize: 14
        }

        // Port
        TextField {
            id: serverPortField
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            placeholderText: "Port"
            text: registerPage.serverPort.toString()
            font.pixelSize: 14
            inputMethodHints: Qt.ImhDigitsOnly
            validator: IntValidator { bottom: 1; top: 65535 }
        }

        // Username
        TextField {
            id: usernameField
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            placeholderText: "Username"
            font.pixelSize: 14
        }

        // Password
        TextField {
            id: passwordField
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            placeholderText: "Password"
            echoMode: TextInput.Password
            font.pixelSize: 14
        }

        // Confirm Password
        TextField {
            id: confirmPasswordField
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            placeholderText: "Confirm Password"
            echoMode: TextInput.Password
            font.pixelSize: 14
        }

        // Nickname
        TextField {
            id: nicknameField
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            placeholderText: "Nickname (optional)"
            font.pixelSize: 14
        }

        // Error message
        Text {
            id: errorText
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            color: "#ff4d4f"
            font.pixelSize: 12
            wrapMode: Text.Wrap
            visible: false
        }

        // Register button
        Button {
            id: registerButton
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            text: "Register"
            font.pixelSize: 16
            font.bold: true
            enabled: usernameField.text.length > 0
                     && passwordField.text.length > 0
                     && passwordField.text === confirmPasswordField.text
                     && !registering

            background: Rectangle {
                radius: 6
                color: registerButton.enabled ? "#52c41a" : "#d9d9d9"
                implicitHeight: 44
            }
            contentItem: Text {
                text: registerButton.text
                color: "white"
                font: registerButton.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                errorText.visible = false
                if (passwordField.text !== confirmPasswordField.text) {
                    errorText.text = "Passwords do not match"
                    errorText.visible = true
                    return
                }
                // Connect to server first, then send register request
                pendingRegister = true
                registering = true
                appCore.connectToServer(serverHostField.text, parseInt(serverPortField.text))
            }
        }

        // Loading indicator
        BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            running: registering
            implicitWidth: 32
            implicitHeight: 32
            visible: registering
        }

        // Back link
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Already have an account? <a href='#'>Back to Login</a>"
            font.pixelSize: 13
            color: "#1890ff"
            linkColor: "#1890ff"

            MouseArea {
                anchors.fill: parent
                onClicked: goBack()
            }
        }
    }

    Connections {
        target: appCore

        function onConnected() {
            // After TCP connection established, send register request
            if (pendingRegister) {
                pendingRegister = false
                appCore.userController.registerUser(
                    usernameField.text,
                    passwordField.text,
                    nicknameField.text
                )
            }
        }

        function onConnectionError(error) {
            if (pendingRegister) {
                pendingRegister = false
                registering = false
                errorText.text = "Connection failed: " + error
                errorText.visible = true
            }
        }
    }

    Connections {
        target: appCore.userController

        function onRegisterSuccess() {
            registering = false
            appCore.showToast("Registration successful! Please login.", "success")
            registerSuccess()
        }

        function onRegisterFailed(reason) {
            registering = false
            errorText.text = reason
            errorText.visible = true
        }
    }
}
