# ChatRoom Windows 构建指南

本文档介绍如何在 Windows 平台上编译、打包和部署 ChatRoom 项目。

## 目录

- [环境要求](#环境要求)
- [安装依赖](#安装依赖)
- [使用构建脚本](#使用构建脚本)
- [手动构建](#手动构建)
- [部署与发布](#部署与发布)
- [常见问题与解决方案](#常见问题与解决方案)

---

## 环境要求

| 依赖 | 最低版本 | 说明 |
|------|---------|------|
| CMake | 3.16+ | 构建系统，推荐 3.21+ 以支持 Preset |
| Qt | 6.2+ | GUI 框架，需要 Core、Network、Sql、Quick、Gui 模块 |
| OpenSSL | 1.1.1+ | 加密库，用于通信加密 |
| 编译器 | - | MSVC 2019+ 或 MinGW-w64 GCC 7+ |

### 操作系统要求

- Windows 10 / Windows 11 (x64)
- Windows Server 2016+

---

## 安装依赖

### 1. 安装 CMake

从官网下载并安装 CMake：

- 下载地址：https://cmake.org/download/
- 安装时勾选 **"Add CMake to the system PATH for all users"**

验证安装：

```cmd
cmake --version
```

### 2. 安装 Qt 6

推荐使用 Qt 在线安装器：

- 下载地址：https://www.qt.io/download-qt-installer
- 安装时选择以下组件：
  - **Qt 6.5.x** (或更高版本)
  - **MinGW 11.2.0 64-bit** (如果使用 MinGW 编译器)
  - **MSVC 2019 64-bit** (如果使用 MSVC 编译器)
  - Qt 模块：Qt Quick、Qt Network、Qt SQL

### 3. 安装 OpenSSL

**方式一：使用 Win64 OpenSSL 安装包**

- 下载地址：https://slproweb.com/products/Win32OpenSSL.html
- 选择 **Win64 OpenSSL v3.x.x** (完整版，非 Light 版)
- 安装到默认路径 `C:\Program Files\OpenSSL-Win64`

**方式二：使用 vcpkg 安装**

```cmd
vcpkg install openssl:x64-windows
vcpkg integrate install
```

### 4. 安装编译器

**MSVC (推荐 Visual Studio 2022)**

- 下载地址：https://visualstudio.microsoft.com/
- 安装时勾选 **"使用 C++ 的桌面开发"** 工作负载

**MinGW-w64**

- 如果使用 Qt 在线安装器，MinGW 会随 Qt 一起安装
- 独立安装：https://www.mingw-w64.org/ 或 https://www.msys2.org/

---

## 使用构建脚本

项目提供了两种 Windows 构建脚本：

### 方式一：批处理脚本 (CMD)

```cmd
cd scripts

REM 使用 MinGW 编译器构建 Release 版本
build_windows.bat mingw release

REM 使用 MSVC 编译器构建 Release 版本
build_windows.bat msvc release

REM 使用 CMake Preset 构建
build_windows.bat --preset mingw release

REM 查看帮助
build_windows.bat --help
```

### 方式二：PowerShell 脚本 (推荐)

```powershell
cd scripts

# 使用 MinGW 编译器构建 Release 版本
.\build_windows.ps1 -Toolchain mingw -BuildType release

# 使用 MSVC 编译器构建 Debug 和 Release 版本
.\build_windows.ps1 -Toolchain msvc -BuildType both

# 自动检测 Qt 和 OpenSSL，并创建 ZIP 压缩包
.\build_windows.ps1 -Toolchain mingw -BuildType release -CreateZip

# 手动指定 Qt 和 OpenSSL 路径
.\build_windows.ps1 -QtDir "C:\Qt\6.5.3\mingw81_64" -OpenSslDir "C:\OpenSSL-Win64" -BuildType release

# 跳过 windeployqt 依赖收集
.\build_windows.ps1 -Toolchain mingw -BuildType release -SkipDeploy
```

### 环境变量

可以通过环境变量指定依赖路径：

```cmd
REM 设置 Qt 安装路径
set QT_DIR=C:\Qt\6.5.3\mingw81_64

REM 设置 OpenSSL 安装路径
set OPENSSL_DIR=C:\OpenSSL-Win64

REM 设置 CMake 搜索路径 (可选)
set CMAKE_PREFIX_PATH=C:\Qt\6.5.3\mingw81_64;C:\OpenSSL-Win64

REM 然后运行构建脚本
scripts\build_windows.bat mingw release
```

---

## 手动构建

如果不使用构建脚本，可以手动执行以下步骤：

### 1. 设置环境变量

```cmd
set QT_DIR=C:\Qt\6.5.3\mingw81_64
set OPENSSL_DIR=C:\OpenSSL-Win64
set CMAKE_PREFIX_PATH=%QT_DIR%;%OPENSSL_DIR%
```

### 2. 使用 CMake Preset 配置

```cmd
REM MinGW
cmake --preset windows-mingw

REM MSVC (需要先运行 vcvars64.bat)
cmake --preset windows-msvc
```

### 3. 或手动配置

```cmd
cmake -G Ninja -S . -B build/windows-release ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_PREFIX_PATH="%CMAKE_PREFIX_PATH%" ^
    -DCMAKE_CXX_STANDARD=17 ^
    -DOPENSSL_ROOT_DIR="%OPENSSL_DIR%"
```

### 4. 编译

```cmd
cmake --build build/windows-release --config Release -- -j %NUMBER_OF_PROCESSORS%
```

### 5. 使用 windeployqt 收集依赖

```cmd
set PATH=%QT_DIR%\bin;%PATH%
windeployqt --release --no-translations build\windows-release\client\ChatRoomClient.exe
```

---

## 部署与发布

### 使用 windeployqt

`windeployqt` 是 Qt 提供的工具，可以自动收集应用程序所需的所有 Qt 依赖：

```cmd
REM 基本用法
windeployqt --release ChatRoomClient.exe

REM 常用选项
windeployqt --release ^
    --no-translations ^
    --no-opengl-sw ^
    --dir deploy ^
    ChatRoomClient.exe
```

**常用 windeployqt 参数：**

| 参数 | 说明 |
|------|------|
| `--release` | 只收集 Release 版本的 DLL |
| `--debug` | 只收集 Debug 版本的 DLL |
| `--no-translations` | 不包含翻译文件 |
| `--no-opengl-sw` | 不包含软件 OpenGL 渲染器 |
| `--dir <path>` | 指定输出目录 |
| `--qmldir <path>` | 指定 QML 导入路径 |

### 发布目录结构

构建脚本会自动创建以下发布目录结构：

```
ChatRoom-windows-x64/
├── bin/
│   ├── ChatRoomServer.exe          # 服务端程序
│   ├── ChatRoomClient.exe          # 客户端程序
│   ├── Qt6Core.dll                 # Qt 核心库
│   ├── Qt6Gui.dll                  # Qt GUI 库
│   ├── Qt6Network.dll              # Qt 网络库
│   ├── Qt6Sql.dll                  # Qt SQL 库
│   ├── Qt6Qml.dll                  # Qt QML 库
│   ├── Qt6Quick.dll                # Qt Quick 库
│   ├── libssl-3-x64.dll            # OpenSSL SSL 库
│   ├── libcrypto-3-x64.dll         # OpenSSL 加密库
│   ├── plugins/
│   │   ├── platforms/
│   │   │   └── qwindows.dll        # Windows 平台插件
│   │   ├── sqldrivers/
│   │   │   └── qsqlite.dll         # SQLite 驱动
│   │   └── imageformats/
│   │       ├── qjpeg.dll           # JPEG 图片插件
│   │       ├── qpng.dll            # PNG 图片插件
│   │       └── qgif.dll            # GIF 图片插件
│   └── qml/                        # QML 模块
│       ├── QtQuick/
│       ├── QtQml/
│       └── ...
├── start_server.bat                # 启动服务端
└── start_client.bat                # 启动客户端
```

### 运行程序

**启动服务端：**

双击 `start_server.bat` 或在命令行中运行：

```cmd
cd ChatRoom-windows-x64
start_server.bat
```

**启动客户端：**

双击 `start_client.bat` 或在命令行中运行：

```cmd
cd ChatRoom-windows-x64
start_client.bat
```

### 创建安装包

可以使用 PowerShell 脚本自动创建 ZIP 压缩包：

```powershell
.\build_windows.ps1 -Toolchain mingw -BuildType release -CreateZip
```

压缩包将生成在 `release/ChatRoom-windows-x64.zip`。

---

## 常见问题与解决方案

### Q1: CMake 找不到 Qt6

**错误信息：**
```
Could not find a package configuration file provided by "Qt6"
```

**解决方案：**

1. 确认 Qt 已正确安装
2. 设置 `QT_DIR` 环境变量指向 Qt 安装目录（包含 `bin/qmake.exe` 的目录）
3. 或设置 `CMAKE_PREFIX_PATH` 环境变量

```cmd
set QT_DIR=C:\Qt\6.5.3\mingw81_64
set CMAKE_PREFIX_PATH=%QT_DIR%
```

### Q2: CMake 找不到 OpenSSL

**错误信息：**
```
Could not find OpenSSL, try to set the path to OpenSSL root folder
```

**解决方案：**

1. 确认 OpenSSL 已正确安装
2. 设置 `OPENSSL_DIR` 环境变量

```cmd
set OPENSSL_DIR=C:\OpenSSL-Win64
```

3. 如果使用 vcpkg 安装的 OpenSSL：

```cmd
set CMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH%;C:\vcpkg\installed\x64-windows
```

### Q3: MSVC 编译时找不到 cl.exe

**错误信息：**
```
未找到 MSVC 编译器 (cl.exe)
```

**解决方案：**

1. 打开 **"x64 Native Tools Command Prompt for VS 2022"**（开始菜单中搜索）
2. 在该命令行中运行构建脚本
3. 或确保 Visual Studio 的 C++ 工作负载已安装

### Q4: MinGW 编译时找不到 g++.exe

**解决方案：**

1. 如果使用 Qt 安装器安装了 MinGW，确保 MinGW 的 `bin` 目录在 PATH 中
2. Qt 自带的 MinGW 通常位于：`C:\Qt\Tools\mingw1120_64\bin`
3. 或手动安装 MinGW-w64：https://www.mingw-w64.org/

### Q5: 运行时提示缺少 DLL

**错误信息：**
```
找不到 Qt6Core.dll
找不到 Qt6Gui.dll
```

**解决方案：**

1. 重新运行 `windeployqt` 收集依赖
2. 确保 Qt 的 `bin` 目录在系统 PATH 中
3. 或将所需的 DLL 复制到可执行文件所在目录

### Q6: 客户端启动后闪退

**可能原因：**

1. 缺少 Qt 平台插件 `qwindows.dll`，确保 `plugins/platforms/` 目录存在
2. 缺少 QML 模块，确保 `qml/` 目录存在且完整
3. 缺少 SQLite 驱动，确保 `plugins/sqldrivers/qsqlite.dll` 存在

**调试方法：**

设置环境变量查看详细错误信息：

```cmd
set QT_DEBUG_PLUGINS=1
ChatRoomClient.exe
```

### Q7: OpenSSL 版本冲突

**错误信息：**
```
OpenSSL SSL_connect: SSL_ERROR_SYSCALL
```

**解决方案：**

1. 确保客户端和服务端使用相同版本的 OpenSSL
2. 检查系统中是否有多个版本的 OpenSSL DLL
3. 将正确版本的 OpenSSL DLL 放在应用程序的 `bin` 目录中

### Q8: 路径中包含空格导致构建失败

**解决方案：**

1. 避免在路径中使用空格和中文字符
2. 如果路径包含空格，在 CMake 命令中使用引号包裹路径
3. 构建脚本已处理路径中的空格问题

### Q9: windeployqt 收集的依赖不完整

**解决方案：**

1. 手动指定 QML 目录：

```cmd
windeployqt --release --qmldir <qml_source_dir> ChatRoomClient.exe
```

2. 手动复制缺失的 QML 模块到 `qml/` 目录
3. 检查 `plugins/` 目录下是否包含所有需要的插件

### Q10: 如何交叉编译 Windows 版本

在 Linux 上可以使用 MXE 进行交叉编译：

```bash
# 安装 MXE
git clone https://github.com/mxe/mxe.git
cd mxe

# 安装依赖
make qt6-base qt6-declarative qt6-sql openssl

# 运行打包脚本
./scripts/package_windows.sh --mxe /path/to/mxe
```

---

## 附录

### 推荐的 Qt 安装配置

| 组件 | 用途 | 是否必需 |
|------|------|---------|
| Qt 6.5.x MinGW 64-bit | MinGW 编译 | 二选一 |
| Qt 6.5.x MSVC 2019 64-bit | MSVC 编译 | 二选一 |
| MinGW 11.2.0 64-bit | MinGW 编译器 | MinGW 编译时必需 |
| Qt Quick | QML UI 框架 | 必需 |
| Qt Network | 网络通信 | 必需 |
| Qt SQL | 数据库 | 必需 |
| Qt ShaderTools | 着色器工具 | Qt 6.5+ 推荐 |

### 相关链接

- Qt 官网：https://www.qt.io/
- Qt 文档：https://doc.qt.io/
- CMake 文档：https://cmake.org/documentation/
- OpenSSL 官网：https://www.openssl.org/
- MinGW-w64：https://www.mingw-w64.org/
- MXE 交叉编译：https://mxe.cc/
