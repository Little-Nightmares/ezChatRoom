@echo off
title ChatRoom Server

set SERVER_PATH=build\Desktop_Qt_6_11_0_MinGW_64_bit-Debug\ChatRoomServer.exe
set QT_PATH=C:\Qt\6.11.0\mingw_64\bin
set MINGW_PATH=C:\Qt\Tools\mingw1310_64\bin
set OPENSSL_PATH=C:\Program Files\OpenSSL-Win64\bin

if not exist "%SERVER_PATH%" (
    echo [ERROR] Server not found: %SERVER_PATH%
    echo [HINT] Build ChatRoomServer in Qt Creator first ^(Ctrl+B^).
    pause
    exit /b 1
)

if not exist "%QT_PATH%" (
    echo [ERROR] Qt path not found: %QT_PATH%
    echo [HINT] Make sure Qt 6.11.0 is installed in C:\Qt.
    pause
    exit /b 1
)

set PATH=%QT_PATH%;%MINGW_PATH%;%OPENSSL_PATH%;%PATH%

echo [INFO] Starting ChatRoom Server...
echo [INFO] Default port: 6667
echo.

"%SERVER_PATH%" 2> server_error.log

set EXIT_CODE=%ERRORLEVEL%

if %EXIT_CODE% neq 0 (
    echo.
    echo [ERROR] Server crashed with exit code: %EXIT_CODE%
    echo.
    if exist server_error.log (
        echo === Error Log ===
        type server_error.log
        echo =================
    ) else (
        echo [No error details captured]
    )
    pause
) else (
    echo [INFO] Server exited normally.
    timeout /t 2 /nobreak >nul
)
