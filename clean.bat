@echo off
setlocal enabledelayedexpansion

echo Cleaning all build artifacts...

:: Check if cmake is available
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake is not found in PATH
    echo Please install CMake and add it to your PATH
    exit /b 1
)

:: Run CMake clean-all script
cmake -P cmake/clean-all.cmake

:: Additional cleanup for Visual Studio files
if exist .vs rmdir /s /q .vs
if exist build32 rmdir /s /q build32
if exist build64 rmdir /s /q build64
if exist dist rmdir /s /q dist

:: Clean generated files
if exist include\version.h del /f /q include\version.h

:: Remove any temporary files
for /r %%f in (*.tmp *.bak *~) do del /f /q "%%f"

echo.
echo Cleanup completed successfully!
echo.

endlocal