// ==================== Qt Quick模块导入 ====================
// 导入Qt Quick核心模块
import QtQuick
// 导入Qt Quick Controls模块
import QtQuick.Controls
// 导入Qt Quick Layouts模块
import QtQuick.Layouts

// ==================== 输入栏根元素 ====================
// 用户输入消息和发送消息的输入栏组件
Item {
    id: root  // 组件唯一标识符

    // ==================== 属性定义 ====================
    // 输入框中的文本内容
    property string text: ""

    // ==================== 信号定义 ====================
    // 发送消息信号，当用户点击发送按钮或按回车键时发出
    signal sendMessage(string text)

    // ==================== 尺寸设置 ====================
    width: parent.width  // 宽度继承自父元素
    height: 50  // 固定高度50像素

    // ==================== 布局容器 ====================
    RowLayout {
        anchors.fill: parent  // 填充父元素
        anchors.margins: 10  // 外边距10像素，避免贴边
        anchors.leftMargin: 20  // 左侧额外间距
        anchors.rightMargin: 20  // 右侧额外间距
        spacing: 8  // 元素间距8像素

        // ==================== 文本输入框 ====================
        TextField {
            id: inputField  // 输入框ID
            Layout.fillWidth: true  // 宽度根据屏幕大小变化
            Layout.fillHeight: true  // 高度根据屏幕大小变化
            placeholderText: qsTr("输入消息...")  // 占位提示文本
            font.pixelSize: Math.max(Math.min(parent.width * 0.03, 12), 10) // 相对字体大小，最小10像素，最大12像素

            // 背景颜色使用主题表面色
            background: Rectangle {
                color: themeManager.currentTheme.surface
                radius: 8  // 圆角8像素
                border.color: themeManager.currentTheme.border
                border.width: 1
            }

            // 输入框样式
            color: themeManager.currentTheme.text  // 文字颜色
            padding: 8  // 内边距8像素

            // 按回车键发送消息
            onAccepted: {
                if (inputField.text.trim() !== "") {  // 如果文本不为空
                    root.text = inputField.text  // 更新属性
                    sendMessage(inputField.text)  // 发出发送信号
                    inputField.text = ""  // 清空输入框
                }
            }
        }

        // ==================== 发送按钮 ====================
        Button {
            id: sendButton  // 按钮ID
            Layout.fillHeight: true  // 高度占据所有可用空间
            width: 70  // 固定宽度70像素

            text: qsTr("发送")  // 按钮文本

            // 按钮样式
            background: Rectangle {
                color: themeManager.currentTheme.button
                radius: 8  // 圆角8像素
            }

            contentItem: Text {
                text: parent.text  // 显示按钮文本
                color: themeManager.currentTheme.buttonText  // 文字颜色
                horizontalAlignment: Text.AlignHCenter  // 水平居中
                verticalAlignment: Text.AlignVCenter  // 垂直居中
            }

            // 点击发送按钮
            onClicked: {
                if (inputField.text.trim() !== "") {  // 如果文本不为空
                    root.text = inputField.text  // 更新属性
                    sendMessage(inputField.text)  // 发出发送信号
                    inputField.text = ""  // 清空输入框
                }
            }
        }
    }
}
