@echo off
setlocal

cd /d "%~dp0"

rem Build first if the executable does not exist yet.
if not exist "build\vs2022\Debug\monster_rogue.exe" (
    call "%~dp0build_vs2022.bat"
    if errorlevel 1 exit /b 1
)

rem Use UTF-8 in the console and run from the project root.
chcp 65001 > nul
"%~dp0build\vs2022\Debug\monster_rogue.exe"
