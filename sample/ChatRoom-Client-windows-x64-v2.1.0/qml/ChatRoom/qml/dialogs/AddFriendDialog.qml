import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChatRoom

Popup {
    id: addFriendDialog

    signal addFriend(var userId, string message)

    width: 350
    height: 280
    anchors.centerIn: parent
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
            text: "Add Friend"
            font.pixelSize: 18
            font.bold: true
            color: "#333"
        }

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: "Search by username..."
            font.pixelSize: 14

            onAccepted: searchButton.clicked()
        }

        Button {
            id: searchButton
            Layout.fillWidth: true
            text: "Search"
            font.pixelSize: 14

            onClicked: {
                appCore.friendController.searchUser(searchField.text)
            }
        }

        // Search results
        ListView {
            id: searchResults
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: appCore.friendController.userModel

            delegate: Rectangle {
                width: ListView.view.width
                height: 48
                color: index % 2 === 0 ? "white" : "#fafafa"
                radius: 4

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    Avatar {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        name: model.nickname || model.username
                    }

                    Text {
                        text: model.nickname || model.username
                        font.pixelSize: 14
                        color: "#333"
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    Button {
                        text: "Add"
                        implicitHeight: 28
                        font.pixelSize: 12

                        onClicked: {
                            addFriendDialog.addFriend(model.userId, "Hi, I'd like to add you as a friend!")
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }
    }

    Connections {
        target: appCore.friendController
        function onFriendAdded() {
            appCore.showToast("Friend request sent", "success")
        }
    }

    onClosed: {
        appCore.friendController.requestFriendList()
    }
}
