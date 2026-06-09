@echo off
setlocal

set "ROOT=%~dp0"
set "MYSQL=%ROOT%mysql57\server\bin\mysql.exe"
set "MYINI=%ROOT%mysql57\my.ini"

rem Gracefully stop the project-local MySQL 5.7 server.
"%MYSQL%" --defaults-file="%MYINI%" -uroot -proot -e "SHUTDOWN;" >nul 2>nul
if errorlevel 1 (
    echo [WARN] Graceful shutdown failed. MySQL may already be stopped.
) else (
    echo [OK] MySQL 5.7 stopped.
)
