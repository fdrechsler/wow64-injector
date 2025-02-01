@echo off
setlocal enabledelayedexpansion

:: Check if cmake is available
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake is not found in PATH
    echo Please install CMake and add it to your PATH
    exit /b 1
)

:: Check if Visual Studio is installed
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" (
    echo ERROR: Visual Studio 2019 or later is required
    exit /b 1
)

:: Get Visual Studio installation path
for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    set "MSBUILD=%%i"
)

if not defined MSBUILD (
    echo ERROR: MSBuild not found
    exit /b 1
)

:: Create build directories for both architectures
if not exist "build32" mkdir build32
if not exist "build64" mkdir build64

:: Build 32-bit version
echo Building 32-bit version...
cd build32
cmake .. -A Win32
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed for 32-bit build
    exit /b 1
)
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed for 32-bit version
    exit /b 1
)
cd ..

:: Build 64-bit version
echo Building 64-bit version...
cd build64
cmake .. -A x64
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed for 64-bit build
    exit /b 1
)
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed for 64-bit version
    exit /b 1
)
cd ..

:: Create dist directory and copy binaries
if not exist "dist" mkdir dist
copy /Y "build32\bin\Release\injector32.exe" "dist\"
copy /Y "build64\bin\Release\injector64.exe" "dist\"

echo.
echo Build completed successfully!
echo Binaries are located in the 'dist' directory:
echo - dist\injector32.exe (32-bit version)
echo - dist\injector64.exe (64-bit version)
echo.

endlocal