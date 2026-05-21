import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Page {
    id: root
    title: qsTr("FriendRequest")

    property var requests: [
        { displayName: "Bob", subtitle: "bob@example.lan" },
        { displayName: "Carol", subtitle: "carol@example.lan" }
    ]

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        SearchBar {
            Layout.fillWidth: true
            placeholderText: qsTr("Search requests")
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.requests

            delegate: Rectangle {
                width: ListView.view.width
                height: 72
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    spacing: 10

                    ContactItem {
                        Layout.fillWidth: true
                        displayName: modelData.displayName
                        subtitle: modelData.subtitle
                    }

                    Button {
                        text: qsTr("Accept")
                    }

                    Button {
                        text: qsTr("Reject")
                    }
                }
            }
        }
    }
}
