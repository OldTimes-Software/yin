@echo off
for %%I in (base\*.pms) do "%~dp0runtime\win32-x86_64\pkgman.exe" %%I
pause
