@echo off
setlocal EnableDelayedExpansion

echo FPS Game Setup Script (Windows)
echo ==================================

:: Check if gcc is available
gcc --version >nul 2>&1
if %errorlevel% neq 0 (
    echo GCC compiler not found. Checking common MinGW-w64 locations...
    
    :: Try common MinGW-w64 installation paths
    set "mingw_paths=C:\msys64\mingw64\bin;C:\mingw64\bin;C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin;C:\MinGW\bin"
    
    for %%p in (!mingw_paths!) do (
        if exist "%%p\gcc.exe" (
            echo Found GCC at: %%p
            set "PATH=%%p;!PATH!"
            goto :gcc_found
        )
    )
    
    echo MinGW-w64 not found in common locations.
    echo Please install MinGW-w64 or add it to your PATH:
    echo.
    echo Option 1: Install via Chocolatey: choco install mingw
    echo Option 2: Install via Scoop: scoop install mingw  
    echo Option 3: Download MSYS2 from: https://www.msys2.org/
    echo Option 4: Download MinGW-w64 from: https://www.mingw-w64.org/downloads/
    echo.
    pause
    exit /b 1
)

:gcc_found
echo ✓ GCC compiler found
gcc --version | findstr "gcc"

:: Check and setup OpenGL libraries
echo.
echo Checking OpenGL libraries...

:: Check for OpenGL32.lib in common Windows SDK locations
set "opengl_found=0"
set "sdk_paths=C:\Program Files (x86)\Windows Kits\10\Lib\*\um\x64;C:\Program Files (x86)\Windows Kits\10\Lib\*\um\x86;C:\Program Files (x86)\Microsoft SDKs\Windows\*\Lib\x64;C:\Program Files (x86)\Microsoft SDKs\Windows\*\Lib"

for %%p in (!sdk_paths!) do (
    if exist "%%p\opengl32.lib" (
        echo ✓ Found OpenGL32.lib at: %%p
        set "opengl_found=1"
        goto :opengl_found
    )
)

:: Check MinGW OpenGL libraries
for %%p in (!mingw_paths!) do (
    if exist "%%p\..\lib\libopengl32.a" (
        echo ✓ Found MinGW OpenGL libraries at: %%p\..\lib\
        set "opengl_found=1"
        goto :opengl_found
    )
)

if !opengl_found! equ 0 (
    echo ⚠ OpenGL libraries not found in standard locations.
    echo OpenGL should be available through:
    echo   - Windows SDK (usually pre-installed)
    echo   - MinGW-w64 installation
    echo.
    echo If you encounter OpenGL linking issues, please install:
    echo   - Visual Studio Build Tools with Windows SDK
    echo   - Or ensure your MinGW installation includes OpenGL libraries
    echo.
) else (
    echo ✓ OpenGL libraries found
)

:opengl_found

:: Check if raylib directory exists
if not exist "raylib" (
    echo.
    echo Raylib not found. Cloning raylib repository...
    git clone https://github.com/raysan5/raylib.git
    
    if %errorlevel% neq 0 (
        echo ✗ Failed to clone raylib
        pause
        exit /b 1
    )
    echo ✓ Raylib cloned successfully
) else (
    echo ✓ Raylib directory already exists
)

:: Build raylib
echo.
echo Building raylib for Windows...
cd raylib\src

make PLATFORM=PLATFORM_DESKTOP CC=gcc

if %errorlevel% neq 0 (
    echo ✗ Failed to build raylib
    cd ..\..
    pause
    exit /b 1
)

echo ✓ Raylib built successfully
cd ..\..

echo.
echo Setup complete! You can now run:
echo   make        - Build for Windows
echo   make run    - Build and run the game
echo   make clean  - Clean build files
echo.
pause