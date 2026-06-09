@echo off
setlocal

set "ROOT=%~dp0"
set "MYSQLD=%ROOT%mysql57\server\bin\mysqld.exe"
set "MYSQL=%ROOT%mysql57\server\bin\mysql.exe"
set "MYINI=%ROOT%mysql57\my.ini"
set "DATA=%ROOT%mysql57\data"
set "LOGS=%ROOT%mysql57\logs"

rem Check that the project-local MySQL 5.7 server exists.
if not exist "%MYSQLD%" (
    echo [ERROR] mysqld.exe not found: %MYSQLD%
    echo Run setup_mysql57.bat first.
    exit /b 1
)

mkdir "%DATA%" 2>nul
mkdir "%LOGS%" 2>nul

rem Generate my.ini with the current project path.
rem MySQL needs absolute basedir/datadir paths, so this is created at runtime.
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$root = '%ROOT%'.TrimEnd('\').Replace('\','/');" ^
  "$ini = \"[mysqld]`r`nbasedir=$root/mysql57/server`r`ndatadir=$root/mysql57/data`r`nport=3307`r`nbind-address=127.0.0.1`r`ncharacter-set-server=utf8mb4`r`ncollation-server=utf8mb4_unicode_ci`r`ndefault-storage-engine=INNODB`r`nsql-mode=NO_ENGINE_SUBSTITUTION`r`nlog-error=$root/mysql57/logs/mysql57.err`r`n`r`n[client]`r`nhost=127.0.0.1`r`nport=3307`r`ndefault-character-set=utf8mb4`r`n\";" ^
  "Set-Content -LiteralPath '%MYINI%' -Value $ini -Encoding ASCII"

rem Initialize the data directory only once.
rem --initialize-insecure creates the system tables with an empty root password.
if not exist "%DATA%\mysql" (
    echo [INFO] Initializing MySQL 5.7 data directory...
    "%MYSQLD%" --defaults-file="%MYINI%" --initialize-insecure
    if errorlevel 1 exit /b 1
)

rem Start the server in the background if it is not already running.
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$p = Get-Process mysqld -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq '%MYSQLD%' }; if (-not $p) { Start-Process -FilePath '%MYSQLD%' -ArgumentList '--defaults-file=\"%MYINI%\"' -WindowStyle Hidden }"

echo [INFO] Waiting for MySQL 5.7 on 127.0.0.1:3307...
for /l %%i in (1,1,40) do (
    "%MYSQL%" --defaults-file="%MYINI%" -uroot -proot -e "SELECT VERSION();" >nul 2>nul
    if not errorlevel 1 goto ready

    "%MYSQL%" --defaults-file="%MYINI%" -uroot -e "SELECT VERSION();" >nul 2>nul
    if not errorlevel 1 goto first_password

    timeout /t 1 /nobreak >nul
)

echo [ERROR] MySQL 5.7 did not start. Check mysql57\logs\mysql57.err
exit /b 1

:first_password
rem After first initialization, root has no password. Set it to root/root.
rem The C source uses root/root, so the server account must match it.
echo [INFO] Setting root password to root...
"%MYSQL%" --defaults-file="%MYINI%" -uroot -e "ALTER USER 'root'@'localhost' IDENTIFIED BY 'root'; CREATE USER IF NOT EXISTS 'root'@'127.0.0.1' IDENTIFIED BY 'root'; GRANT ALL PRIVILEGES ON *.* TO 'root'@'127.0.0.1' WITH GRANT OPTION; CREATE DATABASE IF NOT EXISTS monster_rogue DEFAULT CHARACTER SET utf8mb4; FLUSH PRIVILEGES;"
if errorlevel 1 exit /b 1
goto ready

:ready
rem Ensure the game database exists before the game connects.
"%MYSQL%" --defaults-file="%MYINI%" -uroot -proot -e "CREATE DATABASE IF NOT EXISTS monster_rogue DEFAULT CHARACTER SET utf8mb4;"
echo [OK] MySQL 5.7 is running on 127.0.0.1:3307 root/root
