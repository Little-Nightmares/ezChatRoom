import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: Screen.width * 0.8 // 窗口宽度为屏幕宽度的80%
    height: Screen.height * 0.8 // 窗口高度为屏幕高度的80%
    visible: true
    title: qsTr("ChatRoom")
    // 自适应窗口大小
    minimumWidth: 800 // 最小宽度800像素
    minimumHeight: 600 // 最小高度600像素

    // StackView：页面导航管理器，类似手机的页面栈
    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: "qrc:/qml/pages/LoginPage.qml"  // 初始页面：登录页
        // 添加淡入淡出过渡动画
        pushEnter: Transition {
            PropertyAnimation {
                properties: "opacity"
                from: 0
                to: 1
                duration: 300
            }
        }
        pushExit: Transition {
            PropertyAnimation {
                properties: "opacity"
                from: 1
                to: 0
                duration: 300
            }
        }
        popEnter: Transition {
            PropertyAnimation {
                properties: "opacity"
                from: 0
                to: 1
                duration: 300
            }
        }
        popExit: Transition {
            PropertyAnimation {
                properties: "opacity"
                from: 1
                to: 0
                duration: 300
            }
        }
    }
}
