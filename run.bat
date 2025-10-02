@echo off

REM === Step 1: Compile main.cpp into file_system.exe ===
g++ -std=c++17 -O2 -Wall main.cpp -o file_system.exe

REM === Step 2: Check if compilation failed ===
IF ERRORLEVEL 1 (
    echo Compilation failed!
    pause
    exit /b
)

REM === Step 3:  Run the program ===
file_system.exe
