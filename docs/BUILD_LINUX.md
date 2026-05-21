# ChatRoom Linux 构建指南

本文档介绍如何在 Linux 平台上编译、打包和部署 ChatRoom 项目。

## 目录

- [环境要求](#环境要求)
- [安装依赖](#安装依赖)
- [构建项目](#构建项目)
- [使用 CMake Preset](#使用-cmake-preset)
- [部署与发布](#部署与发布)
- [常见问题与解决方案](#常见问题与解决方案)

---

## 环境要求

| 依赖 | 最低版本 | 说明 |
|------|---------|------|
| CMake | 3.16+ | 构建系统，推荐 3.21+ 以支持 Preset |
| Qt | 6.2+ | GUI 框架，需要 Core、Network、Sql、Quick、Gui 模块 |
| OpenSSL | 1.1.1+ | 加密库，用于通信加密 |
| GCC | 7+ | C++17 编译器，推荐 GCC 9+ |
| Make / Ninja | - | 构建工具 |

### 支持的 Linux 发行版

- Ubuntu 20.04 / 22.04 / 24.04
- Debian 11 / 12
- Fedora 36+
- Arch Linux
- openSUSE Leap 15.4+

---

## 安装依赖

### Ubuntu / Debian

```bash
# 更新软件源
sudo apt update

# 安装基础编译工具
sudo apt install -y build-essential cmake ninja-build pkg-config

# 安装 Qt6
sudo apt install -y qt6-base-dev qt6-declarative-dev libqt6sql6-sqlite

# 安装 OpenSSL 开发库
sudo apt install -y libssl-dev

# (可选) 安装额外的 Qt 模块
sudo apt install -y qt6-tools-dev qt6-tools-dev-tools
```

> **注意：** Ubuntu 22.04 及以上版本默认提供 Qt 6.2+。Ubuntu 20.04 需要通过 PPA 或手动安装 Qt6。

**Ubuntu 20.04 通过 PPA 安装 Qt6：**

```bash
sudo add-apt-repository ppa:okirby/qt6-backports
sudo apt update
sudo apt install -y qt6-base-dev qt6-declarative-dev
```

### Fedora

```bash
# 安装基础编译工具
sudo dnf install -y gcc gcc-c++ cmake ninja-build pkg-config

# 安装 Qt6
sudo dnf install -y qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtbase-sqlite

# 安装 OpenSSL 开发库
sudo dnf install -y openssl-devel
```

### Arch Linux

```bash
# 安装所有依赖
sudo pacman -S --needed base-devel cmake ninja qt6-base qt6-declarative openssl
```

### openSUSE

```bash
# 安装所有依赖
sudo zypper install -y gcc gcc-c++ cmake ninja qt6-base-devel qt6-declarative-devel libopenssl-devel
```

---

## 构建项目

### 快速构建

```bash
# 进入项目根目录
cd ChatRoom

# 创建构建目录
mkdir -p build && cd build

# 配置 CMake
cmake ..

# 编译 (使用所有 CPU 核心)
cmake --build . -j$(nproc)
```

### Debug 构建

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j$(nproc)
```

### Release 构建

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
```

### 使用 Ninja 构建 (推荐)

Ninja 比 Make 更快，推荐使用：

```bash
mkdir -p build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

### 指定 Qt 安装路径

如果 Qt 安装在非标准路径（例如通过 aqtinstall 安装）：

```bash
cmake -DCMAKE_PREFIX_PATH=/opt/Qt/6.5.3/gcc_64 ..
```

或使用环境变量：

```bash
export CMAKE_PREFIX_PATH=/opt/Qt/6.5.3/gcc_64
cmake ..
```

---

## 使用 CMake Preset

项目提供了 CMake Preset 配置，可以简化构建流程：

### 查看可用的 Preset

```bash
cmake --list-presets
```

### 使用 linux-default Preset (Debug)

```bash
cmake --preset linux-default
cmake --build --preset linux-debug -j$(nproc)
```

### 使用 linux-release Preset

```bash
cmake --preset linux-release
cmake --build --preset linux-release -j$(nproc)
```

### 自定义 Preset

可以在 `CMakePresets.json` 中添加自定义配置，或通过命令行覆盖：

```bash
cmake --preset linux-default -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build/linux-default -j$(nproc)
```

---

## 部署与发布

### 使用 linuxdeployqt 收集依赖

`linuxdeployqt` 是 Linux 上的 Qt 应用打包工具，可以自动收集依赖：

```bash
# 安装 linuxdeployqt
wget -O linuxdeployqt https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod +x linuxdeployqt

# 收集客户端依赖
./linuxdeployqt build/client/ChatRoomClient -qmake=$(qmake6 || qmake) -no-translations

# 收集服务端依赖 (不需要 Qt GUI)
# 服务端只需要 Qt6Core、Qt6Network、Qt6Sql
```

### 手动收集依赖

使用 `ldd` 和 `patchelf` 工具手动收集和设置依赖：

```bash
# 安装工具
sudo apt install patchelf

# 创建发布目录
mkdir -p release/ChatRoom-linux-x64/{bin,lib,plugins}

# 复制可执行文件
cp build/server/ChatRoomServer release/ChatRoom-linux-x64/bin/
cp build/client/ChatRoomClient release/ChatRoom-linux-x64/bin/

# 收集 Qt 依赖 (客户端)
for lib in Qt6Core Qt6Gui Qt6Network Qt6Sql Qt6Qml Qt6Quick Qt6OpenGL; do
    cp $(qmake6 -query QT_INSTALL_LIBS)/lib${lib}.so.6 release/ChatRoom-linux-x64/lib/ 2>/dev/null || true
done

# 收集 Qt 插件
cp -r $(qmake6 -query QT_INSTALL_PLUGINS)/platforms release/ChatRoom-linux-x64/plugins/
cp -r $(qmake6 -query QT_INSTALL_PLUGINS)/sqldrivers release/ChatRoom-linux-x64/plugins/
cp -r $(qmake6 -query QT_INSTALL_PLUGINS)/imageformats release/ChatRoom-linux-x64/plugins/

# 收集 QML 模块
cp -r $(qmake6 -query QT_INSTALL_QML)/QtQuick release/ChatRoom-linux-x64/qml/
cp -r $(qmake6 -query QT_INSTALL_QML)/QtQml release/ChatRoom-linux-x64/qml/
cp -r $(qmake6 -query QT_INSTALL_QML)/QtQuick.Controls release/ChatRoom-linux-x64/qml/
cp -r $(qmake6 -query QT_INSTALL_QML)/QtQuick.Layouts release/ChatRoom-linux-x64/qml/

# 收集 OpenSSL 依赖
cp /usr/lib/x86_64-linux-gnu/libssl.so.3 release/ChatRoom-linux-x64/lib/ 2>/dev/null || true
cp /usr/lib/x86_64-linux-gnu/libcrypto.so.3 release/ChatRoom-linux-x64/lib/ 2>/dev/null || true

# 使用 ldd 查找并复制其他缺失的依赖
ldd build/client/ChatRoomClient | grep "=> /" | awk '{print $3}' | while read lib; do
    basename_lib=$(basename "$lib")
    if [[ ! -f "release/ChatRoom-linux-x64/lib/$basename_lib" ]]; then
        cp -v "$lib" release/ChatRoom-linux-x64/lib/
    fi
done

# 设置 RPATH
patchelf --set-rpath '$ORIGIN/../lib' release/ChatRoom-linux-x64/bin/ChatRoomClient
patchelf --set-rpath '$ORIGIN/../lib' release/ChatRoom-linux-x64/bin/ChatRoomServer
```

### 创建启动脚本

**启动服务端：**

```bash
cat > release/ChatRoom-linux-x64/start_server.sh << 'EOF'
#!/bin/bash
DIR="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="${DIR}/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
PORT="${CHATROOM_PORT:-6667}"
echo "Starting ChatRoom Server on port ${PORT}..."
exec "${DIR}/bin/ChatRoomServer" "$@"
EOF
chmod +x release/ChatRoom-linux-x64/start_server.sh
```

**启动客户端：**

```bash
cat > release/ChatRoom-linux-x64/start_client.sh << 'EOF'
#!/bin/bash
DIR="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="${DIR}/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="${DIR}/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"
export QML2_IMPORT_PATH="${DIR}/qml${QML2_IMPORT_PATH:+:$QML2_IMPORT_PATH}"
echo "Starting ChatRoom Client..."
exec "${DIR}/bin/ChatRoomClient" "$@"
EOF
chmod +x release/ChatRoom-linux-x64/start_client.sh
```

### 发布目录结构

```
ChatRoom-linux-x64/
├── bin/
│   ├── ChatRoomServer          # 服务端程序
│   └── ChatRoomClient          # 客户端程序
├── lib/
│   ├── libQt6Core.so.6         # Qt 核心库
│   ├── libQt6Gui.so.6          # Qt GUI 库
│   ├── libQt6Network.so.6      # Qt 网络库
│   ├── libQt6Sql.so.6          # Qt SQL 库
│   ├── libQt6Qml.so.6          # Qt QML 库
│   ├── libQt6Quick.so.6        # Qt Quick 库
│   ├── libssl.so.3             # OpenSSL SSL 库
│   ├── libcrypto.so.3          # OpenSSL 加密库
│   └── ...                     # 其他依赖库
├── plugins/
│   ├── platforms/
│   │   └── libqxcb.so          # XCB 平台插件
│   ├── sqldrivers/
│   │   └── libqsqlite.so       # SQLite 驱动
│   └── imageformats/
│       ├── libqjpeg.so         # JPEG 图片插件
│       ├── libqpng.so          # PNG 图片插件
│       └── libqgif.so          # GIF 图片插件
├── qml/                        # QML 模块
│   ├── QtQuick/
│   ├── QtQml/
│   └── ...
├── start_server.sh             # 启动服务端
└── start_client.sh             # 启动客户端
```

### 创建压缩包

```bash
cd release
tar czf ChatRoom-linux-x64-v1.0.0.tar.gz ChatRoom-linux-x64/
```

### 运行程序

**启动服务端：**

```bash
cd ChatRoom-linux-x64
./start_server.sh
```

或指定端口：

```bash
CHATROOM_PORT=8888 ./start_server.sh
```

**启动客户端：**

```bash
cd ChatRoom-linux-x64
./start_client.sh
```

---

## 常见问题与解决方案

### Q1: CMake 找不到 Qt6

**错误信息：**
```
Could not find a package configuration file provided by "Qt6"
```

**解决方案：**

1. 确认 Qt6 开发包已安装：

```bash
dpkg -l | grep qt6    # Ubuntu/Debian
rpm -qa | grep qt6    # Fedora/RHEL
pacman -Qs qt6        # Arch Linux
```

2. 如果 Qt 安装在非标准路径，设置 `CMAKE_PREFIX_PATH`：

```bash
cmake -DCMAKE_PREFIX_PATH=/opt/Qt/6.5.3/gcc_64 ..
```

3. 如果系统同时安装了 Qt5 和 Qt6，可能需要指定版本：

```bash
cmake -DQt6_DIR=/usr/lib/cmake/Qt6 ..
```

### Q2: CMake 找不到 OpenSSL

**错误信息：**
```
Could not find OpenSSL
```

**解决方案：**

1. 安装 OpenSSL 开发库：

```bash
sudo apt install libssl-dev       # Ubuntu/Debian
sudo dnf install openssl-devel    # Fedora
sudo pacman -S openssl            # Arch Linux
```

2. 如果 OpenSSL 安装在非标准路径：

```bash
cmake -DOPENSSL_ROOT_DIR=/usr/local/ssl ..
```

### Q3: 编译错误 - C++17 特性不支持

**错误信息：**
```
error: 'optional' is not a member of 'std'
```

**解决方案：**

1. 升级 GCC 到 7+ 版本：

```bash
gcc --version
```

2. Ubuntu 18.04 需要手动安装 GCC-9：

```bash
sudo apt install gcc-9 g++-9
cmake -DCMAKE_C_COMPILER=gcc-9 -DCMAKE_CXX_COMPILER=g++-9 ..
```

### Q4: 运行时找不到共享库

**错误信息：**
```
error while loading shared libraries: libQt6Core.so.6: cannot open shared object file
```

**解决方案：**

1. 使用启动脚本（已设置 `LD_LIBRARY_PATH`）
2. 或手动设置：

```bash
export LD_LIBRARY_PATH=/path/to/ChatRoom-linux-x64/lib:$LD_LIBRARY_PATH
```

3. 或设置 `RPATH`（编译时）：

```bash
cmake -DCMAKE_INSTALL_RPATH='$ORIGIN/../lib' ..
```

### Q5: 客户端启动时显示 "Could not find the Qt platform plugin xcb"

**解决方案：**

1. 安装 XCB 相关依赖：

```bash
sudo apt install libxcb-xinerama0 libxcb-cursor0 libxcb-icccm4 \
    libxcb-image0 libxcb-keysyms1 libxcb-randr0 libxcb-render-util0 \
    libxcb-shape0 libxcb-xfixes0 libxcb-xkb1 libxcb-cursor0
```

2. 确认 `plugins/platforms/libqxcb.so` 存在
3. 设置 `QT_PLUGIN_PATH`：

```bash
export QT_PLUGIN_PATH=/path/to/ChatRoom-linux-x64/plugins
```

### Q6: 客户端无法连接服务端

**可能原因：**

1. 防火墙阻止了连接，开放端口：

```bash
sudo ufw allow 6667/tcp
```

2. 服务端未启动，确认服务端正在运行
3. IP 地址或端口配置不正确

### Q7: QML 模块加载失败

**错误信息：**
```
module "QtQuick.Controls" is not installed
```

**解决方案：**

1. 安装 Qt QML 模块：

```bash
sudo apt install qt6-declarative-dev    # Ubuntu/Debian
sudo dnf install qt6-qtdeclarative-devel # Fedora
```

2. 确认 `QML2_IMPORT_PATH` 设置正确：

```bash
export QML2_IMPORT_PATH=/path/to/ChatRoom-linux-x64/qml
```

### Q8: SQLite 驱动未加载

**错误信息：**
```
No QSqlDriverFactory for "QSQLITE"
```

**解决方案：**

1. 安装 SQLite Qt 驱动：

```bash
sudo apt install libqt6sql6-sqlite    # Ubuntu/Debian
sudo dnf install qt6-qtbase-sqlite    # Fedora
```

2. 确认 `plugins/sqldrivers/libqsqlite.so` 存在

### Q9: 在无显示器的服务器上运行服务端

服务端不需要 GUI，可以直接在无显示器的服务器上运行：

```bash
# 服务端不需要 Qt GUI 模块，只需 Qt Core、Network、SQL
./start_server.sh
```

如果编译时遇到 Qt GUI 相关错误，可以设置 `QT_QPA_PLATFORM=offscreen`：

```bash
QT_QPA_PLATFORM=offscreen ./start_server.sh
```

### Q10: 如何创建 AppImage

使用 `linuxdeployqt` 创建 AppImage：

```bash
# 安装 linuxdeployqt 和 linuxdeployqt-plugin-qt
wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod +x linuxdeployqt

# 创建 AppDir
mkdir -p ChatRoomClient.AppDir/usr/bin
cp build/client/ChatRoomClient ChatRoomClient.AppDir/usr/bin/
cp resources/chatroom.png ChatRoomClient.AppDir/  # 应用图标

# 运行 linuxdeployqt
./linuxdeployqt ChatRoomClient.AppDir/usr/bin/ChatRoomClient \
    -qmake=$(qmake6 || qmake) \
    -icon=resources/chatroom.png \
    -desktop=resources/chatroom.desktop \
    -no-translations

# 创建 AppImage
./linuxdeployqt ChatRoomClient.AppDir --appimage-extract
```

---

## 附录

### 使用 aqtinstall 安装 Qt

`aqtinstall` 是一个命令行工具，可以快速安装 Qt：

```bash
# 安装 aqtinstall
pip install aqtinstall

# 安装 Qt 6.5.3
aqt install-qt linux desktop 6.5.3 gcc_64 -m qtbase qtdeclarative qttools qtsql

# 设置 CMAKE_PREFIX_PATH
export CMAKE_PREFIX_PATH=~/Qt/6.5.3/gcc_64
```

### 使用 cqtdeployer 打包

`cqtdeployer` 是另一个 Qt 应用打包工具：

```bash
# 安装
pip install cqtdeployer

# 打包客户端
cqtdeployer -bin build/client/ChatRoomClient -qmake $(qmake6 || qmake)

# 打包服务端
cqtdeployer -bin build/server/ChatRoomServer -qmake $(qmake6 || qmake) -noQt
```

### 相关链接

- Qt 官网：https://www.qt.io/
- Qt Linux 文档：https://doc.qt.io/qt-6/linux.html
- CMake 文档：https://cmake.org/documentation/
- OpenSSL 官网：https://www.openssl.org/
- linuxdeployqt：https://github.com/probonopd/linuxdeployqt
- cqtdeployer：https://github.com/QuasarApp/CQtDeployer
- aqtinstall：https://github.com/miurahr/aqtinstall
