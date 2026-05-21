import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    property var tabs: [qsTr("Chats"), qsTr("Contacts"), qsTr("Profile")]
    property int currentIndex: 0
    signal tabActivated(int index)

    implicitHeight: 48

    RowLayout {
        anchors.fill: parent
        spacing: 4

        Repeater {
            model: root.tabs

            Button {
                text: modelData
                checkable: true
                checked: index === root.currentIndex
                Layout.fillWidth: true
                Layout.fillHeight: true
                onClicked: {
                    root.currentIndex = index
                    root.tabActivated(index)
                }
            }
        }
    }
}
