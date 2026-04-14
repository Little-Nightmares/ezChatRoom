# main.cpp

## 所属模块
client/src/

## 职责
客户端入口，初始化 QGuiApplication 和 QML 引擎，注册 C++ 类型到 QML，加载主界面。

## 主要流程
1. 创建 QGuiApplication
2. 创建 QQmlApplicationEngine
3. 注册 C++ 类型到 QML（待实现）
4. 加载 main.qml

## 依赖关系
- Qt GUI
- Qt QML
