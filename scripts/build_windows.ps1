<#
.SYNOPSIS
    ChatRoom Windows 构建脚本 (PowerShell 版本)
.DESCRIPTION
    支持自动检测 Qt6 和 OpenSSL 安装，构建 Debug/Release 配置，
    运行 windeployqt 收集依赖，并创建 ZIP 发布包。
.PARAMETER Toolchain
    编译器选择: "msvc" 或 "mingw" (默认: "mingw")
.PARAMETER BuildType
    构建类型: "debug", "release" 或 "both" (默认: "release")
.PARAMETER UsePreset
    使用 CMake Preset 进行配置
.PARAMETER SkipDeploy
    跳过 windeployqt 依赖收集
.PARAMETER CreateZip
    构建完成后创建 ZIP 压缩包
.PARAMETER QtDir
    手动指定 Qt6 安装路径
.PARAMETER OpenSslDir
    手动指定 OpenSSL 安装路径
.EXAMPLE
    .\build_windows.ps1 -Toolchain mingw -BuildType release
    .\build_windows.ps1 -Toolchain msvc -BuildType both -CreateZip
    .\build_windows.ps1 -QtDir "C:\Qt\6.5.3\mingw81_64" -BuildType release
#>

[CmdletBinding()]
param(
    [ValidateSet("msvc", "mingw")]
    [string]$Toolchain = "mingw",

    [ValidateSet("debug", "release", "both")]
    [string]$BuildType = "release",

    [switch]$UsePreset,
    [switch]$SkipDeploy,
    [switch]$CreateZip,

    [string]$QtDir,
    [string]$OpenSslDir
)

# ============================================================
#  设置错误处理
# ============================================================
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# 脚本和项目目录
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectDir = Split-Path -Parent $ScriptDir

# 辅助函数：输出带颜色的消息
function Write-Info($Message) {
    Write-Host "[信息] $Message" -ForegroundColor Cyan
}

function Write-Success($Message) {
    Write-Host "[成功] $Message" -ForegroundColor Green
}

function Write-Warning($Message) {
    Write-Host "[警告] $Message" -ForegroundColor Yellow
}

function Write-Error($Message) {
    Write-Host "[错误] $Message" -ForegroundColor Red
}

function Test-Command($Command) {
    try {
        $null = Get-Command $Command -ErrorAction Stop
        return $true
    } catch {
        return $false
    }
}

# ============================================================
#  打印构建信息
# ============================================================
Write-Host ""
Write-Host "========================================================" -ForegroundColor White
Write-Host "  ChatRoom Windows 构建脚本 (PowerShell)" -ForegroundColor White
Write-Host "  编译器: $Toolchain" -ForegroundColor White
Write-Host "  构建类型: $BuildType" -ForegroundColor White
Write-Host "========================================================" -ForegroundColor White
Write-Host ""

# ============================================================
#  检查 CMake
# ============================================================
if (-not (Test-Command "cmake")) {
    Write-Error "未找到 CMake，请安装 CMake 3.16+ 并添加到 PATH"
    Write-Host "  下载地址: https://cmake.org/download/" -ForegroundColor Gray
    exit 1
}

$cmakeVersion = (cmake --version 2>&1 | Select-String "cmake version").ToString().Split(" ")[2]
Write-Info "CMake 版本: $cmakeVersion"

# 检查 CMake 版本是否 >= 3.16
$minVersion = [version]"3.16.0"
$currentVersion = [version]$cmakeVersion
if ($currentVersion -lt $minVersion) {
    Write-Error "CMake 版本过低: $cmakeVersion，需要 3.16+"
    exit 1
}

# ============================================================
#  自动检测 Qt6 安装
# ============================================================
if (-not $QtDir) {
    $QtDir = $env:QT_DIR
}

