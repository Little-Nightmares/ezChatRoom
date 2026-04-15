import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: root
    title: qsTr("聊天")

    // 顶部工具栏
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: qsTr("← 返回")
                onClicked: stackView.pop()  // 返回上一页
            }
            Text {
                text: qsTr("ChatRoom")
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
            ToolButton {
                text: qsTr("设置")
                onClicked: console.log("打开设置")
            }
        }
    }

    // 聊天内容区域（后续实现）
    Text {
        anchors.centerIn: parent
        text: qsTr("聊天页面 - 待实现")
        font.pixelSize: 16
        color: "#999999"
    }
}
