#!/bin/bash

echo "FPS Game Setup Script"
echo "===================="

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