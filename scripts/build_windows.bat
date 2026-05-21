@echo off
chcp 65001 >nul 2>&1
setlocal enabledelayedexpansion

REM ============================================================
REM  ChatRoom Windows 构建脚本
REM  支持 MSVC 和 MinGW 编译器
REM  用法: build_windows.bat [msvc|mingw] [debug|release]
REM ============================================================

set "SCRIPT_DIR=%~dp0"
set "PROJECT_DIR=%SCRIPT_DIR%.."

REM 默认参数
set "TOOLCHAIN=mingw"
set "BUILD_TYPE=release"
set "USE_PRESET=0"

REM 解析命令行参数
:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="msvc" set "TOOLCHAIN=msvc"
if /i "%~1"=="mingw" set "TOOLCHAIN=mingw"
if /i "%~1"=="debug" set "BUILD_TYPE=debug"
if /i "%~1"=="release" set "BUILD_TYPE=release"
if /i "%~1"=="--preset" set "USE_PRESET=1"
if /i "%~1"=="--help" goto :usage
if /i "%~1"=="-h" goto :usage
shift
goto :parse_args
:args_done

echo.
echo ========================================================
echo   ChatRoom Windows 构建脚本
echo   编译器: %TOOLCHAIN%
echo   构建类型: %BUILD_TYPE%
echo ========================================================
echo.

REM ============================================================
REM  检查环境变量和工具链
REM ============================================================

REM 检查 CMake
where cmake >nul 2>&1
if errorlevel 1 (
    echo [错误] 未找到 CMake，请安装 CMake 3.16+ 并添加到 PATH
    echo   下载地址: https://cmake.org/download/
    exit /b 1
)
for /f "tokens=2 delims= " %%v in ('cmake --version 2^>nul ^| findstr /r "^[Cc][Mm]ake"') do (
    echo [信息] CMake 版本: %%v
)

REM 检查 Qt6 安装
if not defined QT_DIR (
    echo [信息] QT_DIR 未设置，尝试自动检测...

    REM 常见 Qt 安装路径
    set "QT_SEARCH_PATHS=C:\Qt\6.2.0;C:\Qt\6.2.3;C:\Qt\6.2.4;C:\Qt\6.3.0;C:\Qt\6.3.1;C:\Qt\6.4.0;C:\Qt\6.4.1;C:\Qt\6.4.2;C:\Qt\6.4.3;C:\Qt\6.5.0;C:\Qt\6.5.1;C:\Qt\6.5.2;C:\Qt\6.5.3;C:\Qt\6.6.0;C:\Qt\6.6.1;C:\Qt\6.6.2;C:\Qt\6.6.3;C:\Qt\6.7.0;C:\Qt\6.8.0"

    REM 根据编译器选择子目录
    if /i "%TOOLCHAIN%"=="msvc" (
        set "QT_COMPILER_SUBDIR=msvc2019_64"
        if defined VCToolsInstallDir (
            REM 检测 VS 2022
            set "QT_COMPILER_SUBDIR=msvc2019_64"
        )
    ) else (
        set "QT_COMPILER_SUBDIR=mingw81_64"
    )

    for %%p in (%QT_SEARCH_PATHS%) do (
        if exist "%%p\%%QT_COMPILER_SUBDIR%\bin\qmake.exe" (
            set "QT_DIR=%%p\%%QT_COMPILER_SUBDIR%"
            echo [信息] 找到 Qt: !QT_DIR!
            goto :qt_found
        )
        if exist "%%p\bin\qmake.exe" (
            set "QT_DIR=%%p"
            echo [信息] 找到 Qt: !QT_DIR!
            goto :qt_found
        )
    )

    echo [错误] 未找到 Qt6 安装！
    echo   请设置 QT_DIR 环境变量指向 Qt6 安装目录
    echo   示例: set QT_DIR=C:\Qt\6.5.3\mingw81_64
    echo   下载地址: https://www.qt.io/download-qt-installer
    exit /b 1
)
:qt_found

REM 验证 Qt 目录
if not exist "%QT_DIR%\bin\qmake.exe" (
    echo [错误] Qt 目录无效，未找到 qmake.exe: %QT_DIR%
    exit /b 1
)

