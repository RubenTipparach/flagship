#!/bin/bash

echo "FPS Game Setup Script"
echo "===================="

# Install gcc compiler if not available
if ! command -v gcc &> /dev/null; then
    echo "GCC compiler not found. Installing..."
    
    if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
        # Windows - just provide instructions
        echo "Please install MinGW-w64 and ensure it's in your PATH:"
        echo "Option 1: Install via Chocolatey: choco install mingw"
        echo "Option 2: Install via Scoop: scoop install mingw"  
        echo "Option 3: Download from: https://www.mingw-w64.org/downloads/"
        echo "Option 4: Install MSYS2 from: https://www.msys2.org/"
        echo ""
        echo "Or temporarily add to PATH: export PATH=\"/c/msys64/mingw64/bin:\$PATH\""
        exit 1
    elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
        # Linux
        if command -v apt-get &> /dev/null; then
            sudo apt-get update && sudo apt-get install -y gcc
        elif command -v yum &> /dev/null; then
            sudo yum install -y gcc
        elif command -v dnf &> /dev/null; then
            sudo dnf install -y gcc
        elif command -v pacman &> /dev/null; then
            sudo pacman -S --noconfirm gcc
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        if command -v brew &> /dev/null; then
            brew install gcc
        else
            echo "Please install Xcode Command Line Tools: xcode-select --install"
        fi
    fi
    
    # Verify installation
    if command -v gcc &> /dev/null; then
        echo "✓ GCC installed successfully"
        gcc --version
    else
        echo "✗ GCC installation failed or requires PATH update"
        echo "Please restart your terminal and try again"
        exit 1
    fi
else
    echo "✓ GCC compiler found"
fi

# Install required dependencies for GLFW on Linux
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Installing X11 development libraries for GLFW..."
    
    # Check if we're on a Debian/Ubuntu system
    if command -v apt-get &> /dev/null; then
        sudo apt-get update
        sudo apt-get install -y libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
    # Check if we're on a Red Hat/Fedora system
    elif command -v yum &> /dev/null; then
        sudo yum install -y libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel mesa-libGL-devel
    elif command -v dnf &> /dev/null; then
        sudo dnf install -y libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel mesa-libGL-devel
    # Check if we're on an Arch system
    elif command -v pacman &> /dev/null; then
        sudo pacman -S --noconfirm libx11 libxrandr libxinerama libxcursor libxi mesa
    else
        echo "Warning: Unknown package manager. Please install X11 development libraries manually."
        echo "Required packages: libx11-dev, libxrandr-dev, libxinerama-dev, libxcursor-dev, libxi-dev, libgl1-mesa-dev"
    fi
    
    if [ $? -eq 0 ]; then
        echo "✓ X11 development libraries installed successfully"
    else
        echo "✗ Failed to install X11 development libraries"
        echo "Please install them manually and run this script again"
        exit 1
    fi
fi

# Check if raylib directory exists
if [ ! -d "raylib" ]; then
    echo "Raylib not found. Cloning raylib repository..."
    git clone https://github.com/raysan5/raylib.git
    
    if [ $? -eq 0 ]; then
        echo "✓ Raylib cloned successfully"
    else
        echo "✗ Failed to clone raylib"
        exit 1
    fi
else
    echo "✓ Raylib directory already exists"
fi

# Build raylib
echo "Building raylib..."
cd raylib/src

# Detect platform and build accordingly
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Building for Linux..."
    make PLATFORM=PLATFORM_DESKTOP GRAPHICS=GRAPHICS_API_OPENGL_ES2
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    echo "Building for Windows..."
    make PLATFORM=PLATFORM_DESKTOP CC=gcc
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Building for macOS..."
    make PLATFORM=PLATFORM_DESKTOP
else
    echo "Unknown platform: $OSTYPE"
    echo "Attempting default build..."
    make PLATFORM=PLATFORM_DESKTOP
fi

if [ $? -eq 0 ]; then
    echo "✓ Raylib built successfully"
else
    echo "✗ Failed to build raylib"
    exit 1
fi

cd ../..

echo ""
echo "Setup complete! You can now run:"
echo "  make        - Build for current platform"
echo "  make run    - Build and run the game"
echo "  make clean  - Clean build files"