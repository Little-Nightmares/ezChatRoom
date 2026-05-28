import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChatRoom

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
        color: appCore.themeManager.background
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20
        width: Math.min(parent.width * 0.8, 400)

        // Title
        Column {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8
            StyledText {
                text: "ChatRoom"
                font.pixelSize: 36
                font.bold: true
                color: appCore.themeManager.primary
                anchors.horizontalCenter: parent.horizontalCenter
            }
            StyledText {
                text: "LAN Messenger"
                font.pixelSize: 14
                color: appCore.themeManager.textTertiary
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
            font.family: appCore.themeManager.fontFamily
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
            font.family: appCore.themeManager.fontFamily
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
            font.family: appCore.themeManager.fontFamily
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
            font.family: appCore.themeManager.fontFamily
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
                color: loginButton.enabled ? appCore.themeManager.primary : appCore.themeManager.borderColor
                implicitHeight: 44
            }
            contentItem: StyledText {
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
        StyledText {
            Layout.alignment: Qt.AlignHCenter
            text: "Don't have an account? <a href='#'>Register</a>"
            font.pixelSize: 13
            color: appCore.themeManager.primary
            linkColor: appCore.themeManager.primary

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
