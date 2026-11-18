@echo off
chcp 65001 >nul
rem 一键运行：优先用 bin\ 中的产物，缺失时退回 build\，再缺失则现场构建
setlocal enabledelayedexpansion
set "DIR=%~dp0"
set "BIN=%DIR%bin\lexer-windows-x86_64.exe"

if not exist "!BIN!" set "BIN=%DIR%bin\lexer.exe"
if not exist "!BIN!" set "BIN=%DIR%build\Release\lexer.exe"
if not exist "!BIN!" set "BIN=%DIR%build\lexer.exe"

if not exist "!BIN!" (
    echo 未找到可执行文件，正在构建...
    cmake -S "%DIR%." -B "%DIR%build"
    if errorlevel 1 exit /b 2
    cmake --build "%DIR%build" --config Release
    if errorlevel 1 exit /b 2
    set "BIN=%DIR%build\Release\lexer.exe"
    if not exist "!BIN!" set "BIN=%DIR%build\lexer.exe"
)

"!BIN!" %*
exit /b %errorlevel%
