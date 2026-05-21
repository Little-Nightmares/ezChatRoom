import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: qsTr("FileTransfer")
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Close

    property var transfers: []

    ColumnLayout {
        width: 420
        height: 300
        spacing: 10

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.transfers

            delegate: ColumnLayout {
                width: ListView.view.width
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        text: modelData.name || qsTr("File")
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    Label {
                        text: (modelData.progress || 0) + "%"
                        color: "#64748b"
                    }
                }

                ProgressBar {
                    from: 0
                    to: 100
                    value: modelData.progress || 0
                    Layout.fillWidth: true
                }
            }
        }

        Label {
            visible: root.transfers.length === 0
            text: qsTr("No active transfers")
            color: "#64748b"
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
    }
}
