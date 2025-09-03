# FPS Cube World

A 3D FPS game built with Raylib featuring procedural geometry, vertex lighting, scene system, and terrain generation.

## Features

- **Scene System**: Switch between multiple 3D environments
  - Maze Scene: Navigate through ASCII maze files
  - Terrain Scene: Explore procedurally generated island landscapes
- **Height Map Terrain**: Load 1024x1024 PNG height maps for realistic terrain
- **Vertex Shading**: Height-based terrain coloring (water → sand → grass → rock → snow)
- **Island Generation Tool**: Procedural island height map generator
- **First-person controls**: WASD movement with mouse look
- **Dynamic lighting**: Multiple light types (directional, point, spot)
- **Cross-platform compatibility**: Linux, Windows, macOS, Raspberry Pi

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
make                    # Build the game
make run                # Build and run
make gles               # Force OpenGL ES build (Raspberry Pi)
make clean              # Clean build files  
make setup              # Download and build raylib
make generate-heightmap # Generate height map for terrain
```

**Windows (MinGW/MSYS2):**
```bash
make -f Makefile.win     # Build for Windows
make -f Makefile.win run # Build and run
```

## Controls

### Movement
- **WASD**: Move around
- **Mouse**: Look around (when cursor is locked)
- **TAB**: Lock/unlock cursor for mouse look

### Scene Controls
- **1**: Switch to Maze Scene
- **2**: Switch to Terrain Scene

### Terrain Controls (in Terrain Scene)
- **+/=**: Increase terrain height
- **-**: Decrease terrain height
- Terrain starts flat and rebuilds dynamically as you adjust height

### Graphics Options
- **F1**: Toggle antialiasing
- **F2**: Cycle wireframe thickness
- **F3**: Toggle high quality rendering
- **F4**: Toggle advanced shading
- **F5**: Cycle specular strength

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

## Height Map Terrain

### Generating Height Maps
Use the built-in height map generator:
```bash
make generate-heightmap    # Generates heightmap.png (1024x1024)
```

The generator creates procedural island terrain using Perlin noise with:
- **Multiple noise octaves**: Large terrain features, medium hills, and small surface details
- **Island shape**: Higher center, lower edges with smooth falloff
- **Smooth transitions**: No blocky artifacts, bilinear interpolation
- **Varied scales**: Large (2.0), medium (8.0), and fine (32.0) detail levels
- **Center elevation**: Guaranteed higher center than corners
- **Random seed**: Pass a seed as argument: `./tools/heightmap_generator 12345`

### Using Custom Height Maps
1. Create or obtain a 1024x1024 grayscale PNG image
2. Name it `heightmap.png` and place in the game directory  
3. **White pixels (255) = Highest elevation, Black pixels (0) = Lowest elevation**
4. Center should be lighter than edges for proper island shape
5. Switch to Terrain Scene (press **2**) to see your terrain

### Terrain Features
- **Square plane**: 102.4x102.4 unit terrain centered at origin (0,0,0)
- **Height-based displacement**: Vertices offset vertically up to 5 units maximum height
- **Gradual height-based coloring**: Smooth color transitions based on actual terrain maximum height
  - **Deep Blue**: Lowest areas (deep water)
  - **Light Blue**: Shallow water
  - **Light Tan**: Beach/sand areas
  - **Green**: Grass and vegetation
  - **Dark Green**: Dense vegetation
  - **Brown**: Rocky mountain areas
  - **White**: Snow-covered peaks (highest elevation)
  - Colors adapt dynamically to actual terrain height range
- **Smooth normals**: Calculated per-vertex for realistic lighting

## Architecture

- **Scene System**: Modular scene management with init/update/render/cleanup
- **Vertex Shading**: Height-based terrain coloring with smooth transitions
- **Procedural Generation**: Runtime terrain generation from height data
- **Dynamic Lighting**: Multiple light sources with proper vertex lighting
- **Cross-platform**: Consistent experience across all supported platforms

## File Structure

```
src/                          # Source files
├── fps_game.c               # Main game loop and initialization
├── scene_manager.c          # Scene system implementation
├── terrain_mesh.c           # Terrain mesh generation from height maps
├── lighting.c               # Dynamic lighting system
├── mesh_generation.c        # Basic mesh generation functions
├── mesh_generation_advanced.c  # Advanced lighting mesh generation
├── rendering.c              # Custom rendering utilities
└── maze.c                   # ASCII maze file loading

include/                      # Header files
├── game_types.h             # Core data structures (scenes, terrain, lighting)
├── scene_manager.h          # Scene management function declarations
├── lighting.h               # Lighting system definitions
├── mesh_generation.h        # Mesh generation function declarations
├── rendering.h              # Custom rendering function declarations
└── maze.h                   # Maze loading function declarations

tools/                        # Utilities
└── heightmap_generator.c    # Procedural island height map generator

Makefile                     # Linux/macOS/Pi build with terrain tools
Makefile.win                 # Windows build  
setup.sh                     # Raylib setup script (Linux/macOS)
setup.bat                    # Raylib setup script (Windows)
maze.txt                     # Sample ASCII maze file
heightmap.png                # Generated terrain height map (1024x1024)
.gitignore                   # Excludes raylib and build files
README.md                    # This documentation
raylib/                      # Raylib library (downloaded by setup, gitignored)
```