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

REM Clean C# build artifacts (bin/obj in all script projects)
for /d %%d in (scripts\*) do (
    if exist %%d\bin (
        rmdir /s /q %%d\bin
        echo    Removed %%d\bin
    )
    if exist %%d\obj (
        rmdir /s /q %%d\obj
        echo    Removed %%d\obj
    )
)

echo.
echo ===============================================
echo            Clean up successfully!
echo ===============================================
echo.

echo.
echo Press any key to exit...
pause >nul