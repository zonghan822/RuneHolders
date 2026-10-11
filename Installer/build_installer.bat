@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

REM ===============================================
REM     Build RuneWeavers_Setup.exe from source
REM ===============================================
REM 1. Runs GameEngine\ship.bat (Release build + package)
REM 2. Mirrors the packaged game into GAMEDIRECTORY (keeps the tracked Icon.ico)
REM 3. Downloads the VC++ redistributable into REDIST if missing
REM 4. Compiles InstallScript.iss with Inno Setup 6
REM Output: INSTALLER\RuneWeavers_Setup.exe

set INSTALLER_DIR=%~dp0
set ENGINE_DIR=%INSTALLER_DIR%..\GameEngine
set SHIP_OUTPUT=%ENGINE_DIR%\RuneWeavers
set GAME_DIR=%INSTALLER_DIR%GAMEDIRECTORY
set REDIST_EXE=%INSTALLER_DIR%REDIST\VC_redist.x64.exe

echo.
echo ===============================================
echo       Building RuneWeavers installer
echo ===============================================
echo.

REM ---------- FIND INNO SETUP ----------
set ISCC=
for %%p in ("%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" "%ProgramFiles%\Inno Setup 6\ISCC.exe") do (
    if exist %%p set ISCC=%%~p
)
if not defined ISCC (
    for /f "delims=" %%p in ('where iscc 2^>nul') do set ISCC=%%p
)
if not defined ISCC (
    echo ERROR: Inno Setup 6 not found. Install it from https://jrsoftware.org/isdl.php
    pause
    exit /b 1
)

REM ---------- BUILD AND PACKAGE THE GAME ----------
REM ship.bat skips the build when Release executables already exist;
REM run GameEngine\clean.bat first to force a fresh build.
REM Input is redirected from nul so ship.bat's closing pause doesn't block.
pushd "%ENGINE_DIR%"
call "%ENGINE_DIR%\ship.bat" < nul
popd

if not exist "%SHIP_OUTPUT%\RuneWeavers.exe" (
    echo ERROR: ship.bat did not produce %SHIP_OUTPUT%\RuneWeavers.exe
    pause
    exit /b 1
)

REM ---------- COPY INTO GAMEDIRECTORY ----------
echo.
echo Copying packaged game into GAMEDIRECTORY...
robocopy "%SHIP_OUTPUT%" "%GAME_DIR%" /MIR /XF Icon.ico /NFL /NDL /NJH /NJS /nc /ns /np
if !errorlevel! gtr 7 (
    echo ERROR: Failed to copy the packaged game!
    pause
    exit /b 1
)
if not exist "%GAME_DIR%\Icon.ico" (
    echo ERROR: GAMEDIRECTORY\Icon.ico is missing. Restore it with: git checkout -- Installer/GAMEDIRECTORY/Icon.ico
    pause
    exit /b 1
)

REM ---------- VC++ REDISTRIBUTABLE ----------
if not exist "%REDIST_EXE%" (
    echo.
    echo Downloading VC++ redistributable...
    if not exist "%INSTALLER_DIR%REDIST" mkdir "%INSTALLER_DIR%REDIST"
    curl -L --fail -o "%REDIST_EXE%" https://aka.ms/vs/17/release/vc_redist.x64.exe
    if !errorlevel! neq 0 (
        echo ERROR: Failed to download VC_redist.x64.exe
        pause
        exit /b 1
    )
)

REM ---------- COMPILE INSTALLER ----------
echo.
echo Compiling installer with Inno Setup...
pushd "%INSTALLER_DIR%"
"%ISCC%" /Q InstallScript.iss
set ISCC_RESULT=!errorlevel!
popd
if !ISCC_RESULT! neq 0 (
    echo ERROR: Inno Setup compilation failed!
    pause
    exit /b 1
)

echo.
echo ===============================================
echo   Installer created successfully:
echo   %INSTALLER_DIR%INSTALLER\RuneWeavers_Setup.exe
echo ===============================================
echo.

echo Press any key to exit...
pause >nul
