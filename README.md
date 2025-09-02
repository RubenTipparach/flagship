# FPS Cube World

A 3D FPS game built with Raylib featuring procedural geometry, vertex lighting, and cross-platform compatibility.

## Features

- First-person camera with mouse look controls  
- Procedural floor and wall geometry with vertex colors
- Dynamic lighting system based on sun position
- Various 3D shapes (cubes, spheres, cylinders) with proper shading
- Cross-platform compatibility (Linux, Windows, macOS, Raspberry Pi)

## Requirements

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential git
# For OpenGL ES (Raspberry Pi):
sudo apt install libgles2-mesa-dev libegl1-mesa-dev
```

### Windows (MSYS2/MinGW)
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make git
```

### macOS
```bash
xcode-select --install  # Install Xcode Command Line Tools
```

## Building

### First Time Setup
Run the setup script to download and build Raylib:
```bash
./setup.sh
```

### Building the Game

**Linux/macOS/Raspberry Pi:**
```bash
make           # Build the game
make run       # Build and run
make gles      # Force OpenGL ES build (Raspberry Pi)
make clean     # Clean build files  
make setup     # Download and build raylib
```

**Windows (MinGW/MSYS2):**
```bash
make -f Makefile.win     # Build for Windows
make -f Makefile.win run # Build and run
```

## Controls

- **WASD**: Move around
- **Mouse**: Look around (when cursor is locked)
- **TAB**: Lock/unlock cursor for mouse look
- **ESC**: Exit game

## Code Compatibility

The code is **fully cross-platform compatible**:

### ✅ Windows Compatible:
- Uses standard C99 and Raylib APIs
- Windows Makefile provided (`Makefile.win`)
- Links against `opengl32.dll`, `gdi32.dll`, `winmm.dll`

### ✅ Linux/Raspberry Pi Compatible:
- OpenGL and OpenGL ES2 support
- X11 window system integration
- ARM64 optimized

### ✅ Cross-Platform Features:
- Standard C math functions (`sinf`, `cosf`, `atan2f`)
- Raylib handles all platform-specific code
- Memory management through Raylib APIs
- Consistent input/graphics across platforms

## Architecture

- **Vertex Lighting**: Custom per-vertex lighting calculations
- **Procedural Geometry**: Runtime generation of floors, walls, and shapes  
- **Sun-based Lighting**: All surfaces lit relative to sun at (50, 100, 50)
- **Mouse Look**: Spherical coordinate system for smooth rotation

## File Structure

```
fps_game.c          # Main game source
Makefile           # Linux/macOS/Pi build
Makefile.win       # Windows build  
setup.sh           # Raylib setup script
.gitignore         # Excludes raylib and build files
README.md          # This file
raylib/            # Downloaded by setup.sh (gitignored)
```