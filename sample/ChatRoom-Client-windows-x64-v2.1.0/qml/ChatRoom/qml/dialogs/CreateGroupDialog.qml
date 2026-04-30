import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChatRoom

Popup {
    id: createGroupDialog

    signal groupCreated(var groupId)

    width: 380
    height: 440
    anchors.centerIn: parent
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        color: "white"
        radius: 12
        border.color: "#e0e0e0"
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        // Title
        Text {
            text: "创建群组"
            font.pixelSize: 18
            font.bold: true
            color: "#333"
        }

        // Group name input
        TextField {
            id: groupNameField
            Layout.fillWidth: true
            placeholderText: "请输入群名"
            font.pixelSize: 14
            color: "#333"
            maximumLength: 30

            background: Rectangle {
                radius: 6
                color: "#f5f5f5"
                border.color: groupNameField.activeFocus ? "#1890ff" : "#d9d9d9"
                border.width: 1
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#f0f0f0"
        }

        // Select members label
        Text {
            text: "选择成员"
            font.pixelSize: 14
            font.bold: true
            color: "#333"
        }

        // Search friends
        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: "搜索好友..."
            font.pixelSize: 13
            color: "#333"

            background: Rectangle {
                radius: 6
                color: "#f5f5f5"
                border.color: searchField.activeFocus ? "#1890ff" : "#d9d9d9"
                border.width: 1
            }
        }

        // Friends list with checkboxes
        ListView {
            id: friendsList
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
                    spacing: 10

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

                    CheckBox {
                        checked: selectedFriends.indexOf(model.userId) >= 0
                        onCheckedChanged: {
                            if (checked) {
                                if (selectedFriends.indexOf(model.userId) < 0) {
                                    selectedFriends.push(model.userId)
                                }
                            } else {
                                var idx = selectedFriends.indexOf(model.userId)
                                if (idx >= 0) {
                                    selectedFriends.splice(idx, 1)
                                }
                            }
                            selectedFriendsChanged()
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }

        // Buttons row
        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Layout.topMargin: 4

            Item { Layout.fillWidth: true }

            Button {
                text: "取消"
                implicitWidth: 80
                implicitHeight: 36
                font.pixelSize: 14

                background: Rectangle {
                    radius: 6
                    color: cancelBtn.hovered ? "#f0f0f0" : "#fafafa"
                    border.color: "#d9d9d9"
                    border.width: 1
                }

                contentItem: Text {
                    text: cancelBtn.text
                    font.pixelSize: 14
                    color: "#666"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                id: cancelBtn
                onClicked: createGroupDialog.close()
            }

            Button {
                id: createBtn
                text: "创建"
                implicitWidth: 80
                implicitHeight: 36
                font.pixelSize: 14
                enabled: groupNameField.text.trim().length > 0 && selectedFriends.length > 0

                background: Rectangle {
                    radius: 6
                    color: createBtn.hovered
                        ? (createBtn.enabled ? "#40a9ff" : "#d9d9d9")
                        : (createBtn.enabled ? "#1890ff" : "#d9d9d9")
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                contentItem: Text {
                    text: createBtn.text
                    font.pixelSize: 14
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if (groupNameField.text.trim().length > 0 && selectedFriends.length > 0) {
                        appCore.groupController.createGroup(groupNameField.text.trim(), selectedFriends)
                    }
                }
            }
        }
    }

    // Track selected friend IDs
    property var selectedFriends: []

    Connections {
        target: appCore.groupController
        function onGroupCreated(groupId) {
            appCore.showToast("群组创建成功", "success")
            createGroupDialog.groupCreated(groupId)
            createGroupDialog.close()
        }
        function onGroupCreateFailed(errorMsg) {
            appCore.showToast(errorMsg || "创建群组失败", "error")
        }
    }

    onClosed: {
        groupNameField.text = ""
        selectedFriends = []
    }
}
