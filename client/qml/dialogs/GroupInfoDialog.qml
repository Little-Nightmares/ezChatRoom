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
        border.color: appCore.themeManager.borderColor
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
                font.family: appCore.themeManager.fontFamily
                color: appCore.themeManager.textPrimary
                readOnly: !groupInfoDialog.isOwner
                horizontalAlignment: TextInput.AlignHCenter
                maximumLength: 30

                background: Rectangle {
                    radius: 6
                    color: "transparent"
                    border.color: groupNameField.activeFocus && groupInfoDialog.isOwner ? appCore.themeManager.primary : "transparent"
                    border.width: 1
                }

                onEditingFinished: {
                    if (groupInfoDialog.isOwner && text.trim().length > 0 && text.trim() !== groupInfoDialog.groupName) {
                        appCore.groupController.updateGroupName(groupInfoDialog.groupId, text.trim())
                    }
                }
            }
        }

        // Announcement section
        Rectangle {
            visible: appCore.groupController.announcement.length > 0
            Layout.fillWidth: true
            Layout.preferredHeight: announcementText.implicitHeight + 16
            radius: 6
            color: "#fff7e6"
            border.color: "#ffd591"
            border.width: 1

            StyledText {
                id: announcementText
                anchors.fill: parent
                anchors.margins: 8
                text: "\uD83D\uDCE2 " + appCore.groupController.announcement
                font.pixelSize: 12
                color: "#d48806"
                wrapMode: Text.Wrap
                verticalAlignment: Text.AlignVCenter
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: appCore.themeManager.divider
        }

        // Members label
        StyledText {
            text: "成员 (" + memberListView.count + "人)"
            font.pixelSize: 14
            font.bold: true
            color: appCore.themeManager.textPrimary
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
                    color: index % 2 === 0 ? appCore.themeManager.surface : appCore.themeManager.surfaceAlt
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

                    StyledText {
                        text: modelData.nickname || modelData.username || ""
                        font.pixelSize: 14
                        color: appCore.themeManager.textPrimary
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    // Role badge
                    Rectangle {
                        visible: modelData.role === 1
                        Layout.preferredWidth: 36
                        Layout.preferredHeight: 18
                        radius: 3
                        color: appCore.themeManager.warning

                        StyledText {
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
                        color: appCore.themeManager.divider

                        StyledText {
                            text: "成员"
                            font.pixelSize: 10
                            color: appCore.themeManager.textTertiary
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
                            color: removeBtn.hovered ? appCore.themeManager.dangerLight : appCore.themeManager.dangerLight
                            border.color: appCore.themeManager.dangerLight
                            border.width: 1
                        }

                        contentItem: StyledText {
                            text: removeBtn.text
                            font.pixelSize: 11
                            color: appCore.themeManager.danger
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
                    color: addMemberBtn.hovered ? appCore.themeManager.primaryLight : appCore.themeManager.primaryLight
                    border.color: appCore.themeManager.primary
                    border.width: 1
                }

                contentItem: StyledText {
                    text: addMemberBtn.text
                    font.pixelSize: 13
                                color: appCore.themeManager.primary
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
                    color: transferBtn.hovered ? appCore.themeManager.warningLight : appCore.themeManager.warningLight
                    border.color: appCore.themeManager.warning
                    border.width: 1
                }

                contentItem: StyledText {
                    text: transferBtn.text
                    font.pixelSize: 13
                    color: appCore.themeManager.warning
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
                    color: dismissBtn.hovered ? appCore.themeManager.danger : appCore.themeManager.dangerLight
                }

                contentItem: StyledText {
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
                color: leaveBtn.hovered ? appCore.themeManager.danger : appCore.themeManager.dangerLight
            }

            contentItem: StyledText {
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
            border.color: appCore.themeManager.borderColor
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            StyledText {
                text: "确定要解散该群组吗？"
                font.pixelSize: 14
                color: appCore.themeManager.textPrimary
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            StyledText {
                text: "此操作不可撤销"
                font.pixelSize: 12
                color: appCore.themeManager.danger
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
                        color: cancelDismissBtn.hovered ? appCore.themeManager.divider : appCore.themeManager.surfaceAlt
                        border.color: appCore.themeManager.borderColor
                        border.width: 1
                    }

                    contentItem: StyledText {
                        text: cancelDismissBtn.text
                        font.pixelSize: 13
                        color: appCore.themeManager.textSecondary
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
                        color: confirmDismissBtn.hovered ? appCore.themeManager.danger : appCore.themeManager.dangerLight
                    }

                    contentItem: StyledText {
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
            border.color: appCore.themeManager.borderColor
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            StyledText {
                text: "确定要退出该群组吗？"
                font.pixelSize: 14
                color: appCore.themeManager.textPrimary
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
                        color: cancelLeaveBtn.hovered ? appCore.themeManager.divider : appCore.themeManager.surfaceAlt
                        border.color: appCore.themeManager.borderColor
                        border.width: 1
                    }

                    contentItem: StyledText {
                        text: cancelLeaveBtn.text
                        font.pixelSize: 13
                        color: appCore.themeManager.textSecondary
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
                        color: confirmLeaveBtn.hovered ? appCore.themeManager.danger : appCore.themeManager.dangerLight
                    }

                    contentItem: StyledText {
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
            border.color: appCore.themeManager.borderColor
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 14

            StyledText {
                text: "添加成员"
                font.pixelSize: 18
                font.bold: true
                color: appCore.themeManager.textPrimary
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
                    color: index % 2 === 0 ? appCore.themeManager.surface : appCore.themeManager.surfaceAlt
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

                        StyledText {
                            text: model.nickname || model.username
                            font.pixelSize: 14
                            color: appCore.themeManager.textPrimary
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
                                color: addBtn.hovered ? appCore.themeManager.primaryLight : appCore.themeManager.primaryLight
                                border.color: appCore.themeManager.primary
                                border.width: 1
                            }

                            contentItem: StyledText {
                                text: addBtn.text
                                font.pixelSize: 11
                    color: appCore.themeManager.primary
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
                    color: closeAddBtn.hovered ? appCore.themeManager.divider : appCore.themeManager.surfaceAlt
                    border.color: appCore.themeManager.borderColor
                    border.width: 1
                }

                contentItem: StyledText {
                    text: closeAddBtn.text
                    font.pixelSize: 14
                    color: appCore.themeManager.textSecondary
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
            border.color: appCore.themeManager.borderColor
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 14

            StyledText {
                text: "转让群主"
                font.pixelSize: 18
                font.bold: true
                color: appCore.themeManager.textPrimary
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
                    color: index % 2 === 0 ? appCore.themeManager.surface : appCore.themeManager.surfaceAlt
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

                        StyledText {
                            text: modelData.nickname || modelData.username || ""
                            font.pixelSize: 14
                            color: appCore.themeManager.textPrimary
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
                                color: transferBtn2.hovered ? appCore.themeManager.warningLight : appCore.themeManager.warningLight
                                border.color: appCore.themeManager.warning
                                border.width: 1
                            }

                            contentItem: StyledText {
                                text: transferBtn2.text
                                font.pixelSize: 11
                                color: appCore.themeManager.warning
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
                    color: cancelTransferBtn.hovered ? appCore.themeManager.divider : appCore.themeManager.surfaceAlt
                    border.color: appCore.themeManager.borderColor
                    border.width: 1
                }

                contentItem: StyledText {
                    text: cancelTransferBtn.text
                    font.pixelSize: 14
                    color: appCore.themeManager.textSecondary
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