REM 检查 OpenSSL
if not defined OPENSSL_DIR (
    echo [信息] OPENSSL_DIR 未设置，尝试自动检测...

    set "OPENSSL_SEARCH_PATHS=C:\OpenSSL;C:\OpenSSL-Win64;C:\Program Files\OpenSSL;C:\Program Files (x86)\OpenSSL;C:\Tools\OpenSSL;C:\Utils\OpenSSL-Win64"

    for %%p in (%OPENSSL_SEARCH_PATHS%) do (
        if exist "%%p\bin\openssl.exe" (
            set "OPENSSL_DIR=%%p"
            echo [信息] 找到 OpenSSL: !OPENSSL_DIR!
            goto :openssl_found
        )
    )

    echo [警告] 未找到 OpenSSL 安装！
    echo   某些加密功能可能不可用
    echo   请设置 OPENSSL_DIR 环境变量
    echo   下载地址: https://slproweb.com/products/Win32OpenSSL.html
    set "OPENSSL_DIR="
)
:openssl_found

REM 设置 CMAKE_PREFIX_PATH
if not defined CMAKE_PREFIX_PATH (
    set "CMAKE_PREFIX_PATH=%QT_DIR%"
) else (
    set "CMAKE_PREFIX_PATH=%QT_DIR%;%CMAKE_PREFIX_PATH%"
)

if defined OPENSSL_DIR (
    set "CMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH%;%OPENSSL_DIR%"
)

echo [信息] CMAKE_PREFIX_PATH: %CMAKE_PREFIX_PATH%

REM ============================================================
REM  配置编译器环境
REM ============================================================

if /i "%TOOLCHAIN%"=="msvc" (
    echo.
    echo [信息] 配置 MSVC 编译器环境...

    REM 尝试使用 vswhere 查找 Visual Studio
    set "VS_WHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "%VS_WHERE%" (
        for /f "usebackq tokens=*" %%i in (`"%VS_WHERE%" -latest -property installationPath`) do (
            set "VS_INSTALL_PATH=%%i"
        )
        if defined VS_INSTALL_PATH (
            echo [信息] Visual Studio 安装路径: !VS_INSTALL_PATH!
            call "!VS_INSTALL_PATH!\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
            if errorlevel 1 (
                echo [警告] 无法加载 vcvars64.bat，尝试使用 vcvarsall.bat
                call "!VS_INSTALL_PATH!\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
            )
        )
    )

    REM 检查 cl.exe
    where cl >nul 2>&1
    if errorlevel 1 (
        echo [错误] 未找到 MSVC 编译器 (cl.exe)
        echo   请安装 Visual Studio 2019/2022 并勾选 "C++ 桌面开发" 工作负载
        exit /b 1
    )
    echo [信息] MSVC 编译器已就绪
) else (
    echo.
    echo [信息] 配置 MinGW 编译器环境...

    REM 查找 MinGW
    if not defined CMAKE_C_COMPILER (
        REM 优先使用 Qt 自带的 MinGW
        set "MINGW_PATH=%QT_DIR%\..\..\Tools\mingw810_64\bin"
        if not exist "!MINGW_PATH!\g++.exe" (
            set "MINGW_PATH=%QT_DIR%\..\..\Tools\mingw1120_64\bin"
        )
        if not exist "!MINGW_PATH!\g++.exe" (
            set "MINGW_PATH=%QT_DIR%\..\..\Tools\mingw1310_64\bin"
        )
        if exist "!MINGW_PATH!\g++.exe" (
            echo [信息] 使用 Qt 自带的 MinGW: !MINGW_PATH!
            set "PATH=!MINGW_PATH!;%PATH%"
        ) else (
            REM 检查系统 PATH 中的 MinGW
            where g++ >nul 2>&1
            if errorlevel 1 (
                echo [错误] 未找到 MinGW g++ 编译器
                echo   请安装 MinGW-w64 或使用 Qt 自带的 MinGW
                echo   下载地址: https://www.mingw-w64.org/
                exit /b 1
            )
        )
    )

    REM 检查 g++
    where g++ >nul 2>&1
    if errorlevel 1 (
        echo [错误] 未找到 g++ 编译器
        exit /b 1
    )
    for /f "tokens=2 delims= " %%v in ('g++ --version 2^>nul ^| findstr /r "g++"') do (
        echo [信息] MinGW g++ 版本: %%v
        goto :mingw_checked
    )
    :mingw_checked
)

