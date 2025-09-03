#ifndef MESH_GENERATION_H
#define MESH_GENERATION_H

#include "raylib.h"
#include "game_types.h"

// Generate a custom floor mesh with vertex colors and lighting
Mesh GenMeshFloorWithColors(float width, float height, int resX, int resZ);

// Generate floor mesh with advanced lighting
Mesh GenMeshFloorWithAdvancedLighting(float width, float height, int resX, int resZ, const LightingSystem* lighting, const GraphicsConfig* config);

// Generate wall mesh with vertex colors and lighting
Mesh GenMeshWallWithColors(float width, float height, int resX, int resY);

// Generate wall mesh with advanced lighting
Mesh GenMeshWallWithAdvancedLighting(float width, float height, int resX, int resY, const LightingSystem* lighting, const GraphicsConfig* config);

// Generate a cube mesh for maze walls with vertex colors and lighting
Mesh GenMeshMazeWallCube(float size, const LightingSystem* lighting, const GraphicsConfig* config);

#endif // MESH_GENERATION_H