import QtQuick
import QtQuick.Controls
import Qt.labs.settings 1.0

ApplicationWindow {
    id: root
    width: Screen.width * 0.8 // 窗口宽度为屏幕宽度的80%
    height: Screen.height * 0.8 // 窗口高度为屏幕高度的80%
    visible: true
    title: qsTr("ChatRoom")
    // 自适应窗口大小
    minimumWidth: 800 // 最小宽度800像素
    minimumHeight: 600 // 最小高度600像素

    // 全局主题管理系统
    QtObject {
        id: themeManager
        
        // 主题颜色定义
        readonly property var lightTheme: ({
            "background": "#ffffff",
            "text": "#1f2937",
            "primary": "#4f46e5",
            "secondary": "#6b7280",
            "border": "#d1d5db",
            "surface": "#f3f4f6",
            "button": "#4f46e5",
            "buttonText": "#ffffff"
        })
        
        readonly property var darkTheme: ({
            "background": "#1f2937",
            "text": "#f9fafb",
            "primary": "#818cf8",
            "secondary": "#9ca3af",
            "border": "#374151",
            "surface": "#111827",
            "button": "#818cf8",
            "buttonText": "#1f2937"
        })
        
        // 樱花主题
        readonly property var sakuraTheme: ({
            "background": "#fff8f8",
            "text": "#6b21a8",
            "primary": "#f9a8d4",
            "secondary": "#d8b4fe",
            "border": "#fecdd3",
            "surface": "#fff1f2",
            "button": "#f472b6",
            "buttonText": "#ffffff"
        })
        
        // 海洋主题
        readonly property var oceanTheme: ({
            "background": "#f0f9ff",
            "text": "#0c4a6e",
            "primary": "#38bdf8",
            "secondary": "#93c5fd",
            "border": "#bae6fd",
            "surface": "#dbeafe",
            "button": "#0ea5e9",
            "buttonText": "#ffffff"
        })
        
        // 森林主题
        readonly property var forestTheme: ({
            "background": "#f0fdf4",
            "text": "#15803d",
            "primary": "#4ade80",
            "secondary": "#a7f3d0",
            "border": "#bbf7d0",
            "surface": "#dcfce7",
            "button": "#22c55e",
            "buttonText": "#ffffff"
        })
        
        // 当前主题索引（0: 浅色, 1: 深色, 2: 樱花, 3: 海洋, 4: 森林, 5: 跟随系统）
        property int currentThemeIndex: 0
        
        // 当前主题颜色
        property var currentTheme: lightTheme
        
        // 应用主题
        function applyTheme(themeIndex) {
            currentThemeIndex = themeIndex
            
            if (themeIndex === 0) {
                // 浅色主题
                currentTheme = lightTheme
            } else if (themeIndex === 1) {
                // 深色主题
                currentTheme = darkTheme
            } else if (themeIndex === 2) {
                // 樱花主题
                currentTheme = sakuraTheme
            } else if (themeIndex === 3) {
                // 海洋主题
                currentTheme = oceanTheme
            } else if (themeIndex === 4) {
                // 森林主题
                currentTheme = forestTheme
            } else {
                // 跟随系统（这里简化为浅色主题）
                currentTheme = lightTheme
            }
            
            // 保存主题设置
            settings.themeIndex = themeIndex
            
            console.log("应用主题: " + (themeIndex === 0 ? "浅色" : themeIndex === 1 ? "深色" : themeIndex === 2 ? "樱花" : themeIndex === 3 ? "海洋" : themeIndex === 4 ? "森林" : "跟随系统"))
        }
        
        // 初始化主题
        Component.onCompleted: {
            applyTheme(settings.themeIndex)
        }
    }
    
    // 主题设置持久化
    Settings {
        id: settings
        category: "Theme"
        property int themeIndex: 0 // 默认浅色主题
    }

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
