# SettingsPage.qml

## 所属模块
client/qml/pages/

## 职责
设置页面，应用配置、主题切换、账号管理，支持全局主题切换。

## 组件结构
- 类型: `Page`
- 属性: title
- 背景: 使用全局主题的背景色

## 子组件
- ToolBar: 顶部工具栏，包含返回按钮
  - ToolButton (返回): 透明背景，使用全局主题的文本色
  - Text: 页面标题，使用全局主题的文本色
- ColumnLayout: 设置内容容器
  - GroupBox (应用配置): 应用配置区域，使用全局主题的颜色
  - GroupBox (主题切换): 主题切换区域，使用全局主题的颜色
    - ComboBox: 主题选择器，使用全局主题的颜色
  - GroupBox (账号管理): 账号管理区域，使用全局主题的颜色
    - Button: 注销按钮，使用全局主题的按钮颜色

## 信号与接口
- stackView.pop(): 返回上一页
- themeManager.applyTheme(themeIndex): 应用主题

## 依赖关系
- QtQuick
- QtQuick.Controls
- QtQuick.Layouts
