import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChatRoom

Popup {
    id: groupInfoDialog

    property var groupId: 0
    property string groupName: ""
    property string groupAvatar: ""
    property bool isOwner: false
    property bool isDefault: false
    property var memberList: []  // local member data

    signal groupUpdated()
    signal groupDismissed()

    width: 380
    height: 520
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

        // Group avatar display
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            Layout.alignment: Qt.AlignHCenter

            Avatar {
                id: groupAvatarComp
                width: 64
                height: 64
                name: groupInfoDialog.groupName
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // Group name (editable if owner)
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: groupNameField
                Layout.fillWidth: true
                text: groupInfoDialog.groupName
                font.pixelSize: 16
                font.bold: true
                color: "#333"
                readOnly: !groupInfoDialog.isOwner
                horizontalAlignment: TextInput.AlignHCenter
                maximumLength: 30

                background: Rectangle {
                    radius: 6
                    color: "transparent"
                    border.color: groupNameField.activeFocus && groupInfoDialog.isOwner ? "#1890ff" : "transparent"
                    border.width: 1
                }

                onEditingFinished: {
                    if (groupInfoDialog.isOwner && text.trim().length > 0 && text.trim() !== groupInfoDialog.groupName) {
                        appCore.groupController.updateGroupName(groupInfoDialog.groupId, text.trim())
                    }
                }
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#f0f0f0"
        }

        // Members label
        Text {
            text: "成员 (" + memberListView.count + "人)"
            font.pixelSize: 14
            font.bold: true
            color: "#333"
        }

        // Members list
        ListView {
            id: memberListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: groupInfoDialog.memberList

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
                        name: modelData.nickname || modelData.username || ""
                    }

                    Text {
                        text: modelData.nickname || modelData.username || ""
                        font.pixelSize: 14
                        color: "#333"
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    // Role badge
                    Rectangle {
                        visible: modelData.role === 1
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 18
                        radius: 3
                        color: "#faad14"

                        Text {
                            text: "群主"
                            font.pixelSize: 10
                            color: "white"
                            anchors.centerIn: parent
                        }
                    }

                    Rectangle {
                        visible: modelData.role !== 1
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 18
                        radius: 3
                        color: "#f0f0f0"

                        Text {
                            text: "成员"
                            font.pixelSize: 10
                            color: "#999"
                            anchors.centerIn: parent
                        }
                    }

                    // Remove button (only for owner, not on self)
                    Button {
                        visible: groupInfoDialog.isOwner && modelData.role !== 1
                        text: "移除"
                        implicitWidth: 48
                        implicitHeight: 26
                        font.pixelSize: 11

                        background: Rectangle {
                            radius: 4
                            color: removeBtn.hovered ? "#fff1f0" : "#fff2f0"
                            border.color: "#ffccc7"
                            border.width: 1
                        }

                        contentItem: Text {
                            text: removeBtn.text
                            font.pixelSize: 11
                            color: "#ff4d4f"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        id: removeBtn
                        onClicked: {
                            appCore.groupController.removeMember(groupInfoDialog.groupId, modelData.userId)
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }

        // Action buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // Add member button (only for owner)
            Button {
                visible: groupInfoDialog.isOwner
                Layout.fillWidth: true
                text: "添加成员"
                implicitHeight: 36
                font.pixelSize: 13

                background: Rectangle {
                    radius: 6
                    color: addMemberBtn.hovered ? "#e6f7ff" : "#f0f5ff"
                    border.color: "#1890ff"
                    border.width: 1
                }

                contentItem: Text {
                    text: addMemberBtn.text
                    font.pixelSize: 13
                    color: "#1890ff"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                id: addMemberBtn
                onClicked: {
                    addMemberDialog.open()
                }
            }
        }

        // Owner action buttons
        RowLayout {
            visible: groupInfoDialog.isOwner && !groupInfoDialog.isDefault
            Layout.fillWidth: true
            spacing: 8

            // Transfer ownership button
            Button {
                Layout.fillWidth: true
                text: "转让群主"
                implicitHeight: 36
                font.pixelSize: 13

                background: Rectangle {
                    radius: 6
                    color: transferBtn.hovered ? "#fff7e6" : "#fffbe6"
                    border.color: "#faad14"
                    border.width: 1
                }

                contentItem: Text {
                    text: transferBtn.text
                    font.pixelSize: 13
                    color: "#faad14"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                id: transferBtn
                onClicked: {
                    transferDialog.open()
                }
            }

            // Dismiss group button
            Button {
                Layout.fillWidth: true
                text: "解散群组"
                implicitHeight: 36
                font.pixelSize: 13

                background: Rectangle {
                    radius: 6
                    color: dismissBtn.hovered ? "#ff4d4f" : "#ff7875"
                }

                contentItem: Text {
                    text: dismissBtn.text
                    font.pixelSize: 13
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                id: dismissBtn
                onClicked: {
                    confirmDismissDialog.open()
                }
            }
        }

        // Non-owner: leave group button
        Button {
            visible: !groupInfoDialog.isOwner
            Layout.fillWidth: true
            text: "退出群组"
            implicitHeight: 36
            font.pixelSize: 13

            background: Rectangle {
                radius: 6
                color: leaveBtn.hovered ? "#ff4d4f" : "#ff7875"
            }

            contentItem: Text {
                text: leaveBtn.text
                font.pixelSize: 13
                font.bold: true
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            id: leaveBtn
            onClicked: {
                confirmLeaveDialog.open()
            }
        }
    }

    // Confirm dismiss dialog
    Popup {
        id: confirmDismissDialog
        anchors.centerIn: parent
        width: 280
        height: 120
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "white"
            radius: 8
            border.color: "#e0e0e0"
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text {
                text: "确定要解散该群组吗？"
                font.pixelSize: 14
                color: "#333"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                text: "此操作不可撤销"
                font.pixelSize: 12
                color: "#ff4d4f"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Item { Layout.fillWidth: true }

                Button {
                    text: "取消"
                    implicitWidth: 64
                    implicitHeight: 32
                    font.pixelSize: 13

                    background: Rectangle {
                        radius: 4
                        color: cancelDismissBtn.hovered ? "#f0f0f0" : "#fafafa"
                        border.color: "#d9d9d9"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: cancelDismissBtn.text
                        font.pixelSize: 13
                        color: "#666"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    id: cancelDismissBtn
                    onClicked: confirmDismissDialog.close()
                }

                Button {
                    text: "确定"
                    implicitWidth: 64
                    implicitHeight: 32
                    font.pixelSize: 13

                    background: Rectangle {
                        radius: 4
                        color: confirmDismissBtn.hovered ? "#ff4d4f" : "#ff7875"
                    }

                    contentItem: Text {
                        text: confirmDismissBtn.text
                        font.pixelSize: 13
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    id: confirmDismissBtn
                    onClicked: {
                        appCore.groupController.dissolveGroup(groupInfoDialog.groupId)
                        confirmDismissDialog.close()
                        groupInfoDialog.groupDismissed()
                        groupInfoDialog.close()
                    }
                }
            }
        }
    }

    // Confirm leave dialog
    Popup {
        id: confirmLeaveDialog
        anchors.centerIn: parent
        width: 280
        height: 120
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "white"
            radius: 8
            border.color: "#e0e0e0"
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text {
                text: "确定要退出该群组吗？"
                font.pixelSize: 14
                color: "#333"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Item { Layout.fillWidth: true }

                Button {
                    text: "取消"
                    implicitWidth: 64
                    implicitHeight: 32
                    font.pixelSize: 13

                    background: Rectangle {
                        radius: 4
                        color: cancelLeaveBtn.hovered ? "#f0f0f0" : "#fafafa"
                        border.color: "#d9d9d9"
                        border.width: 1
                    }

                    contentItem: Text {
                        text: cancelLeaveBtn.text
                        font.pixelSize: 13
                        color: "#666"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    id: cancelLeaveBtn
                    onClicked: confirmLeaveDialog.close()
                }

                Button {
                    text: "确定"
                    implicitWidth: 64
                    implicitHeight: 32
                    font.pixelSize: 13

                    background: Rectangle {
                        radius: 4
                        color: confirmLeaveBtn.hovered ? "#ff4d4f" : "#ff7875"
                    }

                    contentItem: Text {
                        text: confirmLeaveBtn.text
                        font.pixelSize: 13
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    id: confirmLeaveBtn
                    onClicked: {
                        appCore.groupController.leaveGroup(groupInfoDialog.groupId)
                        confirmLeaveDialog.close()
                        groupInfoDialog.close()
                    }
                }
            }
        }
    }

    // Add member dialog (inline)
    Popup {
        id: addMemberDialog
        width: 380
        height: 400
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

            Text {
                text: "添加成员"
                font.pixelSize: 18
                font.bold: true
                color: "#333"
            }

            ListView {
                id: addMemberList
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

                        Button {
                            text: "添加"
                            implicitWidth: 48
                            implicitHeight: 26
                            font.pixelSize: 11

                            background: Rectangle {
                                radius: 4
                                color: addBtn.hovered ? "#e6f7ff" : "#f0f5ff"
                                border.color: "#1890ff"
                                border.width: 1
                            }

                            contentItem: Text {
                                text: addBtn.text
                                font.pixelSize: 11
                                color: "#1890ff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            id: addBtn
                            onClicked: {
                                appCore.groupController.addMembers(groupInfoDialog.groupId, [model.userId])
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }

            Button {
                Layout.fillWidth: true
                text: "关闭"
                implicitHeight: 36
                font.pixelSize: 14
                onClicked: addMemberDialog.close()

                background: Rectangle {
                    radius: 6
                    color: closeAddBtn.hovered ? "#f0f0f0" : "#fafafa"
                    border.color: "#d9d9d9"
                    border.width: 1
                }

                contentItem: Text {
                    text: closeAddBtn.text
                    font.pixelSize: 14
                    color: "#666"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                id: closeAddBtn
            }
        }
    }

    // Transfer owner dialog (inline)
    Popup {
        id: transferDialog
        width: 380
        height: 400
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

            Text {
                text: "转让群主"
                font.pixelSize: 18
                font.bold: true
                color: "#333"
            }

            ListView {
                id: transferMemberList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: groupInfoDialog.memberList

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 48
                    color: index % 2 === 0 ? "white" : "#fafafa"
                    radius: 4
                    visible: modelData.role !== 1  // hide owner

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 10

                        Avatar {
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            name: modelData.nickname || modelData.username || ""
                        }

                        Text {
                            text: modelData.nickname || modelData.username || ""
                            font.pixelSize: 14
                            color: "#333"
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }

                        Button {
                            text: "转让"
                            implicitWidth: 48
                            implicitHeight: 26
                            font.pixelSize: 11

                            background: Rectangle {
                                radius: 4
                                color: transferBtn2.hovered ? "#fff7e6" : "#fffbe6"
                                border.color: "#faad14"
                                border.width: 1
                            }

                            contentItem: Text {
                                text: transferBtn2.text
                                font.pixelSize: 11
                                color: "#faad14"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            id: transferBtn2
                            onClicked: {
                                appCore.groupController.transferOwner(groupInfoDialog.groupId, modelData.userId)
                                transferDialog.close()
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }

            Button {
                Layout.fillWidth: true
                text: "取消"
                implicitHeight: 36
                font.pixelSize: 14
                onClicked: transferDialog.close()

                background: Rectangle {
                    radius: 6
                    color: cancelTransferBtn.hovered ? "#f0f0f0" : "#fafafa"
                    border.color: "#d9d9d9"
                    border.width: 1
                }

                contentItem: Text {
                    text: cancelTransferBtn.text
                    font.pixelSize: 14
                    color: "#666"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                id: cancelTransferBtn
            }
        }
    }

    Connections {
        target: appCore.groupController
        function onGroupUpdated() {
            appCore.showToast("群信息已更新", "success")
        }
        function onMemberRemoved() {
            appCore.showToast("成员已移除", "success")
        }
        function onOwnerTransferred() {
            appCore.showToast("群主已转让", "success")
            groupInfoDialog.close()
        }
        function onGroupDissolved() {
            appCore.showToast("群组已解散", "success")
            groupInfoDialog.close()
        }
        function onGroupLeft() {
            appCore.showToast("已退出群组", "success")
            groupInfoDialog.close()
        }
    }

    onAboutToShow: {
        appCore.groupController.requestGroupInfo(groupInfoDialog.groupId)
    }
}