if (-not $QtDir) {
    Write-Info "QT_DIR 未设置，尝试自动检测 Qt6 安装..."

    # 常见 Qt 安装路径
    $qtBasePaths = @(
        "C:\Qt",
        "D:\Qt",
        "${env:LOCALAPPDATA}\Qt"
    )

    # 搜索的 Qt 版本
    $qtVersions = @(
        "6.2.0", "6.2.3", "6.2.4",
        "6.3.0", "6.3.1", "6.3.2",
        "6.4.0", "6.4.1", "6.4.2", "6.4.3",
        "6.5.0", "6.5.1", "6.5.2", "6.5.3",
        "6.6.0", "6.6.1", "6.6.2", "6.6.3",
        "6.7.0", "6.7.1",
        "6.8.0", "6.8.1"
    )

    # 根据编译器选择子目录
    $compilerSubDirs = if ($Toolchain -eq "msvc") {
        @("msvc2019_64", "msvc2022_64")
    } else {
        @("mingw81_64", "mingw1120_64", "mingw1310_64", "mingw_64")
    }

    $qtFound = $false
    foreach ($base in $qtBasePaths) {
        if (-not (Test-Path $base)) { continue }
        foreach ($ver in $qtVersions) {
            foreach ($sub in $compilerSubDirs) {
                $candidate = Join-Path $base "$ver\$sub"
                $qmakePath = Join-Path $candidate "bin\qmake.exe"
                if (Test-Path $qmakePath) {
                    $QtDir = $candidate
                    $qtFound = $true
                    break
                }
            }
            if ($qtFound) { break }
        }
        if ($qtFound) { break }
    }

    # 尝试通过注册表查找 Qt
    if (-not $qtFound) {
        try {
            $regPath = "HKCU:\Software\Trolltech\Qt5Versions"
            if (Test-Path $regPath) {
                $qtReg = Get-ItemProperty $regPath
                # 检查是否有 6.x 版本
                $qtInstallDir = $qtReg.InstallDir
                if ($qtInstallDir -and (Test-Path $qtInstallDir)) {
                    $QtDir = $qtInstallDir
                    $qtFound = $true
                }
            }
        } catch {
            # 忽略注册表读取错误
        }
    }

    if (-not $qtFound) {
        Write-Error "未找到 Qt6 安装！"
        Write-Host "  请使用 -QtDir 参数指定 Qt6 安装目录" -ForegroundColor Gray
        Write-Host "  示例: .\build_windows.ps1 -QtDir 'C:\Qt\6.5.3\mingw81_64'" -ForegroundColor Gray
        Write-Host "  下载地址: https://www.qt.io/download-qt-installer" -ForegroundColor Gray
        exit 1
    }
}

# 验证 Qt 目录
$qmakeExe = Join-Path $QtDir "bin\qmake.exe"
if (-not (Test-Path $qmakeExe)) {
    Write-Error "Qt 目录无效，未找到 qmake.exe: $QtDir"
    exit 1
}

Write-Success "Qt6 路径: $QtDir"

# ============================================================
#  自动检测 OpenSSL 安装
# ============================================================
if (-not $OpenSslDir) {
    $OpenSslDir = $env:OPENSSL_DIR
}

if (-not $OpenSslDir) {
    Write-Info "OPENSSL_DIR 未设置，尝试自动检测 OpenSSL 安装..."

    $opensslSearchPaths = @(
        "C:\OpenSSL",
        "C:\OpenSSL-Win64",
        "C:\Program Files\OpenSSL",
        "C:\Program Files (x86)\OpenSSL",
        "C:\Tools\OpenSSL",
        "C:\Utils\OpenSSL-Win64",
        "${env:ProgramFiles}\OpenSSL-Win64",
        "${env:ProgramFiles(x86)}\OpenSSL-Win64"
    )

    foreach ($path in $opensslSearchPaths) {
        $opensslExe = Join-Path $path "bin\openssl.exe"
        if (Test-Path $opensslExe) {
            $OpenSslDir = $path
            break
        }
    }

    if (-not $OpenSslDir) {
        Write-Warning "未找到 OpenSSL 安装！某些加密功能可能不可用"
        Write-Host "  请使用 -OpenSslDir 参数指定 OpenSSL 安装目录" -ForegroundColor Gray
        Write-Host "  下载地址: https://slproweb.com/products/Win32OpenSSL.html" -ForegroundColor Gray
    }
}

if ($OpenSslDir) {
    Write-Success "OpenSSL 路径: $OpenSslDir"
}

# ============================================================
#  设置 CMAKE_PREFIX_PATH
# ============================================================
$cmakePrefixPaths = @($QtDir)
if ($OpenSslDir) {
    $cmakePrefixPaths += $OpenSslDir
}
if ($env:CMAKE_PREFIX_PATH) {
    $cmakePrefixPaths += $env:CMAKE_PREFIX_PATH -split ";"
}
$env:CMAKE_PREFIX_PATH = $cmakePrefixPaths -join ";"
Write-Info "CMAKE_PREFIX_PATH: $($env:CMAKE_PREFIX_PATH)"

