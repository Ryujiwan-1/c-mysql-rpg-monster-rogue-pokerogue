@echo off
setlocal

"%~dp0mysql57\server\bin\mysql.exe" --defaults-file="%~dp0mysql57\my.ini" -uroot -proot
