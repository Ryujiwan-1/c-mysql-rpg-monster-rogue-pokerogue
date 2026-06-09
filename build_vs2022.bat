@echo off
setlocal

set "ROOT=%~dp0"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

rem Default to the project-local MySQL 5.7 ZIP distribution.
rem This path contains include\mysql.h and lib\libmysql.lib.
if not defined MySqlDir (
    set "MySqlDir=%ROOT%mysql57\server"
)

rem Check the MySQL headers and import library needed by MSVC.
if not exist "%MySqlDir%\include\mysql.h" (
    echo [ERROR] mysql.h not found: %MySqlDir%\include\mysql.h
    echo Set MySqlDir to your MySQL Connector or MySQL Server folder.
    exit /b 1
)

if not exist "%MySqlDir%\lib\libmysql.lib" (
    echo [ERROR] libmysql.lib not found: %MySqlDir%\lib\libmysql.lib
    echo Set MySqlDir to your MySQL Connector or MySQL Server folder.
    exit /b 1
)

rem Find the Visual Studio installation automatically.
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do set "VSINSTALL=%%i"
)

if not defined VSINSTALL (
    set "VSINSTALL=C:\Program Files\Microsoft Visual Studio\2022\Community"
)

if not exist "%VSINSTALL%\Common7\Tools\VsDevCmd.bat" (
    echo [ERROR] Visual Studio 2022 developer tools not found.
    exit /b 1
)

rem Start the x64 developer environment, then build Debug/x64.
rem /p:MySqlDir passes the MySQL path into MonsterRogue.vcxproj.
call "%VSINSTALL%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
msbuild "%ROOT%MonsterRogue.sln" /m /p:Configuration=Debug /p:Platform=x64 /p:MySqlDir="%MySqlDir%"