# ============================================================
#  配置编译器环境
# ============================================================
if ($Toolchain -eq "msvc") {
    Write-Info "配置 MSVC 编译器环境..."

    # 使用 vswhere 查找 Visual Studio
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsInstallPath = & $vsWhere -latest -property installationPath 2>$null
        if ($vsInstallPath) {
            Write-Info "Visual Studio 安装路径: $vsInstallPath"
            $vcvarsBat = Join-Path $vsInstallPath "VC\Auxiliary\Build\vcvars64.bat"
            if (Test-Path $vcvarsBat) {
                # 使用 cmd 运行 vcvars64.bat 并导入环境变量
                $cmdOutput = cmd /c "`"$vcvarsBat`" >nul 2>&1 && set" 2>$null
                foreach ($line in $cmdOutput) {
                    if ($line -match "^([^=]+)=(.*)$") {
                        $key = $matches[1]
                        $value = $matches[2]
                        Set-Item -Path "env:$key" -Value $value
                    }
                }
                Write-Success "MSVC 环境已加载"
            } else {
                Write-Warning "未找到 vcvars64.bat: $vcvarsBat"
            }
        }
    } else {
        Write-Warning "未找到 vswhere.exe，请确保 Visual Studio 已安装"
    }

    # 验证 cl.exe
    if (-not (Test-Command "cl")) {
        Write-Error "未找到 MSVC 编译器 (cl.exe)"
        Write-Host "  请安装 Visual Studio 2019/2022 并勾选 'C++ 桌面开发' 工作负载" -ForegroundColor Gray
        exit 1
    }
} else {
    Write-Info "配置 MinGW 编译器环境..."

    # 优先使用 Qt 自带的 MinGW
    $mingwPaths = @(
        (Join-Path (Split-Path (Split-Path $QtDir -Parent) -Parent) "Tools\mingw1310_64\bin"),
        (Join-Path (Split-Path (Split-Path $QtDir -Parent) -Parent) "Tools\mingw1120_64\bin"),
        (Join-Path (Split-Path (Split-Path $QtDir -Parent) -Parent) "Tools\mingw810_64\bin")
    )

    $mingwFound = $false
    foreach ($mp in $mingwPaths) {
        $gppExe = Join-Path $mp "g++.exe"
        if (Test-Path $gppExe) {
            $env:PATH = "$mp;$($env:PATH)"
            Write-Info "使用 Qt 自带的 MinGW: $mp"
            $mingwFound = $true
            break
        }
    }

    if (-not $mingwFound) {
        if (-not (Test-Command "g++")) {
            Write-Error "未找到 MinGW g++ 编译器"
            Write-Host "  请安装 MinGW-w64 或使用 Qt 自带的 MinGW" -ForegroundColor Gray
            Write-Host "  下载地址: https://www.mingw-w64.org/" -ForegroundColor Gray
            exit 1
        }
    }

    $gppVersion = (g++ --version 2>&1 | Select-String "g++").ToString().Trim()
    Write-Info "MinGW: $gppVersion"
}

# ============================================================
#  构建函数
# ============================================================
function Build-Project {
    param(
        [string]$ConfigName,
        [string]$ConfigType
    )

    Write-Host ""
    Write-Info "========== 开始构建 $ConfigName 配置 =========="

    $buildDir = Join-Path $ProjectDir "build\windows-$Toolchain-$ConfigName"

    # CMake 配置
    Write-Info "正在配置 CMake ($ConfigName)..."

    $cmakeArgs = @(
        "-S", $ProjectDir,
        "-B", $buildDir,
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=$ConfigType",
        "-DCMAKE_PREFIX_PATH=`"$($env:CMAKE_PREFIX_PATH)`"",
        "-DCMAKE_CXX_STANDARD=17",
        "-DCMAKE_CXX_STANDARD_REQUIRED=ON",
        "-DCMAKE_AUTOMOC=ON",
        "-DCMAKE_AUTORCC=ON"
    )

    if ($OpenSslDir) {
        $cmakeArgs += "-DOPENSSL_ROOT_DIR=`"$OpenSslDir`""
    }

    if ($UsePreset) {
        Write-Info "使用 CMake Preset: windows-$Toolchain"
        $cmakeArgs = @(
            "--preset", "windows-$Toolchain",
            "-DCMAKE_BUILD_TYPE=$ConfigType"
        )
    }

    $cmakeResult = & cmake @cmakeArgs 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake 配置失败 ($ConfigName)！"
        Write-Host ($cmakeResult | Out-String)
        exit 1
    }

    # 编译
    Write-Info "正在编译 ($ConfigName)..."
    $numProcs = [Environment]::ProcessorCount
    $buildResult = & cmake --build $buildDir --config $ConfigType -- "-j" $numProcs 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Error "编译失败 ($ConfigName)！"
        Write-Host ($buildResult | Out-String)
        exit 1
    }

    Write-Success "$ConfigName 构建成功"
    return $buildDir
}

# ============================================================
#  执行构建
# ============================================================
$buildDirs = @{}

if ($BuildType -eq "debug" -or $BuildType -eq "both") {
    $buildDirs["debug"] = Build-Project -ConfigName "debug" -ConfigType "Debug"
}

if ($BuildType -eq "release" -or $BuildType -eq "both") {
    $buildDirs["release"] = Build-Project -ConfigName "release" -ConfigType "Release"
}

# ============================================================
#  创建发布包
# ============================================================
Write-Host ""
Write-Info "正在创建发布目录..."

$releaseDir = Join-Path $ProjectDir "release\ChatRoom-windows-x64"
$releaseBin = Join-Path $releaseDir "bin"

# 使用 Release 构建作为发布基础，如果没有则使用 Debug
$primaryBuild = if ($buildDirs.ContainsKey("release")) { $buildDirs["release"] } else { $buildDirs["debug"] }

# 清理旧的发布目录
if (Test-Path $releaseDir) {
    Remove-Item -Recurse -Force $releaseDir
}

New-Item -ItemType Directory -Path $releaseBin -Force | Out-Null

# 复制可执行文件
$serverExe = Join-Path $primaryBuild "server\ChatRoomServer.exe"
$clientExe = Join-Path $primaryBuild "client\ChatRoomClient.exe"

if (Test-Path $serverExe) {
    Copy-Item -Path $serverExe -Destination $releaseBin -Force
    Write-Info "已复制 ChatRoomServer.exe"
} else {
    Write-Warning "未找到 ChatRoomServer.exe"
}

if (Test-Path $clientExe) {
    Copy-Item -Path $clientExe -Destination $releaseBin -Force
    Write-Info "已复制 ChatRoomClient.exe"
} else {
    Write-Warning "未找到 ChatRoomClient.exe"
}

# MinGW: 去除调试符号
if ($Toolchain -eq "mingw" -and (Test-Command "strip")) {
    Write-Info "正在去除调试符号..."
    $serverDest = Join-Path $releaseBin "ChatRoomServer.exe"
    $clientDest = Join-Path $releaseBin "ChatRoomClient.exe"
    if (Test-Path $serverDest) {
        & strip --strip-all $serverDest 2>$null
        Write-Info "ChatRoomServer.exe 已去除调试符号"
    }
    if (Test-Path $clientDest) {
        & strip --strip-all $clientDest 2>$null
        Write-Info "ChatRoomClient.exe 已去除调试符号"
    }
}

# ============================================================
#  运行 windeployqt 收集 Qt 依赖
# ============================================================
if (-not $SkipDeploy) {
    Write-Info "正在运行 windeployqt 收集 Qt 依赖..."

    $windeployqt = Join-Path $QtDir "bin\windeployqt.exe"

    if (Test-Path $windeployqt) {
        $clientDest = Join-Path $releaseBin "ChatRoomClient.exe"
        if (Test-Path $clientDest) {
            Write-Info "正在为 ChatRoomClient.exe 收集依赖..."
            $deployArgs = @(
                "--release",
                "--no-translations",
                "--no-opengl-sw",
                $clientDest
            )
            & $windeployqt @deployArgs 2>&1 | Out-Null
            if ($LASTEXITCODE -ne 0) {
                Write-Warning "windeployqt 运行失败，可能缺少部分 Qt 依赖"
            } else {
                Write-Success "Qt 依赖收集完成"
            }
        }

        # 复制 OpenSSL DLL
        if ($OpenSslDir) {
            $opensslBin = Join-Path $OpenSslDir "bin"
            $opensslDlls = @(
                "libssl-1_1-x64.dll", "libcrypto-1_1-x64.dll",
                "libssl-3-x64.dll", "libcrypto-3-x64.dll",
                "libssl-1_1.dll", "libcrypto-1_1.dll",
                "libssl-3.dll", "libcrypto-3.dll"
            )
            foreach ($dll in $opensslDlls) {
                $dllSrc = Join-Path $opensslBin $dll
                if (Test-Path $dllSrc) {
                    Copy-Item -Path $dllSrc -Destination $releaseBin -Force
                    Write-Info "已复制 $dll"
                }
            }
        }
    } else {
        Write-Warning "未找到 windeployqt ($windeployqt)，跳过 Qt 依赖收集"
        Write-Host "  请手动复制 Qt DLL 到发布目录" -ForegroundColor Gray
    }
}

# ============================================================
#  创建启动脚本
# ============================================================
Write-Info "正在创建启动脚本..."

# 启动服务端脚本
$serverBat = @"
@echo off
chcp 65001 >nul 2>&1
setlocal
set "DIR=%~dp0"
set "PATH=%DIR%bin;%DIR%bin\plugins\platforms;%DIR%bin\plugins\sqldrivers;%DIR%bin\plugins\imageformats;%PATH%"
set "QT_PLUGIN_PATH=%DIR%bin\plugins"
set "QML2_IMPORT_PATH=%DIR%bin\qml"
echo Starting ChatRoom Server on port %CHATROOM_PORT:~-6667%...
"%DIR%bin\ChatRoomServer.exe" %*
pause
"@
Set-Content -Path (Join-Path $releaseDir "start_server.bat") -Value $serverBat -Encoding ASCII

# 启动客户端脚本
$clientBat = @"
@echo off
chcp 65001 >nul 2>&1
setlocal
set "DIR=%~dp0"
set "PATH=%DIR%bin;%DIR%bin\plugins\platforms;%DIR%bin\plugins\sqldrivers;%DIR%bin\plugins\imageformats;%PATH%"
set "QT_PLUGIN_PATH=%DIR%bin\plugins"
set "QML2_IMPORT_PATH=%DIR%bin\qml"
echo Starting ChatRoom Client...
"%DIR%bin\ChatRoomClient.exe" %*
pause
"@
Set-Content -Path (Join-Path $releaseDir "start_client.bat") -Value $clientBat -Encoding ASCII

Write-Success "启动脚本已创建"

# ============================================================
#  创建 ZIP 压缩包
# ============================================================
if ($CreateZip) {
    Write-Info "正在创建 ZIP 压缩包..."

    $zipFile = Join-Path $ProjectDir "release\ChatRoom-windows-x64.zip"

    if (Test-Path $zipFile) {
        Remove-Item -Force $zipFile
    }

    # 使用 PowerShell 5.0+ 的 Compress-Archive
    if ($PSVersionTable.PSVersion.Major -ge 5) {
        Compress-Archive -Path $releaseDir -DestinationPath $zipFile -CompressionLevel Optimal
        Write-Success "ZIP 压缩包已创建: $zipFile"
    } else {
        Write-Warning "PowerShell 版本过低，无法创建 ZIP (需要 5.0+)"
    }
}

# ============================================================
#  完成
# ============================================================
Write-Host ""
Write-Host "========================================================" -ForegroundColor Green
Write-Host "  构建完成！" -ForegroundColor Green
Write-Host "  发布目录: $releaseDir" -ForegroundColor Green
Write-Host ""
Write-Host "  目录结构:" -ForegroundColor Green
Write-Host "  ChatRoom-windows-x64\" -ForegroundColor Green
Write-Host "    +-- bin\" -ForegroundColor Green
Write-Host "    |     +-- ChatRoomServer.exe" -ForegroundColor Green
Write-Host "    |     +-- ChatRoomClient.exe" -ForegroundColor Green
Write-Host "    |     +-- (Qt DLLs 和插件)" -ForegroundColor Green
Write-Host "    +-- start_server.bat" -ForegroundColor Green
Write-Host "    +-- start_client.bat" -ForegroundColor Green
Write-Host "========================================================" -ForegroundColor Green
Write-Host ""

exit 0
