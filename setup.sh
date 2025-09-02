#!/bin/bash

echo "FPS Game Setup Script"
echo "===================="

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