import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ChatRoom

Page {
    id: chatPage

    property bool searchVisible: false
    property bool loadingMessages: false
    property bool chatActive: appCore.chatController.currentChatFriendId !== 0 || appCore.chatController.isInGroupChat

    signal logout()

    background: Rectangle {
        color: "#f0f2f5"
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left sidebar - Conversation list
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 300
            color: "white"

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
                        spacing: 8

                        Text {
                            text: appCore.sessionManager.nickname || "ChatRoom"
                            font.pixelSize: 18
                            font.bold: true
                            color: "white"
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }

                        // Friend request button
                        Button {
                            id: friendReqBtn
                            flat: true
                            text: " + "
                            font.pixelSize: 20
                            font.bold: true

                            contentItem: Text {
                                text: friendReqBtn.text
                                font: friendReqBtn.font
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                radius: 4
                                color: friendReqBtn.hovered ? "#40a9ff" : "transparent"
                            }

                            onClicked: {
                                appCore.friendController.requestFriendRequestList()
                                friendRequestPopup.open()
                            }
                        }

                        // Settings button
                        Button {
                            id: settingsBtn
                            flat: true
                            text: " ⚙ "
                            font.pixelSize: 18

                            contentItem: Text {
                                text: settingsBtn.text
                                font: settingsBtn.font
                                color: "white"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                radius: 4
                                color: settingsBtn.hovered ? "#40a9ff" : "transparent"
                            }

                            onClicked: settingsPopup.open()
                        }
                    }
                }

                // Tab bar: Chats / Contacts
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    color: "#f7f7f7"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 0

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: leftTabBar.currentIndex === 0 ? "white" : "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: "Chats"
                                font.pixelSize: 13
                                font.bold: leftTabBar.currentIndex === 0
                                color: leftTabBar.currentIndex === 0 ? "#1890ff" : "#666"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: leftTabBar.currentIndex = 0
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: leftTabBar.currentIndex === 1 ? "white" : "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: "\u7FA4\u7EC4"
                                font.pixelSize: 13
                                font.bold: leftTabBar.currentIndex === 1
                                color: leftTabBar.currentIndex === 1 ? "#1890ff" : "#666"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: leftTabBar.currentIndex = 1
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: leftTabBar.currentIndex === 2 ? "white" : "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: "Contacts"
                                font.pixelSize: 13
                                font.bold: leftTabBar.currentIndex === 2
                                color: leftTabBar.currentIndex === 2 ? "#1890ff" : "#666"
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: leftTabBar.currentIndex = 2
                            }
                        }
                    }

                    // Bottom border
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 1
                        color: "#e8e8e8"
                    }
                }

                // Search bar
                SearchBar {
                    id: searchFriends
                    Layout.fillWidth: true
                    Layout.margins: 8
                    visible: leftTabBar.currentIndex === 2
                    onSearch: function(keyword) {
                        appCore.friendController.searchUser(keyword)
                    }
                }

                // Stacked list: Conversations / Groups / Contacts
                Item {
                    id: leftTabBar
                    property int currentIndex: 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    // Tab 0: Conversation list
                    ListView {
                        clip: true
                        anchors.fill: parent
                        visible: leftTabBar.currentIndex === 0
                        model: appCore.conversationModel

                        delegate: ConversationItem {
                            width: ListView.view.width
                            friendId: model.friendId
                            nickname: model.nickname
                            avatarUrl: model.avatar
                            lastMessage: model.lastMessage
                            lastTime: model.lastTime
                            unreadCount: model.unreadCount
                            selected: friendId === appCore.chatController.currentChatFriendId

                            onClicked: {
                                leftTabBar.currentIndex = 0
                                loadingMessages = true
                                appCore.chatController.loadChatHistory(friendId)
                                appCore.chatController.currentChatFriendId = friendId
                                appCore.chatController.currentGroupId = 0
                                Qt.callLater(function() { loadingMessages = false })
                            }
                        }

                        ScrollBar.vertical: ScrollBar {}
                    }

                    // Tab 1: Group list
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0
                        visible: leftTabBar.currentIndex === 1

                        // Create group button
                        Button {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            Layout.margins: 8
                            text: "+ \u521B\u5EFA\u7FA4\u7EC4"
                            flat: true

                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 13
                                color: "#1890ff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                radius: 4
                                color: parent.hovered ? "#e6f7ff" : "#f0f9ff"
                                border.color: "#91d5ff"
                                border.width: 1
                            }

                            onClicked: createGroupDialog.open()
                        }

                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: appCore.groupModel

                            delegate: Rectangle {
                                width: ListView.view.width
                                height: 64
                                color: "transparent"

                                property var delegateGroupId: model.groupId
                                property string delegateGroupName: model.groupName

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: 12
                                    anchors.rightMargin: 12
                                    spacing: 10

                                    // Simple avatar placeholder (no Image)
                                    Rectangle {
                                        width: 44
                                        height: 44
                                        radius: 22
                                        color: "#1890ff"
                                        anchors.verticalCenter: parent.verticalCenter

                                        Text {
                                            anchors.centerIn: parent
                                            text: delegateGroupName ? delegateGroupName.charAt(0) : "?"
                                            font.pixelSize: 18
                                            font.bold: true
                                            color: "white"
                                        }
                                    }

                                    Column {
                                        width: parent.width - 64
                                        anchors.verticalCenter: parent.verticalCenter
                                        spacing: 4

                                        Text {
                                            text: delegateGroupName || "未知群组"
                                            font.pixelSize: 14
                                            font.bold: true
                                            color: "#333"
                                            elide: Text.ElideRight
                                            width: parent.width
                                        }

                                        Text {
                                            text: model.lastMessage || ""
                                            font.pixelSize: 12
                                            color: "#999"
                                            elide: Text.ElideRight
                                            width: parent.width
                                        }
                                    }
                                }

                                // Bottom divider
                                Rectangle {
                                    anchors.bottom: parent.bottom
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.leftMargin: 66
                                    height: 1
                                    color: "#f0f0f0"
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        leftTabBar.currentIndex = 1
                                        loadingMessages = true
                                        appCore.chatController.currentGroupId = delegateGroupId
                                        appCore.chatController.currentChatFriendId = 0
                                        appCore.chatController.loadGroupChatHistory(delegateGroupId)
                                        Qt.callLater(function() { loadingMessages = false })
                                    }
                                }
                            }

                            ScrollBar.vertical: ScrollBar {}
                        }
                    }

                    // Tab 2: Contact list (friends)
                    ListView {
                        clip: true
                        anchors.fill: parent
                        visible: leftTabBar.currentIndex === 2
                        model: appCore.friendController.userModel

                        delegate: ContactItem {
                            width: ListView.view.width
                            userId: model.userId
                            nickname: model.nickname || model.username
                            username: model.username
                            avatar: model.avatar
                            isOnline: model.isOnline
                            selected: userId === appCore.chatController.currentChatFriendId

                            onClicked: {
                                loadingMessages = true
                                appCore.chatController.loadChatHistory(userId)
                                appCore.chatController.currentChatFriendId = userId
                                Qt.callLater(function() { loadingMessages = false })
                            }
                        }

                        ScrollBar.vertical: ScrollBar {}
                    }
                }

                // Logout button
                Button {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    text: "Logout"
                    flat: true

                    onClicked: logout()
                }
            }
        }

        // Right side - Chat area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#ebebeb"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Chat header
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 56
                    color: "white"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16

                        Avatar {
                            Layout.preferredWidth: 36
                            Layout.preferredHeight: 36
                            name: appCore.chatController.isInGroupChat
                                   ? appCore.chatController.currentGroupName
                                   : appCore.chatController.currentChatNickname
                        }

                        Column {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: appCore.chatController.isInGroupChat
                                       ? (appCore.chatController.currentGroupName || "Select a conversation")
                                       : (appCore.chatController.currentChatNickname || "Select a conversation")
                                font.pixelSize: 16
                                font.bold: true
                                color: "#333"
                                elide: Text.ElideRight
                                width: parent.width
                            }

                            Text {
                                text: appCore.chatController.isInGroupChat
                                       ? (appCore.groupController.currentGroupMemberCount > 0
                                          ? appCore.groupController.currentGroupMemberCount + " \u4EBA"
                                          : "")
                                       : ""
                                font.pixelSize: 11
                                color: "#999"
                                visible: appCore.chatController.isInGroupChat
                            }
                        }

                        // Group info button (only visible in group chat)
                        Button {
                            visible: appCore.chatController.isInGroupChat
                            flat: true
                            text: "\u2139\uFE0F"
                            font.pixelSize: 16

                            contentItem: Text {
                                text: parent.text
                                font: parent.font
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                radius: 4
                                color: parent.hovered ? "#e6f7ff" : "transparent"
                            }

                            onClicked: groupInfoDialog.open()
                        }

                        // Delete friend button (only visible in private chat)
                        Button {
                            visible: !appCore.chatController.isInGroupChat && appCore.chatController.currentChatFriendId !== 0
                            flat: true
                            text: " \u2716 "
                            font.pixelSize: 14

                            contentItem: Text {
                                text: parent.text
                                font: parent.font
                                color: "#ff4d4f"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            background: Rectangle {
                                radius: 4
                                color: parent.hovered ? "#fff1f0" : "transparent"
                            }

                            onClicked: deleteFriendDialog.open()
                        }

                        // Search button
                        Button {
                            visible: chatActive
                            text: "\uD83D\uDD0D"
                            flat: true
                            font.pixelSize: 14
                            onClicked: { searchVisible = !searchVisible; if (searchVisible) searchInput.forceActiveFocus() }
                        }
                    }
                }

                // Message search bar
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: searchVisible ? 36 : 0
                    color: "#f5f5f5"
                    visible: searchVisible
                    clip: true

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 8

                        TextField {
                            id: searchInput
                            Layout.fillWidth: true
                            placeholderText: "搜索消息..."
                            font.pixelSize: 13
                            onAccepted: appCore.chatController.searchMessages(text)
                            onTextChanged: if (text.isEmpty()) appCore.chatController.clearSearch()
                        }

                        Text {
                            text: "\u2715"
                            font.pixelSize: 16
                            color: "#999"
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    searchInput.text = ""
                                    searchVisible = false
                                }
                            }
                        }
                    }
                }

                // Message list
                ListView {
                    id: messageListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: appCore.messageModel
                    spacing: 8
                    verticalLayoutDirection: ListView.TopToBottom
                    visible: chatActive

                    property bool atBottom: true

                    onCountChanged: {
                        if (atBottom) {
                            Qt.callLater(function() { messageListView.positionViewAtEnd() })
                        }
                    }

                    onContentYChanged: {
                        atBottom = (contentY >= contentHeight - height - 100)
                    }

                    Component.onCompleted: {
                        Qt.callLater(function() { messageListView.positionViewAtEnd() })
                    }

                    delegate: Loader {
                        width: ListView.view.width
                        sourceComponent: appCore.chatController.isInGroupChat ? groupBubbleComponent : privateBubbleComponent
                        property var modelData: model

                        Component {
                            id: privateBubbleComponent
                            MessageBubble {
                                width: parent ? parent.width : ListView.view.width
                                content: modelData.content
                                isMine: modelData.isMine
                                senderName: isMine ? appCore.sessionManager.nickname : appCore.chatController.currentChatNickname
                                timestamp: modelData.timestamp
                                type: modelData.type
                                status: modelData.status
                                messageId: modelData.messageId
                            }
                        }

                        Component {
                            id: groupBubbleComponent
                            GroupMessageBubble {
                                width: parent ? parent.width : ListView.view.width
                                content: modelData.content
                                isMine: modelData.isMine
                                senderNickname: modelData.senderNickname
                                timestamp: modelData.timestamp
                                type: modelData.type
                                status: modelData.status
                                messageId: modelData.messageId
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {}
                }

                // Message list loading indicator
                BusyIndicator {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    z: 10
                    running: loadingMessages
                    visible: loadingMessages && chatActive
                }

                // Empty state when no conversation is selected
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: !chatActive

                    Column {
                        anchors.centerIn: parent
                        spacing: 12

                        Text {
                            text: "\uD83D\uDCAC"
                            font.pixelSize: 48
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "选择一个会话开始聊天"
                            font.pixelSize: 16
                            color: "#999"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "从左侧列表中选择好友，或点击 + 添加新好友"
                            font.pixelSize: 12
                            color: "#bbb"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }

                // Input bar
                InputBar {
                    Layout.fillWidth: true
                    onSendMessage: function(text) {
                        if (appCore.chatController.isInGroupChat) {
                            appCore.chatController.sendGroupMessage(text)
                        } else {
                            appCore.chatController.sendMessage(text)
                        }
                    }
                }
            }
        }
    }

    // Friend request popup
    Popup {
        id: friendRequestPopup
        width: 400
        height: 500
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                color: "#1890ff"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12

                    Text {
                        text: "Friend Requests"
                        font.pixelSize: 16
                        font.bold: true
                        color: "white"
                        Layout.fillWidth: true
                    }

                    Button {
                        flat: true
                        text: "+ Add Friend"
                        font.pixelSize: 12
                        onClicked: addFriendDialog.open()
                    }
                }
            }

            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: appCore.friendRequestModel

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 64
                    color: index % 2 === 0 ? "white" : "#fafafa"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 8

                        Column {
                            Layout.fillWidth: true
                            Text {
                                text: model.fromUsername
                                font.bold: true
                                font.pixelSize: 14
                                color: "#333"
                            }
                            Text {
                                text: model.message
                                font.pixelSize: 12
                                color: "#888"
                                elide: Text.ElideRight
                                width: parent.width
                            }
                        }

                        Button {
                            text: "Accept"
                            implicitHeight: 32
                            visible: model.status === 0

                            onClicked: {
                                appCore.friendController.acceptFriendRequest(model.requestId)
                            }
                        }

                        Button {
                            text: "Reject"
                            implicitHeight: 32
                            visible: model.status === 0
                            flat: true

                            onClicked: {
                                appCore.friendController.rejectFriendRequest(model.requestId)
                            }
                        }

                        Text {
                            text: model.status === 1 ? "Accepted" : "Rejected"
                            font.pixelSize: 12
                            color: model.status === 1 ? "#52c41a" : "#ff4d4f"
                            visible: model.status !== 0
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }
        }
    }

    // Add friend dialog
    AddFriendDialog {
        id: addFriendDialog
        onAddFriend: function(userId, message) {
            appCore.friendController.addFriend(userId, message)
        }
    }

    // Settings popup
    Popup {
        id: settingsPopup
        width: 350
        height: 300
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        SettingsPage {
            anchors.fill: parent
            onLogoutRequested: {
                settingsPopup.close()
                logout()
            }
        }
    }

    // Delete friend confirmation dialog
    Popup {
        id: deleteFriendDialog
        width: 300
        height: 150
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16

            Text {
                text: "Delete this friend?"
                font.pixelSize: 16
                font.bold: true
                color: "#333"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "You will no longer be able to chat with " + appCore.chatController.currentChatNickname
                font.pixelSize: 13
                color: "#666"
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 12

                Button {
                    text: "Cancel"
                    flat: true
                    onClicked: deleteFriendDialog.close()
                }

                Button {
                    text: "Delete"
                    highlighted: true

                    background: Rectangle {
                        radius: 4
                        color: "#ff4d4f"
                        implicitWidth: 80
                        implicitHeight: 32
                    }

                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        var friendId = appCore.chatController.currentChatFriendId
                        appCore.friendController.deleteFriend(friendId)
                        appCore.chatController.clearCurrentChat()
                        deleteFriendDialog.close()
                    }
                }
            }
        }
    }

    Connections {
        target: appCore.friendController
        function onFriendRequestAccepted() {
            appCore.friendController.requestFriendRequestList()
            appCore.friendController.requestFriendList()
        }
        function onFriendRequestRejected() {
            appCore.friendController.requestFriendRequestList()
        }
        function onFriendDeleted() {
            appCore.friendController.requestFriendList()
        }
        function onNewFriendRequestReceived() {
            // Auto-refresh friend request list when a new request arrives
            appCore.friendController.requestFriendRequestList()
            friendRequestPopup.open()
        }
    }

    // Create group dialog
    CreateGroupDialog {
        id: createGroupDialog
        onGroupCreated: function(groupId) {
            createGroupDialog.close()
        }
    }

    // Group info dialog
    GroupInfoDialog {
        id: groupInfoDialog
        groupId: appCore.chatController.currentGroupId
        groupName: appCore.chatController.currentGroupName
        isOwner: appCore.groupController.isGroupOwner
        onGroupDismissed: {
            appCore.chatController.currentGroupId = 0
        }
    }
}