REM ============================================================
REM  CMake 配置
REM ============================================================

echo.
echo [信息] 正在配置 CMake...

set "BUILD_DIR=%PROJECT_DIR%\build\windows-%TOOLCHAIN%-%BUILD_TYPE%"

if "%USE_PRESET%"=="1" (
    echo [信息] 使用 CMake Preset: windows-%TOOLCHAIN%
    cmake --preset "windows-%TOOLCHAIN%" -S "%PROJECT_DIR%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_PREFIX_PATH="%CMAKE_PREFIX_PATH%"
) else (
    cmake -G "Ninja" -S "%PROJECT_DIR%" -B "%BUILD_DIR%" ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DCMAKE_PREFIX_PATH="%CMAKE_PREFIX_PATH%" ^
        -DCMAKE_CXX_STANDARD=17 ^
        -DCMAKE_CXX_STANDARD_REQUIRED=ON ^
        -DCMAKE_AUTOMOC=ON ^
        -DCMAKE_AUTORCC=ON ^
        -DOPENSSL_ROOT_DIR="%OPENSSL_DIR%"
)

if errorlevel 1 (
    echo [错误] CMake 配置失败！
    exit /b 1
)

echo [信息] CMake 配置成功

REM ============================================================
REM  编译项目
REM ============================================================

echo.
echo [信息] 正在编译项目 (Build Type: %BUILD_TYPE%)...

cmake --build "%BUILD_DIR%" --config %BUILD_TYPE% -- -j %NUMBER_OF_PROCESSORS%

if errorlevel 1 (
    echo [错误] 编译失败！
    exit /b 1
)

echo [信息] 编译成功

REM ============================================================
REM  创建发布目录
REM ============================================================

echo.
echo [信息] 正在创建发布目录...

set "RELEASE_DIR=%PROJECT_DIR%\release\ChatRoom-windows-x64"
set "RELEASE_BIN=%RELEASE_DIR%\bin"

REM 清理旧的发布目录
if exist "%RELEASE_DIR%" rd /s /q "%RELEASE_DIR%"

mkdir "%RELEASE_BIN%"

REM 复制可执行文件
if exist "%BUILD_DIR%\server\ChatRoomServer.exe" (
    copy /y "%BUILD_DIR%\server\ChatRoomServer.exe" "%RELEASE_BIN%\" >nul
    echo [信息] 已复制 ChatRoomServer.exe
) else (
    echo [警告] 未找到 ChatRoomServer.exe
)

if exist "%BUILD_DIR%\client\ChatRoomClient.exe" (
    copy /y "%BUILD_DIR%\client\ChatRoomClient.exe" "%RELEASE_BIN%\" >nul
    echo [信息] 已复制 ChatRoomClient.exe
) else (
    echo [警告] 未找到 ChatRoomClient.exe
)

REM MinGW: 去除调试符号
if /i "%TOOLCHAIN%"=="mingw" (
    echo [信息] 正在去除调试符号...
    where strip >nul 2>&1
    if not errorlevel 1 (
        if exist "%RELEASE_BIN%\ChatRoomServer.exe" (
            strip --strip-all "%RELEASE_BIN%\ChatRoomServer.exe"
            echo [信息] ChatRoomServer.exe 已去除调试符号
        )
        if exist "%RELEASE_BIN%\ChatRoomClient.exe" (
            strip --strip-all "%RELEASE_BIN%\ChatRoomClient.exe"
            echo [信息] ChatRoomClient.exe 已去除调试符号
        )
    ) else (
        echo [警告] 未找到 strip 工具，跳过去除调试符号
    )
)

REM ============================================================
REM  运行 windeployqt 收集 Qt 依赖
REM ============================================================

echo [信息] 正在运行 windeployqt 收集 Qt 依赖...

set "WINDEPLOYQT=%QT_DIR%\bin\windeployqt.exe"

