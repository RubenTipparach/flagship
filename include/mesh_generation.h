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

// Generate terrain mesh from height map with vertex colors based on height
Mesh GenMeshTerrainFromHeightMap(const TerrainData* terrain, float scale, float heightScale);

// Generate a cube projected to sphere with dynamic tessellation
Mesh GenMeshCubeSphere(float radius, int subdivisions, Vector3 center);

// Calculate subdivision level based on camera distance
int CalculateSubdivisionLevel(Vector3 sphereCenter, Vector3 cameraPosition, float radius, int maxSubdivisions);

// Generate a subdivided cube that can morph towards a sphere
Mesh GenMeshSubdividedCube(float size, int subdivisions, float morphFactor);

// Generate cube with terrain height map displacement on each face
Mesh GenMeshTerrainCube(float size, int subdivisions, const TerrainData* terrain, float heightScale);

// Generate cube with terrain displacement that can morph towards a sphere
Mesh GenMeshTerrainCubeMorphing(float size, int subdivisions, const TerrainData* terrain, float heightScale, float morphFactor);

#endif // MESH_GENERATION_H