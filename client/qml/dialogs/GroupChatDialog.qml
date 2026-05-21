import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: qsTr("GroupChat")
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Ok | Dialog.Cancel

    property alias groupName: groupNameField.text
    signal groupRequested(string name)

    onAccepted: root.groupRequested(groupNameField.text.trim())

    ColumnLayout {
        width: 340
        spacing: 12

        TextField {
            id: groupNameField
            placeholderText: qsTr("Group name")
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Members can be added after the group is created.")
            color: "#64748b"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
