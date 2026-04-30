# ChatPage.qml

## 所属模块
client/qml/pages/

## 职责
主聊天页面，包含会话列表、聊天区域、用户信息面板，支持主题切换。

## 组件结构
- 类型: `Page`
- 属性: title
- 背景: 使用全局主题的背景色

## 子组件
- ToolBar: 顶部工具栏，包含返回按钮和设置按钮
  - ToolButton (返回): 透明背景，使用全局主题的文本色
  - Text: 页面标题，使用全局主题的文本色
  - ToolButton (设置): 透明背景，使用全局主题的文本色
- Text: 聊天页面占位文本

## 信号与接口
- stackView.pop(): 返回上一页
- stackView.push("qrc:/qml/pages/SettingsPage.qml"): 打开设置页面

## 依赖关系
- QtQuick
- QtQuick.Controls
- QtQuick.Layouts
