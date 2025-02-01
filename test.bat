@echo off
setlocal enabledelayedexpansion

echo Running tests for WoW Lua Injector...
echo.

:: Check for admin privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script requires administrator privileges.
    echo Please run as administrator.
    exit /b 1
)

:: Check if executables exist
if not exist "dist\injector32.exe" (
    echo ERROR: injector32.exe not found.
    echo Please run build.bat first.
    exit /b 1
)

if not exist "dist\injector64.exe" (
    echo ERROR: injector64.exe not found.
    echo Please run build.bat first.
    exit /b 1
)

echo Testing 32-bit version...
echo -------------------

:: Test anti-debug
echo [TEST] Anti-debug detection...
dist\injector32.exe --test-anti-debug
if %errorLevel% equ 0 (
    echo [PASS] Anti-debug detection working
) else (
    echo [FAIL] Anti-debug detection failed
)

:: Test pattern scanning
echo [TEST] Pattern scanning...
dist\injector32.exe --test-pattern-scan
if %errorLevel% equ 0 (
    echo [PASS] Pattern scanning working
) else (
    echo [FAIL] Pattern scanning failed
)

:: Test IAT hiding
echo [TEST] IAT hiding...
dist\injector32.exe --test-iat-hiding
if %errorLevel% equ 0 (
    echo [PASS] IAT hiding working
) else (
    echo [FAIL] IAT hiding failed
)

echo.
echo Testing 64-bit version...
echo -------------------

:: Test anti-debug (64-bit)
echo [TEST] Anti-debug detection...
dist\injector64.exe --test-anti-debug
if %errorLevel% equ 0 (
    echo [PASS] Anti-debug detection working
) else (
    echo [FAIL] Anti-debug detection failed
)

:: Test pattern scanning (64-bit)
echo [TEST] Pattern scanning...
dist\injector64.exe --test-pattern-scan
if %errorLevel% equ 0 (
    echo [PASS] Pattern scanning working
) else (
    echo [FAIL] Pattern scanning failed
)

:: Test IAT hiding (64-bit)
echo [TEST] IAT hiding...
dist\injector64.exe --test-iat-hiding
if %errorLevel% equ 0 (
    echo [PASS] IAT hiding working
) else (
    echo [FAIL] IAT hiding failed
)

echo.
echo Test Summary
echo -----------
echo All tests completed. Check the results above for any failures.
echo Note: Some tests may require World of Warcraft to be running.
echo.

endlocal