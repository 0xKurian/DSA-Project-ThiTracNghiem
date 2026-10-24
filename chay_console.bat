@echo off
chcp 65001 >nul
title HE THONG THI TRAC NGHIEM C++ (CONSOLE)
cd /d "%~dp0"
if not exist "build\app.exe" (
    echo Dang bien dich ung dung Console...
    make app
)
build\app.exe --console
pause
