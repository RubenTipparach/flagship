#ifndef RENDERING_H
#define RENDERING_H

#include "raylib.h"
#include "rlgl.h"
#include "game_types.h"

// Custom wireframe rendering function with thickness and antialiasing support
void DrawCubeWiresThick(Vector3 position, float width, float height, float length, Color color, GraphicsConfig* config);

// Draw cube-sphere wireframe with dynamic tessellation
void DrawCubeSphereWires(Vector3 center, float radius, int subdivisions, Color color, GraphicsConfig* config);

#endif // RENDERING_H