@echo off
rem 逐用例比对期望输出（Windows 下用 fc /b，避免换行差异误判）
setlocal enabledelayedexpansion
cd /d "%~dp0.."

set "BIN=build\Release\lexer.exe"
if not exist "!BIN!" set "BIN=build\lexer.exe"
if not exist "!BIN!" (
    cmake -S . -B build
    cmake --build build --config Release
    set "BIN=build\Release\lexer.exe"
    if not exist "!BIN!" set "BIN=build\lexer.exe"
)

set /a PASS=0
set /a FAIL=0

for %%f in (tests\cases\*.c) do (
    set "NAME=%%~nf"
    "!BIN!" "tests\cases\%%~nf.c" > "tests\actual.tmp" 2>&1
    fc /b "tests\expected\%%~nf.out" "tests\actual.tmp" >nul 2>&1
    if errorlevel 1 (
        echo FAIL  %%~nf
        set /a FAIL+=1
    ) else (
        echo PASS  %%~nf
        set /a PASS+=1
    )
)

del "tests\actual.tmp" >nul 2>&1
echo -----
echo 合计：!PASS! 通过，!FAIL! 失败
if !FAIL! neq 0 exit /b 1
