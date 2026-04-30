import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChatRoom

Page {
    id: friendRequestPage

    signal goBack()

    background: Rectangle {
        color: "#f0f2f5"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: "#1890ff"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12

                Button {
                    flat: true
                    text: "< Back"
                    font.pixelSize: 14
                    onClicked: goBack()
                }

                Text {
                    text: "Friend Requests"
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                    Layout.fillWidth: true
                }

                Button {
                    flat: true
                    text: "+ Add"
                    font.pixelSize: 14
                    onClicked: addFriendDialog.open()
                }
            }
        }

        // Request list
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: appCore.friendRequestModel

            delegate: Rectangle {
                width: ListView.view.width
                height: 80
                color: index % 2 === 0 ? "white" : "#fafafa"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Avatar {
                        Layout.preferredWidth: 44
                        Layout.preferredHeight: 44
                        name: model.fromUsername
                    }

                    Column {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: model.fromUsername
                            font.bold: true
                            font.pixelSize: 15
                            color: "#333"
                        }

                        Text {
                            text: model.message || "(No message)"
                            font.pixelSize: 13
                            color: "#888"
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    Row {
                        spacing: 8

                        Button {
                            text: "Accept"
                            visible: model.status === 0
                            implicitHeight: 32

                            onClicked: {
                                appCore.friendController.acceptFriendRequest(model.requestId)
                            }
                        }

                        Button {
                            text: "Reject"
                            visible: model.status === 0
                            implicitHeight: 32
                            flat: true

                            onClicked: {
                                appCore.friendController.rejectFriendRequest(model.requestId)
                            }
                        }

                        Text {
                            text: model.status === 1 ? "Accepted" : "Rejected"
                            font.pixelSize: 13
                            color: model.status === 1 ? "#52c41a" : "#ff4d4f"
                            visible: model.status !== 0
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }
    }

    AddFriendDialog {
        id: addFriendDialog
        onAddFriend: function(userId, message) {
            appCore.friendController.addFriend(userId, message)
        }
    }

    Connections {
        target: appCore.friendController
        function onFriendRequestAccepted() {
            appCore.friendController.requestFriendRequestList()
        }
        function onFriendRequestRejected() {
            appCore.friendController.requestFriendRequestList()
        }
    }
}