if exist "%WINDEPLOYQT%" (
    if exist "%RELEASE_BIN%\ChatRoomClient.exe" (
        echo [信息] 正在为 ChatRoomClient.exe 收集依赖...
        "%WINDEPLOYQT%" --release --no-translations --no-opengl-sw "%RELEASE_BIN%\ChatRoomClient.exe"
        if errorlevel 1 (
            echo [警告] windeployqt 运行失败，可能缺少部分 Qt 依赖
        )
    )

    REM 额外复制 OpenSSL DLL
    if defined OPENSSL_DIR (
        if exist "%OPENSSL_DIR%\bin\libssl-1_1-x64.dll" (
            copy /y "%OPENSSL_DIR%\bin\libssl-1_1-x64.dll" "%RELEASE_BIN%\" >nul
            copy /y "%OPENSSL_DIR%\bin\libcrypto-1_1-x64.dll" "%RELEASE_BIN%\" >nul
            echo [信息] 已复制 OpenSSL 1.1 DLL
        )
        if exist "%OPENSSL_DIR%\bin\libssl-3-x64.dll" (
            copy /y "%OPENSSL_DIR%\bin\libssl-3-x64.dll" "%RELEASE_BIN%\" >nul
            copy /y "%OPENSSL_DIR%\bin\libcrypto-3-x64.dll" "%RELEASE_BIN%\" >nul
            echo [信息] 已复制 OpenSSL 3.x DLL
        )
    )
) else (
    echo [警告] 未找到 windeployqt，跳过 Qt 依赖收集
    echo   请手动复制 Qt DLL 到发布目录
)

REM ============================================================
REM  创建启动脚本
REM ============================================================

REM 创建启动服务端脚本
(
echo @echo off
echo chcp 65001 ^>nul 2^>^&1
echo setlocal
echo set "DIR=%%~dp0"
echo set "PATH=%%DIR%%bin;%%DIR%%bin\plugins\platforms;%%DIR%%bin\plugins\sqldrivers;%%DIR%%bin\plugins\imageformats;%%PATH%%"
echo set "QT_PLUGIN_PATH=%%DIR%%bin\plugins"
echo set "QML2_IMPORT_PATH=%%DIR%%bin\qml"
echo echo Starting ChatRoom Server on port %%CHATROOM_PORT:~-6667%%...
echo "%%DIR%%bin\ChatRoomServer.exe" %%*
echo pause
) > "%RELEASE_DIR%\start_server.bat"

REM 创建启动客户端脚本
(
echo @echo off
echo chcp 65001 ^>nul 2^>^&1
echo setlocal
echo set "DIR=%%~dp0"
echo set "PATH=%%DIR%%bin;%%DIR%%bin\plugins\platforms;%%DIR%%bin\plugins\sqldrivers;%%DIR%%bin\plugins\imageformats;%%PATH%%"
echo set "QT_PLUGIN_PATH=%%DIR%%bin\plugins"
echo set "QML2_IMPORT_PATH=%%DIR%%bin\qml"
echo echo Starting ChatRoom Client...
echo "%%DIR%%bin\ChatRoomClient.exe" %%*
echo pause
) > "%RELEASE_DIR%\start_client.bat"

echo [信息] 已创建启动脚本

REM ============================================================
REM  完成
REM ============================================================

echo.
echo ========================================================
echo   构建完成！
echo   发布目录: %RELEASE_DIR%
echo.
echo   目录结构:
echo   ChatRoom-windows-x64\
echo     +-- bin\
echo     +--     +-- ChatRoomServer.exe
echo     +--     +-- ChatRoomClient.exe
echo     +--     +-- (Qt DLLs 和插件)
echo     +-- start_server.bat
echo     +-- start_client.bat
echo ========================================================
echo.

exit /b 0

REM ============================================================
REM  帮助信息
REM ============================================================
:usage
echo.
echo 用法: build_windows.bat [选项]
echo.
echo 选项:
echo   msvc              使用 MSVC 编译器 (默认: mingw)
echo   mingw             使用 MinGW 编译器 (默认)
echo   debug             构建 Debug 版本
echo   release           构建 Release 版本 (默认)
echo   --preset          使用 CMake Preset 配置
echo   --help, -h        显示帮助信息
echo.
echo 环境变量:
echo   QT_DIR            Qt6 安装路径
echo                     示例: C:\Qt\6.5.3\mingw81_64
echo   OPENSSL_DIR       OpenSSL 安装路径
echo                     示例: C:\OpenSSL-Win64
echo   CMAKE_PREFIX_PATH CMake 搜索路径 (追加)
echo.
echo 示例:
echo   build_windows.bat mingw release
echo   build_windows.bat msvc debug
echo   build_windows.bat --preset mingw release
echo.
exit /b 0
