cmake_minimum_required(VERSION 3.16)
project(ChatRoom VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 查找 Qt6 模块（QML 项目需要这些）
find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2)

# 启用自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 如果你的 QML 文件是单独的 qml.qrc 资源文件，可以这样添加
# 如果没有 qml.qrc 就删掉这行
qt_add_resources(qml_files "qml.qrc")

# 设置 QML 导入路径（很重要！）
# 这样 QML 文件能正确找到组件
# qt_import_qml_plugins()

# 添加子目录
add_subdirectory(client)
add_subdirectory(server)
