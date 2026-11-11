@echo off
cd /d "%~dp0"
if exist piu.exe (start "" piu.exe -game ..\game) else (start "" PiuReconstructed.exe -game ..\game)
