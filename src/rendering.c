#include "rendering.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>

// Initialize wireframe shader system
WireframeShader LoadWireframeShader(void) {
    WireframeShader wireframeShader = {0};
    
    // Load the wireframe shader
    wireframeShader.shader = LoadShader("wireframe.vs", "wireframe.fs");
    
    if (wireframeShader.shader.id != rlGetShaderIdDefault()) {
        wireframeShader.loaded = true;
        
        // Get uniform locations
        wireframeShader.enableWireframeLoc = GetShaderLocation(wireframeShader.shader, "enableWireframe");
        wireframeShader.wireframeColorLoc = GetShaderLocation(wireframeShader.shader, "wireframeColor");
        wireframeShader.wireframeThicknessLoc = GetShaderLocation(wireframeShader.shader, "wireframeThickness");
        
        printf("SHADER: Wireframe shader loaded successfully!\n");
        printf("SHADER: enableWireframe location: %d\n", wireframeShader.enableWireframeLoc);
        printf("SHADER: wireframeColor location: %d\n", wireframeShader.wireframeColorLoc);
        printf("SHADER: wireframeThickness location: %d\n", wireframeShader.wireframeThicknessLoc);
    } else {
        wireframeShader.loaded = false;
        printf("SHADER: Failed to load wireframe shader!\n");
    }
    
    return wireframeShader;
}

// Cleanup wireframe shader
void UnloadWireframeShader(WireframeShader* wireframeShader) {
    if (wireframeShader && wireframeShader->loaded) {
        UnloadShader(wireframeShader->shader);
        wireframeShader->loaded = false;
        TraceLog(LOG_INFO, "SHADER: Wireframe shader unloaded");
    }
}

// Draw model with wireframe shader
void DrawModelWireframe(Model model, Vector3 position, float scale, Color tint, 
                       WireframeShader* wireframeShader, GraphicsConfig* config) {
    printf("DrawModelWireframe called! Wireframe enabled: %s\n", 
           config ? (config->wireframeShaderEnabled ? "YES" : "NO") : "NULL");
    
    if (!wireframeShader || !wireframeShader->loaded || !config) {
        // Fallback to regular model drawing
        printf("Using fallback rendering\n");
        DrawModelEx(model, position, (Vector3){0.0f, 1.0f, 0.0f}, 0.0f, 
                   (Vector3){scale, scale, scale}, tint);
        return;
    }
    
    printf("Using wireframe shader\n");
    // Always use the wireframe shader, and control the wireframe effect through uniforms
    BeginShaderMode(wireframeShader->shader);
    
    // Set shader uniforms - use float as that's what the shader expects now
    float enableWireframe = config->wireframeShaderEnabled ? 1.0f : 0.0f;
    printf("Setting enableWireframe uniform: %.1f (location: %d)\n", enableWireframe, wireframeShader->enableWireframeLoc);
    
    if (wireframeShader->enableWireframeLoc >= 0) {
        SetShaderValue(wireframeShader->shader, wireframeShader->enableWireframeLoc, 
                      &enableWireframe, SHADER_UNIFORM_FLOAT);
    }
    
    // Always set wireframe color and thickness (even when wireframe is disabled)
    float wireframeColor[4] = {
        config->wireframeColor.r / 255.0f,
        config->wireframeColor.g / 255.0f,
        config->wireframeColor.b / 255.0f,
        config->wireframeColor.a / 255.0f
    };
    SetShaderValue(wireframeShader->shader, wireframeShader->wireframeColorLoc, 
                  wireframeColor, SHADER_UNIFORM_VEC4);
    
    SetShaderValue(wireframeShader->shader, wireframeShader->wireframeThicknessLoc, 
                  &config->wireframeThickness, SHADER_UNIFORM_FLOAT);
    
    // Draw the model
    DrawModelEx(model, position, (Vector3){0.0f, 1.0f, 0.0f}, 0.0f, 
               (Vector3){scale, scale, scale}, tint);
    
    // End shader mode
    EndShaderMode();
}

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