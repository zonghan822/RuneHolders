@echo off
REM ===============================================
REM                Clean up Project
REM ===============================================

echo.
echo ==============================================
echo            Cleaning up projects
echo ==============================================
echo.

if exist build (
    rmdir /s /q build
    echo    Previous build directory removed
) else (
    echo    No previous build found
)

echo.
echo ===============================================
echo            Clean up successfully!
echo ===============================================
echo.

echo.
echo Press any key to exit...
pause >nul