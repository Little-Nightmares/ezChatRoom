import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import ChatRoom

Rectangle {
    id: inputBar

    signal sendMessage(string text)

    // Fixed height like QQ
    property int barHeight: 56
    height: barHeight
    color: "white"

    //
    // @mention autocomplete state
    //
    property bool atMentionActive: false
    property int atCursorPos: -1
    property string atFilter: ""

    function isInGroupChat() {
        return appCore.chatController && appCore.chatController.isInGroupChat
    }

    function getMemberNames() {
        if (!isInGroupChat()) return []
        return appCore.groupController.currentGroupMemberNicknames || []
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        spacing: 8

        // Input area with border - fixed size, scrollable inside
        Rectangle {
            id: inputRect
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 8
            color: appCore.themeManager.surfaceAlt
            border.color: appCore.themeManager.borderColor
            border.width: 1
            clip: true

            Flickable {
                id: flickable
                anchors.fill: parent
                anchors.margins: 8
                contentWidth: width
                contentHeight: editArea.implicitHeight
                boundsBehavior: Flickable.StopAtBounds

                TextEdit {
                    id: editArea
                    width: flickable.width
                    wrapMode: TextEdit.Wrap
                    font.pixelSize: 14
                    font.family: appCore.themeManager.fontFamily
                    color: appCore.themeManager.textPrimary
                    selectByMouse: true
                    textFormat: TextEdit.PlainText

                    StyledText {
                        anchors.fill: parent
                        text: qsTr("输入消息...")
                        color: appCore.themeManager.textTertiary
                        font.pixelSize: 14
                        visible: parent.text.length === 0 && !parent.activeFocus
                    }

                    Keys.onReturnPressed: function(event) {
                        if (!(event.modifiers & Qt.ShiftModifier)) {
                            event.accepted = true
                            if (text.trim().length > 0) {
                                inputBar.sendMessage(text.trim())
                                text = ""
                                atMentionActive = false
                            }
                        }
                    }

                    onTextChanged: {
                        flickable.contentY = Math.max(0, contentHeight - flickable.height)
                        updateAtMention()
                    }

                    onCursorPositionChanged: {
                        if (atMentionActive) {
                            updateAtFilter()
                        }
                    }

                    function updateAtMention() {
                        var txt = editArea.text
                        var pos = editArea.cursorPosition

                        // Find the last '@' before cursor and after any whitespace/start
                        if (pos <= 0) {
                            atMentionActive = false
                            return
                        }

                        // Scan backwards from cursor to find '@' 
                        var foundAt = -1
                        for (var i = pos - 1; i >= 0; i--) {
                            var ch = txt.charAt(i)
                            if (ch === '@') {
                                foundAt = i
                                break
                            }
                            if (ch === ' ' || ch === '\n') {
                                break // whitespace before @ means no active mention
                            }
                        }

                        if (foundAt >= 0) {
                            // Check nothing between @ and cursor except valid name chars
                            var afterAt = txt.substring(foundAt + 1, pos)
                            // Only allow alphanumeric, Chinese chars, underscore, hyphen
                            atMentionActive = true
                            atCursorPos = foundAt
                            atFilter = afterAt
                        } else {
                            atMentionActive = false
                            atFilter = ""
                        }
                    }

                    function updateAtFilter() {
                        var txt = editArea.text
                        var pos = editArea.cursorPosition
                        if (atCursorPos >= 0 && pos > atCursorPos) {
                            atFilter = txt.substring(atCursorPos + 1, pos)
                        } else {
                            atMentionActive = false
                            atFilter = ""
                        }
                    }
                }
            }

            // Fade indicator at top when scrolled
            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 12
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#f5f5f5" }
                    GradientStop { position: 1.0; color: "transparent" }
                }
                visible: flickable.contentY > 2
            }

            // @mention autocomplete popup (anchored inside input area)
            Popup {
                id: atPopup
                width: Math.min(200, inputRect.width - 16)
                height: Math.min(200, atListView.contentHeight + 16)
                x: 8
                y: -height - 4
                modal: false
                closePolicy: Popup.NoAutoClose
                visible: atMentionActive && filteredMembers.length > 0

                property var allMembers: inputBar.getMemberNames()

                property var filteredMembers: {
                    if (!atMentionActive) return []
                    var filter = atFilter.toLowerCase()
                    var result = []
                    var seen = {}
                    for (var i = 0; i < allMembers.length; i++) {
                        var name = allMembers[i]
                        if (!name || seen[name]) continue
                        seen[name] = true
                        if (filter.length === 0 || name.toLowerCase().indexOf(filter) >= 0) {
                            result.push(name)
                        }
                    }
                    // Always include "bot" for @bot mention
                    if (filter.length === 0 || "bot".indexOf(filter) >= 0) {
                        if (!seen["bot"]) {
                            result.push("bot")
                            seen["bot"] = true
                        }
                    }
                    return result
                }

                background: Rectangle {
                    radius: 8
                    color: "white"
                    border.color: appCore.themeManager.borderColor
                    border.width: 1
                }

                contentItem: ListView {
                    id: atListView
                    clip: true
                    spacing: 0
                    model: atPopup.filteredMembers

                    delegate: ItemDelegate {
                        width: ListView.view.width
                        height: 36

                        contentItem: StyledText {
                            text: "@" + modelData
                            font.pixelSize: 13
                            color: "#333"
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 10
                        }

                        background: Rectangle {
                            color: hovered ? "#e6f7ff" : "transparent"
                        }

                        onClicked: {
                            // Replace from @ to cursor with @nickname
                            var txt = editArea.text
                            var before = txt.substring(0, atCursorPos)
                            var after = txt.substring(editArea.cursorPosition)
                            var insert = "@" + modelData + " "
                            editArea.text = before + insert + after
                            editArea.cursorPosition = (before + insert).length
                            editArea.forceActiveFocus()
                            atMentionActive = false
                        }
                    }

                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }
                }
            }
        }

        // Emoji button
        Button {
            id: emojiButton
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignVCenter
            flat: true

            contentItem: StyledText {
                text: "😊"
                font.pixelSize: 20
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                emojiPicker.open()
            }
        }

        // File attach button
        Button {
            id: attachButton
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignVCenter
            flat: true

            contentItem: StyledText {
                text: "📎"
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: {
                filePickerDialog.open()
            }
        }

        // Send button - fixed square with text
        Button {
            id: sendButton
            Layout.preferredWidth: 72
            Layout.preferredHeight: 36
            Layout.alignment: Qt.AlignVCenter
            enabled: editArea.text.trim().length > 0

            background: Rectangle {
                radius: 6
                color: sendButton.hovered
                    ? (sendButton.enabled ? appCore.themeManager.primaryLight : appCore.themeManager.borderColor)
                    : (sendButton.enabled ? appCore.themeManager.primary : appCore.themeManager.borderColor)
                Behavior on color { ColorAnimation { duration: 150 } }
            }

            contentItem: StyledText {
                text: "发送~biu"
                font.pixelSize: 13
                font.bold: true
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            ToolTip.visible: hovered
            ToolTip.text: "发送 (Enter)"
            ToolTip.delay: 500

            onClicked: {
                if (editArea.text.trim().length > 0) {
                    inputBar.sendMessage(editArea.text.trim())
                    editArea.text = ""
                    atMentionActive = false
                }
            }
        }
    }

    // Top border
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: appCore.themeManager.borderColor
    }

    // Emoji picker popup
    Popup {
        id: emojiPicker
        width: 336
        height: 260
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        // Position above the emoji button
        x: emojiButton.x - 150
        y: -height - 12

        background: Rectangle {
            radius: 10
            color: appCore.themeManager.surface
            border.color: appCore.themeManager.borderColor
            border.width: 1
        }

        property var emojiData: [
            "😀","😃","😄","😁","😆","😅","🤣","😂","🙂","😊","😇","🥰","😍","🤩","😘","😗","😚","😋","😛","😜","🤪","😝","🤑","🤗","🤭","🤫","🤔","🤐","🤨","😐","😑","😶","😏","😒","🙄","😬","🤥","😌","😔","😪","🤤","😴","😷","🤒","🤕","🤢","🤮","🥴","😵","🤯","🥳","🥺","😢","😭","😤","😠","😡","🤬","💀",
            "👋","🤚","🖐","✋","🖖","👌","🤌","🤏","✌️","🤞","🤟","🤘","🤙","👍","👎","✊","👊","🤛","🤜","👏","🙌","👐","🤲","🤝","🙏","💪",
            "❤️","🧡","💛","💚","💙","💜","🖤","💔","💕","💞","💗","💖","💘","💝","✨","🔥","💫","💦","💤"
        ]

        GridView {
            anchors.fill: parent
            anchors.margins: 8
            model: emojiPicker.emojiData
            cellWidth: 40
            cellHeight: 40
            clip: true

            delegate: Item {
                width: GridView.view.cellWidth
                height: GridView.view.cellHeight

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 2
                    radius: 6
                    color: ma.containsMouse ? appCore.themeManager.surfaceAlt : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 22
                    }

                    MouseArea {
                        id: ma
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            editArea.insert(editArea.cursorPosition, modelData)
                            editArea.forceActiveFocus()
                            emojiPicker.close()
                        }
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }
        }
    }

    // File picker dialog
    FileDialog {
        id: filePickerDialog
        title: "选择文件"
        onAccepted: {
            var fileUrl = selectedFile.toString()
            var filePath = decodeURIComponent(fileUrl)
            if (Qt.platform.os === "windows") {
                filePath = filePath.replace(/^(file:\/\/\/)/, "")
            } else {
                filePath = filePath.replace(/^(file:\/\/)/, "")
            }
            appCore.chatController.sendFile(filePath)
        }
    }
}
