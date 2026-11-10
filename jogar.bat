@echo off
cd /d "%~dp0"
if exist piu.exe (piu.exe -game ..\game) else (PiuReconstructed.exe -game ..\game)
