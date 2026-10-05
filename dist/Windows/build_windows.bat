@echo off
title Build Pac-Man Arcade - Developer: Md. Abu Rise Zunaed
echo ========================================================
echo   PAC-MAN ARCADE BUILDER FOR WINDOWS
echo   Developer: Md. Abu Rise Zunaed
echo ========================================================
echo.
echo Compiling Pac-Man with SDL2...
g++ -std=c++17 -O2 ../../main.cpp -lmingw32 -lSDL2main -lSDL2 -o pacman.exe
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================================
    echo   SUCCESS: pacman.exe created successfully!
    echo   Double-click Play_Pacman.bat or pacman.exe to play!
    echo ========================================================
) else (
    echo.
    echo [ERROR] Build failed. Make sure MSYS2 UCRT64 with gcc & sdl2 is installed.
)
pause
