@echo off
chcp 65001 >nul
rem 生成 Windows 分发包：静态 Release 构建、跑测试、铺 dist 目录、打包 zip、出校验和
setlocal enabledelayedexpansion

set "ROOT=%~dp0.."
set "VERSION=1.0"
set "NAME=lexer-%VERSION%-windows-x86_64"
set "STAGE=%ROOT%\dist\%NAME%"

echo 构建 Release 静态产物...
cmake -S "%ROOT%\lexer" -B "%ROOT%\lexer\build-static" -DCMAKE_BUILD_TYPE=Release -DLEXER_STATIC=ON
if errorlevel 1 goto fail
cmake --build "%ROOT%\lexer\build-static" --config Release
if errorlevel 1 goto fail

echo 运行测试...
set "LEXER_BIN=%ROOT%\lexer\build-static\Release\lexer.exe"
call "%ROOT%\lexer\tests\run_tests.bat"
if errorlevel 1 goto fail

echo 整理 %NAME% ...
if exist "%STAGE%" rmdir /s /q "%STAGE%"
mkdir "%STAGE%\bin" "%STAGE%\include" "%STAGE%\src"
mkdir "%STAGE%\tests\cases" "%STAGE%\tests\expected" "%STAGE%\examples" "%STAGE%\docs"

copy /y "%ROOT%\lexer\build-static\Release\lexer.exe" "%STAGE%\bin\lexer-windows-x86_64.exe" >nul
copy /y "%ROOT%\lexer\CMakeLists.txt" "%STAGE%\" >nul
copy /y "%ROOT%\lexer\run.sh" "%STAGE%\" >nul
copy /y "%ROOT%\lexer\run.bat" "%STAGE%\" >nul
xcopy /e /i /y /q "%ROOT%\lexer\include" "%STAGE%\include" >nul
xcopy /e /i /y /q "%ROOT%\lexer\src" "%STAGE%\src" >nul
copy /y "%ROOT%\lexer\tests\cases\*.c" "%STAGE%\tests\cases\" >nul
copy /y "%ROOT%\lexer\tests\expected\*.out" "%STAGE%\tests\expected\" >nul
copy /y "%ROOT%\lexer\tests\run_tests.sh" "%STAGE%\tests\" >nul
copy /y "%ROOT%\lexer\tests\run_tests.bat" "%STAGE%\tests\" >nul
copy /y "%ROOT%\lexer\examples\*.c" "%STAGE%\examples\" >nul
copy /y "%ROOT%\docs\*.md" "%STAGE%\docs\" >nul
copy /y "%ROOT%\README.md" "%STAGE%\" >nul

if exist "%ROOT%\dist\%NAME%.zip" del /q "%ROOT%\dist\%NAME%.zip"
powershell -NoProfile -Command "Compress-Archive -Path '%STAGE%' -DestinationPath '%ROOT%\dist\%NAME%.zip' -Force"
if errorlevel 1 goto fail

set "HASH="
for /f "skip=1 delims=" %%h in ('certutil -hashfile "%ROOT%\dist\%NAME%.zip" SHA256') do (
    if not defined HASH set "HASH=%%h"
)
echo %NAME%.zip !HASH!> "%ROOT%\dist\CHECKSUMS.txt"

echo 完成：dist\%NAME%.zip
exit /b 0

:fail
echo 打包失败
exit /b 1
