# main.qml

## 所属模块
client/qml//

## 职责
应用入口，加载初始页面，管理全局状态，提供主题管理系统。

## 组件结构
- 类型: `ApplicationWindow`
- 属性: title, width, height, visible, minimumWidth, minimumHeight

## 子组件
- StackView: 页面导航管理器
- QtObject (themeManager): 全局主题管理系统
- Settings: 主题设置持久化

## 信号与接口
- themeManager.applyTheme(themeIndex): 应用主题
- stackView.push(): 导航到新页面
- stackView.pop(): 返回上一页
- stackView.Popup(): 弹窗组件，用于显示错误信息、确认操作等

## 依赖关系
- QtQuick
- QtQuick.Controls
- Qt.labs.settings 1.0
