import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: confirmDialog

    property string title: "Confirm"
    property string message: ""

    signal accepted()
    signal rejected()

    width: 320
    height: 160
    anchors.centerIn: parent
    modal: true
    closePolicy: Popup.NoAutoClose

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        Text {
            text: confirmDialog.title
            font.pixelSize: 16
            font.bold: true
            color: "#333"
        }

        Text {
            text: confirmDialog.message
            font.pixelSize: 13
            color: "#666"
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Item { Layout.fillWidth: true }

            Button {
                text: "Cancel"
                flat: true
                implicitHeight: 34

                onClicked: {
                    confirmDialog.close()
                    confirmDialog.rejected()
                }
            }

            Button {
                text: "Confirm"
                implicitHeight: 34

                background: Rectangle {
                    radius: 4
                    color: "#1890ff"
                    implicitHeight: 34
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    confirmDialog.close()
                    confirmDialog.accepted()
                }
            }
        }
    }
}
