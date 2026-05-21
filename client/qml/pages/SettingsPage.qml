import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root
    title: qsTr("Settings")

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        GroupBox {
            title: qsTr("Server")
            Layout.fillWidth: true

            GridLayout {
                columns: 2
                anchors.fill: parent
                rowSpacing: 10
                columnSpacing: 10

                Label { text: qsTr("Host") }
                TextField {
                    text: "127.0.0.1"
                    Layout.fillWidth: true
                }

                Label { text: qsTr("Port") }
                SpinBox {
                    from: 1
                    to: 65535
                    value: 6667
                    editable: true
                    Layout.fillWidth: true
                }
            }
        }

        GroupBox {
            title: qsTr("Preferences")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                CheckBox { text: qsTr("Start minimized") }
                CheckBox { text: qsTr("Play notification sound"); checked: true }
                ComboBox {
                    model: [qsTr("System"), qsTr("Light"), qsTr("Dark")]
                    Layout.fillWidth: true
                }
            }
        }

        Item { Layout.fillHeight: true }

        Button {
            text: qsTr("Save settings")
            Layout.fillWidth: true
        }
    }
}
