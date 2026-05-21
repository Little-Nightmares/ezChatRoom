import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root
    title: qsTr("Login")

    property string statusMessage: ""
    property color statusColor: "#5f6b7a"

    background: Rectangle {
        color: "#f4f7fb"
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 360)
        spacing: 14

        Label {
            text: qsTr("ChatRoom")
            font.pixelSize: 30
            font.bold: true
            color: "#1f2937"
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        TextField {
            id: serverField
            text: "127.0.0.1"
            placeholderText: qsTr("Server")
            selectByMouse: true
            Layout.fillWidth: true
        }

        TextField {
            id: portField
            text: "6667"
            placeholderText: qsTr("Port")
            inputMethodHints: Qt.ImhDigitsOnly
            validator: IntValidator { bottom: 1; top: 65535 }
            selectByMouse: true
            Layout.fillWidth: true
        }

        TextField {
            id: usernameField
            placeholderText: qsTr("Username")
            selectByMouse: true
            Layout.fillWidth: true
        }

        TextField {
            id: passwordField
            placeholderText: qsTr("Password")
            echoMode: TextInput.Password
            selectByMouse: true
            Layout.fillWidth: true
            Keys.onReturnPressed: loginButton.clicked()
        }

        Button {
            id: loginButton
            text: qsTr("Login")
            Layout.fillWidth: true
            onClicked: {
                UserController.login(usernameField.text,
                                     passwordField.text,
                                     serverField.text,
                                     Number(portField.text))
            }
        }

        Label {
            text: root.statusMessage
            visible: text.length > 0
            color: root.statusColor
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
    }

    Connections {
        target: UserController

        function onLoginStarted(username, host, port) {
            root.statusColor = "#2563eb"
            root.statusMessage = qsTr("Connecting to %1:%2").arg(host).arg(port)
        }

        function onServerConnected() {
            root.statusColor = "#15803d"
            root.statusMessage = qsTr("Connected")
        }

        function onServerDisconnected() {
            root.statusColor = "#5f6b7a"
            root.statusMessage = qsTr("Disconnected")
        }

        function onLoginFailed(message) {
            root.statusColor = "#b91c1c"
            root.statusMessage = message
        }
    }
}
