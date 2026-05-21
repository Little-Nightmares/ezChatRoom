import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: qsTr("AddFriend")
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Ok | Dialog.Cancel

    property alias account: accountField.text
    property alias message: messageField.text
    signal friendRequested(string account, string message)

    onAccepted: root.friendRequested(accountField.text.trim(), messageField.text.trim())

    ColumnLayout {
        width: 320
        spacing: 12

        TextField {
            id: accountField
            placeholderText: qsTr("Account")
            Layout.fillWidth: true
        }

        TextArea {
            id: messageField
            placeholderText: qsTr("Message")
            wrapMode: TextArea.Wrap
            Layout.fillWidth: true
            Layout.preferredHeight: 84
        }
    }
}
