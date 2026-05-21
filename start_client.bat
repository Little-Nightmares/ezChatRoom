@echo off
title ChatRoom Client

set CLIENT_PATH=build\Desktop_Qt_6_11_0_MinGW_64_bit-Debug\ChatRoomClient.exe
set QT_PATH=C:\Qt\6.11.0\mingw_64\bin
set MINGW_PATH=C:\Qt\Tools\mingw1310_64\bin
set OPENSSL_PATH=C:\Program Files\OpenSSL-Win64\bin

if not exist "%CLIENT_PATH%" (
    echo [ERROR] Client not found: %CLIENT_PATH%
    echo [HINT] Build ChatRoomClient in Qt Creator first ^(Ctrl+B^).
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

echo [INFO] Starting ChatRoom Client...
echo.

:: Run client and capture error output to a log file
"%CLIENT_PATH%" 2> client_error.log

set EXIT_CODE=%ERRORLEVEL%

if %EXIT_CODE% neq 0 (
    echo.
    echo [ERROR] Client crashed with exit code: %EXIT_CODE%
    echo.
    if exist client_error.log (
        echo === Error Log ===
        type client_error.log
        echo =================
    ) else (
        echo [No error details captured]
    )
    echo.
    echo [HINT] Try rebuilding: Open Qt Creator ^(Ctrl+B^), then run this again.
    echo [HINT] If still crashing, check client_error.log for details.
    pause
) else (
    echo [INFO] Client exited normally.
    timeout /t 2 /nobreak >nul
)
