@echo off
chcp 65001 >nul 2>&1
cd /d "%~dp0"
set PATH=%~dp0bin;%PATH%
set QT_PLUGIN_PATH=%~dp0plugins
echo Starting ChatRoom Server v1.9...
echo ============================================
bin\ChatRoomServer.exe 2>&1
echo ============================================
pause
