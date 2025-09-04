#include "rendering.h"
#include "raymath.h"
#include <math.h>

// Custom wireframe rendering function with thickness and antialiasing support
void DrawCubeWiresThick(Vector3 position, float width, float height, float length, Color color, GraphicsConfig* config)
{
    if (!config->highQualityRendering)
    {
        // Fallback to basic wireframe for low-end devices
        DrawCubeWires(position, width, height, length, color);
        return;
    }
    
    float x = position.x;
    float y = position.y;
    float z = position.z;
    float w = width/2;
    float h = height/2;
    float l = length/2;
    float thickness = config->wireframeThickness;
    
    // Enable smooth lines if antialiasing is enabled
    if (config->antialiasingEnabled)
    {
        rlSetLineWidth(thickness);
        rlEnableSmoothLines();
    }
    else
    {
        rlSetLineWidth(thickness);
    }
    
    rlBegin(RL_LINES);
    rlColor4ub(color.r, color.g, color.b, color.a);
    
    // Front face
    rlVertex3f(x-w, y-h, z+l);
    rlVertex3f(x+w, y-h, z+l);
    rlVertex3f(x+w, y-h, z+l);
    rlVertex3f(x+w, y+h, z+l);
    rlVertex3f(x+w, y+h, z+l);
    rlVertex3f(x-w, y+h, z+l);
    rlVertex3f(x-w, y+h, z+l);
    rlVertex3f(x-w, y-h, z+l);
    
    // Back face
    rlVertex3f(x-w, y-h, z-l);
    rlVertex3f(x+w, y-h, z-l);
    rlVertex3f(x+w, y-h, z-l);
    rlVertex3f(x+w, y+h, z-l);
    rlVertex3f(x+w, y+h, z-l);
    rlVertex3f(x-w, y+h, z-l);
    rlVertex3f(x-w, y+h, z-l);
    rlVertex3f(x-w, y-h, z-l);
    
    // Connecting edges
    rlVertex3f(x-w, y-h, z+l);
    rlVertex3f(x-w, y-h, z-l);
    rlVertex3f(x+w, y-h, z+l);
    rlVertex3f(x+w, y-h, z-l);
    rlVertex3f(x+w, y+h, z+l);
    rlVertex3f(x+w, y+h, z-l);
    rlVertex3f(x-w, y+h, z+l);
    rlVertex3f(x-w, y+h, z-l);
    
    rlEnd();
    
    if (config->antialiasingEnabled)
    {
        rlDisableSmoothLines();
    }
    
    rlSetLineWidth(1.0f); // Reset line width
}

// Draw cube-sphere wireframe with dynamic tessellation
void DrawCubeSphereWires(Vector3 center, float radius, int subdivisions, Color color, GraphicsConfig* config) {
    float thickness = config ? config->wireframeThickness : DEFAULT_WIREFRAME_THICKNESS;
    
    // Enable smooth lines if antialiasing is enabled
    if (config && config->antialiasingEnabled) {
        rlSetLineWidth(thickness);
        rlEnableSmoothLines();
    } else {
        rlSetLineWidth(thickness);
    }
    
    // For now, use the built-in raylib function and just draw a regular wireframe sphere
    // This can be enhanced later with the proper cube-sphere projection
    int rings = subdivisions * 4;
    int slices = subdivisions * 4;
    DrawSphereWires(center, radius, rings, slices, color);
    
    if (config && config->antialiasingEnabled) {
        rlDisableSmoothLines();
    }
    
    rlSetLineWidth(1.0f); // Reset line width
}