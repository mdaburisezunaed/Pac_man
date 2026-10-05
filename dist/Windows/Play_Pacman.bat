@echo off
title Pac-Man Arcade - Developer: Md. Abu Rise Zunaed
echo ========================================================
echo   PAC-MAN ARCADE EDITION
echo   Developer: Md. Abu Rise Zunaed
echo ========================================================
echo.

if exist "pacman.exe" (
    echo Starting Pac-Man...
    start "" pacman.exe
    exit /b 0
)

echo pacman.exe not found in current folder. Attempting one-click build...
where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] MinGW / g++ compiler was not detected in PATH.
    echo Please run build_windows.bat inside MSYS2 UCRT64 terminal,
    echo or open web/index.html in Chrome/Edge to play instantly without building!
    pause
    exit /b 1
)

echo Compiling main.cpp...
g++ -std=c++17 -O2 ../../main.cpp -lmingw32 -lSDL2main -lSDL2 -mwindows -o pacman.exe
if %ERRORLEVEL% EQU 0 (
    echo Build successful! Launching Pac-Man...
    start "" pacman.exe
) else (
    echo Build failed. Please see README_WINDOWS.txt.
    pause
)
