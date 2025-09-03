#include "rendering.h"

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