#ifndef RENDERING_H
#define RENDERING_H

#include "raylib.h"
#include "rlgl.h"
#include "game_types.h"

// Wireframe shader management
typedef struct {
    Shader shader;
    bool loaded;
    int enableWireframeLoc;
    int wireframeColorLoc;
    int wireframeThicknessLoc;
} WireframeShader;

// Initialize wireframe shader system
WireframeShader LoadWireframeShader(void);

// Cleanup wireframe shader
void UnloadWireframeShader(WireframeShader* wireframeShader);

// Draw model with wireframe shader
void DrawModelWireframe(Model model, Vector3 position, float scale, Color tint, 
                       WireframeShader* wireframeShader, GraphicsConfig* config);

// Custom wireframe rendering function with thickness and antialiasing support
void DrawCubeWiresThick(Vector3 position, float width, float height, float length, Color color, GraphicsConfig* config);

// Draw cube-sphere wireframe with dynamic tessellation
void DrawCubeSphereWires(Vector3 center, float radius, int subdivisions, Color color, GraphicsConfig* config);

#endif // RENDERING_H