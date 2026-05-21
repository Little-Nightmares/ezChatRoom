import QtQuick
import QtQuick.Controls

Dialog {
    id: root
    title: qsTr("Confirm")
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Ok | Dialog.Cancel

    property string message: ""

    Label {
        width: 300
        text: root.message
        wrapMode: Text.WordWrap
    }
}
