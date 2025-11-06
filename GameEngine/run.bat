@echo off
REM ===============================================
REM          Windows Build and Run Script
REM ===============================================

if exist build\projects\App\Release\Editor.exe (
    echo Application found...
    cd build

) else (
    echo No previous build found....
    echo Proceed to build project...
    echo.
    echo ===============================================
    echo           Build and Run RuneHolders
    echo ===============================================
    echo.

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
        echo ERROR: Visual Studio Build Tools not found in PATH
        echo Make sure you have Visual Studio or Build Tools installed
        pause
        exit /b 1
    )

    REM Clean previous build
    echo [1/4] Cleaning previous build...
    if exist build (
        rmdir /s /q build
        echo    Previous build directory removed
    ) else (
        echo    No previous build found
    )

    REM Create build directory
    echo [2/4] Creating build directory...
    mkdir build
    cd build

    REM Importing Dependencies
    cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 
    echo [3/4] Configuring Visual Studio with CMake...
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

    REM Build the project
    echo [4/4] Building project...
    cmake --build . --config Release
    if %errorlevel% neq 0 (
        echo ERROR: Build failed
        cd ..
        pause
        exit /b 1
    )

    echo.
    echo ===============================================
    echo   Build completed successfully!
    echo ===============================================
    echo.
)

REM Run the Imgui editor
echo Running Imgui Editor...
echo.
cd projects\App\Release
Editor.exe
if %errorlevel% neq 0 (
    echo.
    echo Application exited with error code: %errorlevel%
) else (
    echo.
    echo Application completed successfully!
)

echo.
echo Press any key to exit...
pause >nul
