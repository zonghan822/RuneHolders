@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

REM ===============================================
REM        Windows Build and Run Script
REM ===============================================

:CHOOSE_CONFIG
echo ===============================================
echo       Choose build configuration:
echo ===============================================
echo   1. Debug
echo   2. Release
echo ===============================================
set /p userChoice="Enter choice (1 or 2): "

if "%userChoice%"=="1" (
    set CONFIG=Debug
) else if "%userChoice%"=="2" (
    set CONFIG=Release
) else (
    echo Invalid choice. Try again.
    echo.
    goto CHOOSE_CONFIG
)

echo.
echo Selected configuration: %CONFIG%
echo.

REM Detect existing executable
if exist build\projects\App\%CONFIG%\Editor.exe (
    echo Application found for %CONFIG%
    cd build

) else (
    echo No existing %CONFIG% build found...
    echo Proceeding to configure and build project...
    echo.

    REM ---------- CHECK CMAKE ----------
    cmake --version >nul 2>&1
    if %errorlevel% neq 0 (
        echo ERROR: CMake is not installed or not in PATH
        pause
        exit /b 1
    )

    REM ---------- CHECK MSVC ----------
    where cl >nul 2>&1
    if %errorlevel% neq 0 (
        echo ERROR: Visual Studio Build Tools not found
        pause
        exit /b 1
    )

    REM ---------- CREATE BUILD DIRECTORY ----------
    if not exist build (mkdir build)
    cd build

    REM ---------- CMAKE CONFIGURE ----------
    echo Configuring project using CMake...
    cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5

    if %errorlevel% neq 0 (
        echo ERROR: CMake configuration failed!
        cd ..
        pause
        exit /b 1
    )

    REM ---------- BUILD ----------
    echo Building project %CONFIG%
    cmake --build . --config %CONFIG%

    if %errorlevel% neq 0 (
        echo ERROR: Build failed!
        cd ..
        pause
        exit /b 1
    )

    echo Build completed successfully!
)

REM ---------- RUN EDITOR ----------
echo.
echo Running ImGui Editor in %CONFIG% mode...
cd projects\App\%CONFIG%
Editor.exe

if %errorlevel% neq 0 (
    echo Application exited with error code: %errorlevel%
) else (
    echo Application completed successfully!
)

echo.
echo Press any key to exit...
pause >nul
