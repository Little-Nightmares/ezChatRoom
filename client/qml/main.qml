import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 800
    height: 600
    visible: true
    title: qsTr("ChatRoom")

    minimumWidth: 420
    minimumHeight: 560

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: Qt.resolvedUrl("pages/LoginPage.qml")
    }
}
