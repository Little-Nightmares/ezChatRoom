@echo off
chcp 65001 >nul 2>&1
cd /d "%~dp0"
set PATH=%~dp0bin;%PATH%
set QT_PLUGIN_PATH=%~dp0plugins
set QML2_IMPORT_PATH=%~dp0qml
echo Starting ChatRoom Client v1.9...
bin\ChatRoomClient.exe 2>error.log
echo Program exited with code: %errorlevel%
if exist error.log (type error.log)
pause
