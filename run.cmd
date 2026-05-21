@echo off
REM Double-click launcher. Invokes run.ps1 with execution-policy bypass so
REM Windows doesn't refuse to run an unsigned local script.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0run.ps1"
pause
