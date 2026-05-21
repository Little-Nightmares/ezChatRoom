import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property alias text: editor.text
    signal sendRequested(string text)
    signal fileRequested()

    implicitHeight: 58

    function submit() {
        var value = editor.text.trim()
        if (value.length === 0)
            return
        root.sendRequested(value)
        editor.text = ""
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Button {
            text: "+"
            Layout.preferredWidth: 42
            Layout.preferredHeight: 42
            onClicked: root.fileRequested()
        }

        TextArea {
            id: editor
            placeholderText: qsTr("Message")
            wrapMode: TextArea.Wrap
            Layout.fillWidth: true
            Layout.preferredHeight: 42
            Keys.onReturnPressed: {
                if (!(event.modifiers & Qt.ShiftModifier)) {
                    event.accepted = true
                    root.submit()
                }
            }
        }

        Button {
            text: qsTr("Send")
            Layout.preferredWidth: 72
            Layout.preferredHeight: 42
            onClicked: root.submit()
        }
    }
}
