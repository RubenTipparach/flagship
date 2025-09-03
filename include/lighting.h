#ifndef LIGHTING_H
#define LIGHTING_H

#include "raylib.h"
#include "raymath.h"
#include "game_types.h"

// Initialize the lighting system
LightingSystem InitLightingSystem(void);

// Add a light to the lighting system
void AddLight(LightingSystem* lighting, LightType type, Vector3 position, Vector3 direction, Color color, float intensity, float range, float spotAngle);

// Update lighting system (for dynamic lights)
void UpdateLightingSystem(LightingSystem* lighting, float deltaTime);

// Calculate lighting for a vertex with multiple lights
Color CalculateVertexLighting(Vector3 vertexPos, Vector3 normal, Vector3 viewDir, Color baseColor, const LightingSystem* lighting, const GraphicsConfig* config);

// Calculate simple lighting for backward compatibility
Color CalculateSimpleLighting(Vector3 vertexPos, Vector3 normal, Color baseColor);

// Draw all light sources in the scene
void DrawLights(const LightingSystem* lighting);

// Draw sun/light source in the scene (legacy)
void DrawSunLight(void);

#endif // LIGHTING_H