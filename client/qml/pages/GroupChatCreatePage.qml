import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Page {
    id: root
    title: qsTr("GroupChatCreate")

    property var members: [
        { displayName: "Alice", subtitle: "Online", online: true },
        { displayName: "Bob", subtitle: "Away", online: false },
        { displayName: "Carol", subtitle: "Online", online: true }
    ]

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        TextField {
            id: groupNameField
            placeholderText: qsTr("Group name")
            Layout.fillWidth: true
        }

        SearchBar {
            Layout.fillWidth: true
            placeholderText: qsTr("Search contacts")
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.members
            delegate: RowLayout {
                width: ListView.view.width
                height: 58

                CheckBox {}

                ContactItem {
                    Layout.fillWidth: true
                    displayName: modelData.displayName
                    subtitle: modelData.subtitle
                    online: modelData.online
                }
            }
        }

        Button {
            text: qsTr("Create group")
            Layout.fillWidth: true
            enabled: groupNameField.text.trim().length > 0
        }
    }
}
