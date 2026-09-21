@echo off
set PATH=C:\msys64\ucrt64\bin;%PATH%

echo ========================================================
echo   Compiling Snake on a Cube (C++ / FreeGLUT / OpenGL)
echo ========================================================

g++ -O2 -std=c++17 ^
    src/main.cpp ^
    src/cube_topology.cpp ^
    src/game_state.cpp ^
    src/camera.cpp ^
    src/ui.cpp ^
    src/renderer.cpp ^
    -o main.exe ^
    -lfreeglut -lopengl32 -lglu32

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Build succeeded! main.exe is ready.
    echo Launching game...
    start main.exe
) else (
    echo.
    echo [ERROR] Compilation failed! Check error messages above.
)
