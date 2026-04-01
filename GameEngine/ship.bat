@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

REM ===============================================
REM       Ship RuneHolders - Release Only
REM ===============================================

echo.
echo ===============================================
echo       Shipping RuneHolders (Release)
echo ===============================================
echo.

set BUILD_DIR=build
set BUILD_OUTPUT=%BUILD_DIR%\projects\App\Release\RuneWeavers
set SHIP_DIR=RuneWeavers

set LAUNCHER_OUTPUT=%BUILD_DIR%\projects\App\Release\RuneLauncher\RuneLauncher.exe

REM ---------- BUILD (skip if both exes already exist) ----------
if exist "%BUILD_OUTPUT%\RuneWeavers.exe" (
    if exist "%LAUNCHER_OUTPUT%" (
        echo Release builds found, skipping build step.
        echo.
        goto PACKAGE
    )
)

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
if not exist %BUILD_DIR% (mkdir %BUILD_DIR%)
cd %BUILD_DIR%

REM ---------- CMAKE CONFIGURE ----------
echo Configuring project...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5

if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

REM ---------- BUILD RUNEWEAVERS RELEASE ----------
echo.
echo Building RuneWeavers in Release mode...
cmake --build . --config Release --target RuneWeavers

if %errorlevel% neq 0 (
    echo ERROR: RuneWeavers build failed!
    cd ..
    pause
    exit /b 1
)

REM ---------- BUILD RUNELAUNCHER RELEASE ----------
echo.
echo Building RuneLauncher in Release mode...
cmake --build . --config Release --target RuneLauncher

if %errorlevel% neq 0 (
    echo ERROR: RuneLauncher build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
cd ..

:PACKAGE
REM ---------- PACKAGE ----------
echo.
echo ===============================================
echo             Packaging ship output
echo ===============================================
echo.

REM Clean previous ship output
if exist %SHIP_DIR% (
    echo Removing previous ship folder...
    rmdir /s /q %SHIP_DIR%
)
mkdir %SHIP_DIR%

REM Copy everything from the Release build output, excluding dev-only files and
REM Roslyn locale subdirectories (cs, de, es, fr, it, ja, ko, pl, pt-BR, ru, tr, zh-Hans, zh-Hant)
echo Copying build output...
robocopy "%BUILD_OUTPUT%" "%SHIP_DIR%" /E /XF Editor.exe *.pdb imgui.ini /XD cs de es fr it ja ko pl pt-BR ru tr zh-Hans zh-Hant /NFL /NDL /NJH /NJS /nc /ns /np
if %errorlevel% gtr 7 (
    echo ERROR: Failed to copy build output!
    pause
    exit /b 1
)

REM Copy RuneLauncher.exe
echo Copying RuneLauncher...
copy /Y "%BUILD_DIR%\projects\App\Release\RuneLauncher\RuneLauncher.exe" "%SHIP_DIR%\RuneLauncher.exe" >nul
if %errorlevel% neq 0 (
    echo WARNING: RuneLauncher.exe not found, skipping.
)

REM Copy assets directly from source
echo Copying assets...
robocopy "assets\RuneWeavers" "%SHIP_DIR%\assets" /E /NFL /NDL /NJH /NJS /nc /ns /np
if %errorlevel% gtr 7 (
    echo ERROR: Failed to copy assets!
    pause
    exit /b 1
)

REM Copy scripts
echo Copying scripts...
robocopy "scripts\RuneWeavers\user_scripts" "%SHIP_DIR%\scripts\user_scripts" /E /NFL /NDL /NJH /NJS /nc /ns /np
if %errorlevel% gtr 7 (
    echo ERROR: Failed to copy scripts!
    pause
    exit /b 1
)

echo.
echo ===============================================
echo   Ship package created successfully in:
echo   %SHIP_DIR%\
echo ===============================================
echo.

echo Press any key to exit...
pause >nul
