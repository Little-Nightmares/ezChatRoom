import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: loginPage

    signal loginSuccess(string host, int port)
    signal goToRegister()

    // Track pending login to send credentials after connection
    property bool pendingLogin: false
    property bool loggingIn: false
    property alias serverHostText: serverHostField.text
    property alias serverPortText: serverPortField.text

    background: Rectangle {
        color: "#f0f2f5"
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20
        width: Math.min(parent.width * 0.8, 400)

        // Title
        Column {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8
            Text {
                text: "ChatRoom"
                font.pixelSize: 36
                font.bold: true
                color: "#1890ff"
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: "LAN Messenger"
                font.pixelSize: 14
                color: "#888"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        // Server address
        TextField {
            id: serverHostField
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            placeholderText: "Server Address (e.g. 192.168.1.100)"
            text: "127.0.0.1"
            font.pixelSize: 14
        }

        // Port
        TextField {
            id: serverPortField
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            placeholderText: "Port"
            text: "6667"
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
            onAccepted: loginButton.clicked()
        }

        // Login button
        Button {
            id: loginButton
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            text: "Login"
            font.pixelSize: 16
            font.bold: true
            enabled: usernameField.text.length > 0 && passwordField.text.length > 0

            background: Rectangle {
                radius: 6
                color: loginButton.enabled ? "#1890ff" : "#d9d9d9"
                implicitHeight: 44
            }
            contentItem: Text {
                text: loginButton.text
                color: "white"
                font: loginButton.font
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                pendingLogin = true
                loggingIn = true
                appCore.connectToServer(serverHostField.text, parseInt(serverPortField.text))
            }
        }

        // Loading indicator
        BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            running: loggingIn
            implicitWidth: 32
            implicitHeight: 32
            visible: loggingIn
        }

        // Register link
        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Don't have an account? <a href='#'>Register</a>"
            font.pixelSize: 13
            color: "#1890ff"
            linkColor: "#1890ff"

            onLinkActivated: {
                goToRegister()
            }
        }
    }

    Connections {
        target: appCore

        function onConnected() {
            // After TCP connection established, send login request
            if (pendingLogin) {
                pendingLogin = false
                appCore.userController.login(usernameField.text, passwordField.text)
            }
        }

        function onConnectionError(error) {
            pendingLogin = false
            loggingIn = false
            appCore.showToast("Connection failed: " + error, "error")
        }
    }

    Connections {
        target: appCore.userController

        function onLoginSuccess() {
            loggingIn = false
            loginSuccess(serverHostField.text, parseInt(serverPortField.text))
        }

        function onLoginFailed(reason) {
            loggingIn = false
            appCore.showToast("Login failed: " + reason, "error")
        }
    }
}
