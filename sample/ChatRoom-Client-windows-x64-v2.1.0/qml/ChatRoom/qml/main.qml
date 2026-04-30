import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import ChatRoom

Window {
    id: mainWindow
    width: 1000
    height: 700
    minimumWidth: 800
    minimumHeight: 600
    visible: true
    title: "ChatRoom - LAN Messenger"

    color: "#f5f5f5"

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: appCore.loggedIn ? chatPage : loginPage
    }

    // Reconnect status bar
    Rectangle {
        id: reconnectBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 32
        color: "#fff7e6"
        visible: appCore.appManager && appCore.appManager.isReconnecting
        z: 100

        RowLayout {
            anchors.centerIn: parent
            spacing: 8

            Text {
                text: "连接已断开，正在重连... (" + (appCore.appManager ? appCore.appManager.reconnectAttempt : 0) + "/" + 5 + ")"
                font.pixelSize: 12
                color: "#fa8c16"
            }
        }
    }

    Component {
        id: loginPage
        LoginPage {
            onLoginSuccess: function(host, port) {
                stackView.replace(chatPage)
            }
            onGoToRegister: {
                var currentPage = stackView.currentItem
                stackView.push(registerPage, {
                    serverHost: currentPage.serverHostText,
                    serverPort: parseInt(currentPage.serverPortText)
                })
            }
        }
    }

    Component {
        id: registerPage
        RegisterPage {
            onRegisterSuccess: {
                stackView.pop()
            }
            onGoBack: {
                stackView.pop()
            }
        }
    }

    Component {
        id: chatPage
        ChatPage {
            onLogout: {
                appCore.logout()
                stackView.replace(loginPage)
            }
        }
    }

    // Global Toast notification
    Toast {
        id: globalToast
        z: 1000
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: 20
        }
    }

    Connections {
        target: appCore
        function onShowNotification(message, type) {
            globalToast.show(message, type)
        }
        function onNewMessagesReceived(senderId, senderName, count) {
            globalToast.show(senderName + "：" + (count > 1 ? count + "条新消息" : "新消息"))
        }
        function onKicked(reason) {
            kickDialog.reasonText = reason
            kickDialog.open()
        }
    }

    Popup {
        id: kickDialog
        property string reasonText: ""
        modal: true
        anchors.centerIn: parent
        width: 300
        height: 150
        closePolicy: Popup.NoAutoClose

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16

            Text {
                text: "您已被踢出"
                font.pixelSize: 18
                font.bold: true
                color: "#ff4d4f"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: kickDialog.reasonText
                font.pixelSize: 14
                color: "#666"
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }

            Button {
                text: "确定"
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    kickDialog.close()
                    appCore.logout()
                }
            }
        }
    }
}
