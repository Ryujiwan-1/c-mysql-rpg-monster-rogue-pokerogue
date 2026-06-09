@echo off
setlocal

set "ROOT=%~dp0"
set "DOWNLOAD_DIR=%ROOT%work\downloads"
set "ZIP_FILE=%DOWNLOAD_DIR%\mysql-5.7.44-winx64.zip"
set "EXTRACT_DIR=%DOWNLOAD_DIR%\mysql57_extract"
set "SERVER_DIR=%ROOT%mysql57\server"
set "MYSQL_URL=https://cdn.mysql.com/Downloads/MySQL-5.7/mysql-5.7.44-winx64.zip"

rem This script installs the project-local MySQL 5.7 ZIP distribution.
rem It avoids committing large MySQL binary files to GitHub.

if exist "%SERVER_DIR%\bin\mysqld.exe" (
    echo [OK] MySQL 5.7 server already exists: %SERVER_DIR%
    exit /b 0
)

mkdir "%DOWNLOAD_DIR%" 2>nul
mkdir "%ROOT%mysql57" 2>nul

if not exist "%ZIP_FILE%" (
    echo [INFO] Downloading MySQL 5.7.44...
    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
      "Invoke-WebRequest -Uri '%MYSQL_URL%' -OutFile '%ZIP_FILE%'"
    if errorlevel 1 exit /b 1
)

echo [INFO] Extracting MySQL 5.7.44...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "Expand-Archive -LiteralPath '%ZIP_FILE%' -DestinationPath '%EXTRACT_DIR%' -Force"
if errorlevel 1 exit /b 1

if exist "%SERVER_DIR%" rmdir /s /q "%SERVER_DIR%"
xcopy "%EXTRACT_DIR%\mysql-5.7.44-winx64" "%SERVER_DIR%\" /E /I /Y >nul
if errorlevel 1 exit /b 1

mkdir "%ROOT%mysql57\data" 2>nul
mkdir "%ROOT%mysql57\logs" 2>nul

echo [OK] MySQL 5.7 installed locally.
echo Run start_mysql57.bat before running the game.
