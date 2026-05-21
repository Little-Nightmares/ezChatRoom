import QtQuick
import QtQuick.Controls

Item {
    id: root

    signal accepted()
    signal rejected()

    function openConfirm(title, message) {
        confirmDialog.title = title
        messageLabel.text = message
        confirmDialog.open()
    }

    Dialog {
        id: confirmDialog
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: root.accepted()
        onRejected: root.rejected()

        Label {
            id: messageLabel
            width: 280
            wrapMode: Text.WordWrap
        }
    }
}
