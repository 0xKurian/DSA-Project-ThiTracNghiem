@echo off
chcp 65001 >nul
title HE THONG THI TRAC NGHIEM C++ (GUI RAYLIB)
cd /d "%~dp0"
if not exist "build\app.exe" (
    echo Dang bien dich ung dung Do hoa...
    make app
)
start "" "build\app.exe"
