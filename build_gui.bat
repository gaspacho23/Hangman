@echo off
rem Сборка и запуск графической версии. Запускать из корня проекта.
set PATH=C:\msys64\ucrt64\bin;%PATH%
g++ -std=c++17 -Wall -Wextra -Iinclude src/gui_main.cpp src/Dictionary.cpp src/GameSession.cpp -o hangman_gui.exe -lsfml-graphics -lsfml-window -lsfml-system
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)
hangman_gui.exe
