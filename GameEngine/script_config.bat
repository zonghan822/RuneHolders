@echo off
REM ===============================================
REM Windows Build and Run Script
REM ===============================================

echo.
echo ==============================================
echo   Configuring RuneHolders for Visual Studio
echo ==============================================

REM Create build directory
if not exist build mkdir build
cd build

REM Importing Dependencies
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5
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