@echo off
REM ===============================================
REM Windows Build and Run Script
REM ===============================================

echo.
echo ==============================================
echo   Configuring RuneHolders for Visual Studio
echo ==============================================


REM Check if CMake is available
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/download/
    pause
    exit /b 1
)

REM Check if Visual Studio Build Tools are available
where cl >nul 2>&1
if %errorlevel% neq 0 (
    echo WARNING: Visual Studio Build Tools not found in PATH
    echo Make sure you have Visual Studio or Build Tools installed
    echo.
)

REM Create build directory
if not exist build mkdir build
cd build

REM Importing Dependencies
cmake .. -G "Visual Studio 17 2022" -A x64 -DIMPORT_DEPS=ON
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo ===============================================
echo   CMake configuration completed successfully!
echo ===============================================
echo.


echo.
echo Press any key to exit...
pause >nul