#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Include our modular headers
#include "game_types.h"
#include "lighting.h"
#include "mesh_generation.h"
#include "rendering.h"
#include "maze.h"
#include "scene_manager.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    // Initialize graphics configuration
    GraphicsConfig gfxConfig = {0};
    gfxConfig.antialiasingEnabled = true;
    gfxConfig.wireframeThickness = DEFAULT_WIREFRAME_THICKNESS;
    gfxConfig.highQualityRendering = true;
    gfxConfig.advancedShadingEnabled = true;
    gfxConfig.normalMappingEnabled = false;
    gfxConfig.specularStrength = 0.5f;
    gfxConfig.shininess = 32.0f;
    
    // Initialize lighting system
    LightingSystem lighting = InitLightingSystem();
    
    // Add a directional light (sun)
    AddLight(&lighting, LIGHT_DIRECTIONAL, (Vector3){SUN_POSITION_X, SUN_POSITION_Y, SUN_POSITION_Z}, 
             (Vector3){-0.3f, -1.0f, -0.2f}, YELLOW, 1.0f, 1000.0f, 0.0f);
    
    // Add a moving point light
    AddLight(&lighting, LIGHT_POINT, (Vector3){0.0f, 15.0f, 0.0f}, (Vector3){0.0f, 0.0f, 0.0f}, 
             ORANGE, 2.0f, 100.0f, 0.0f);
    
    // Add a spot light
    AddLight(&lighting, LIGHT_SPOT, (Vector3){50.0f, 20.0f, 0.0f}, (Vector3){-1.0f, -1.0f, 0.0f}, 
             BLUE, 1.5f, 80.0f, 45.0f);
    
    SetConfigFlags(FLAG_VSYNC_HINT | (gfxConfig.antialiasingEnabled ? FLAG_MSAA_4X_HINT : 0));
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(screenWidth, screenHeight, "FPS Cube World - Pi 4");
    
    if (!IsWindowReady())
    {
        printf("Failed to create window, trying fallback settings...\n");
        CloseWindow();
        
        SetConfigFlags(0);
        InitWindow(screenWidth, screenHeight, "FPS Cube World - Pi 4");
        
        if (!IsWindowReady())
        {
            printf("Failed to initialize graphics. Make sure:\n");
            printf("1. X11 is running\n");
            printf("2. GPU memory split is set (raspi-config -> Advanced -> Memory Split -> 128 or 256)\n");
            printf("3. OpenGL driver is enabled (raspi-config -> Advanced -> GL Driver -> Legacy or Fake KMS)\n");
            return -1;
        }
    }
    
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 20.0f };  // Start above smaller terrain
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Look toward terrain center
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    srand(time(NULL));
    
    // Initialize scene manager
    SceneManager sceneManager = InitSceneManager();
    
    // Create and add scenes
    Scene mazeScene = CreateMazeScene();
    Scene terrainScene = CreateTerrainScene();
    
    AddScene(&sceneManager, mazeScene);
    AddScene(&sceneManager, terrainScene);
    
    // Start with maze scene
    SwitchScene(&sceneManager, 0);
    
    // Initialize current scene
    if (sceneManager.currentScene && sceneManager.currentScene->init) {
        sceneManager.currentScene->init(sceneManager.currentScene, &lighting, &gfxConfig);
    }
    
    bool cursorLocked = false;
    Vector2 mousePos = {0};
    Vector2 centerPos = {screenWidth/2.0f, screenHeight/2.0f};
    
    // Calculate initial yaw and pitch from camera direction
    Vector3 initialDir = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    float yaw = atan2f(initialDir.x, initialDir.z);
    float pitch = asinf(initialDir.y);
    
    SetTargetFPS(60);
    
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        
        if (IsKeyPressed(KEY_TAB))
        {
            cursorLocked = !cursorLocked;
            if (cursorLocked)
            {
                DisableCursor();
                SetMousePosition(centerPos.x, centerPos.y);
            }
            else
            {
                EnableCursor();
            }
        }
        
        // Scene switching controls
        if (IsKeyPressed(KEY_ONE))
        {
            SwitchScene(&sceneManager, 0);
            if (sceneManager.currentScene && sceneManager.currentScene->init) {
                sceneManager.currentScene->init(sceneManager.currentScene, &lighting, &gfxConfig);
            }
        }
        
        if (IsKeyPressed(KEY_TWO))
        {
            SwitchScene(&sceneManager, 1);
            if (sceneManager.currentScene && sceneManager.currentScene->init) {
                sceneManager.currentScene->init(sceneManager.currentScene, &lighting, &gfxConfig);
            }
        }
        
        // Graphics config controls
        if (IsKeyPressed(KEY_F1))
        {
            gfxConfig.antialiasingEnabled = !gfxConfig.antialiasingEnabled;
            TraceLog(LOG_INFO, "Antialiasing: %s", gfxConfig.antialiasingEnabled ? "ON" : "OFF");
        }
        
        if (IsKeyPressed(KEY_F2))
        {
            gfxConfig.wireframeThickness += 0.5f;
            if (gfxConfig.wireframeThickness > MAX_WIREFRAME_THICKNESS)
                gfxConfig.wireframeThickness = 1.0f;
            TraceLog(LOG_INFO, "Wireframe thickness: %.1f", gfxConfig.wireframeThickness);
        }
        
        if (IsKeyPressed(KEY_F3))
        {
            gfxConfig.highQualityRendering = !gfxConfig.highQualityRendering;
            TraceLog(LOG_INFO, "High Quality Rendering: %s", gfxConfig.highQualityRendering ? "ON" : "OFF");
        }
        
        if (IsKeyPressed(KEY_F4))
        {
            gfxConfig.advancedShadingEnabled = !gfxConfig.advancedShadingEnabled;
            TraceLog(LOG_INFO, "Advanced Shading: %s", gfxConfig.advancedShadingEnabled ? "ON" : "OFF");
        }
        
        if (IsKeyPressed(KEY_F5))
        {
            gfxConfig.specularStrength += 0.1f;
            if (gfxConfig.specularStrength > 1.0f) gfxConfig.specularStrength = 0.0f;
            TraceLog(LOG_INFO, "Specular Strength: %.1f", gfxConfig.specularStrength);
        }
        
        // Update lighting system
        UpdateLightingSystem(&lighting, deltaTime);
        
        // Update current scene
        UpdateCurrentScene(&sceneManager, deltaTime, &camera);
        
        if (cursorLocked)
        {
            mousePos = GetMousePosition();
            Vector2 mouseDelta = Vector2Subtract(mousePos, centerPos);
            
            if (Vector2Length(mouseDelta) > 0.1f)
            {
                yaw -= mouseDelta.x * MOUSE_SENSITIVITY;  // Inverted X axis
                pitch -= mouseDelta.y * MOUSE_SENSITIVITY;  // Inverted Y axis
                
                // Clamp pitch to prevent over-rotation
                if (pitch > PI/2 - 0.1f) pitch = PI/2 - 0.1f;
                if (pitch < -PI/2 + 0.1f) pitch = -PI/2 + 0.1f;
                
                // Reset mouse to center
                SetMousePosition(centerPos.x, centerPos.y);
            }
            
            // Update camera target based on current yaw and pitch
            Vector3 forward = {
                sinf(yaw) * cosf(pitch),
                sinf(pitch), 
                cosf(yaw) * cosf(pitch)
            };
            
            camera.target = Vector3Add(camera.position, forward);
        }
        
        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
        
        Vector3 moveVector = { 0.0f, 0.0f, 0.0f };
        
        if (IsKeyDown(KEY_W)) moveVector = Vector3Add(moveVector, forward);
        if (IsKeyDown(KEY_S)) moveVector = Vector3Subtract(moveVector, forward);
        if (IsKeyDown(KEY_A)) moveVector = Vector3Subtract(moveVector, right);
        if (IsKeyDown(KEY_D)) moveVector = Vector3Add(moveVector, right);
        
        if (Vector3Length(moveVector) > 0)
        {
            moveVector = Vector3Normalize(moveVector);
            moveVector = Vector3Scale(moveVector, PLAYER_SPEED * deltaTime);
            camera.position = Vector3Add(camera.position, moveVector);
            camera.target = Vector3Add(camera.target, moveVector);
        }
        
        // Floor collision detection - prevent falling too far underground
        float minHeight = -2.0f; // Allow going below sea level but not too far
        if (camera.position.y < minHeight) {
            camera.position.y = minHeight;
            camera.target.y = camera.target.y + (minHeight - (camera.position.y - (camera.target.y - camera.position.y)));
        }
        
        BeginDrawing();
        
        ClearBackground(SKYBLUE);
        
        BeginMode3D(camera);
        
        // Draw all lights in the lighting system
        DrawLights(&lighting);
        
        // Render current scene
        RenderCurrentScene(&sceneManager, camera, &gfxConfig);
        
        DrawGrid(100, 1.0f);
        
        EndMode3D();
        
        DrawText("FPS Cube World", 10, 10, 20, BLACK);
        DrawText("WASD to move", 10, 30, 16, DARKGRAY);
        if (cursorLocked)
            DrawText("TAB to unlock cursor, Mouse to look", 10, 50, 16, DARKGRAY);
        else
            DrawText("TAB to lock cursor for mouse look", 10, 50, 16, DARKGRAY);
        
        DrawText("1: Maze Scene, 2: Terrain Scene", 10, 70, 16, DARKGRAY);
        DrawText("+/-: Terrain Height (in Terrain Scene)", 10, 90, 16, DARKGRAY);
        DrawText("F1: AA, F2: Wireframe, F3: Quality, F4: Shading, F5: Specular", 10, 110, 14, DARKGRAY);
        
        // Display current scene
        if (sceneManager.currentScene) {
            char sceneText[100];
            sprintf(sceneText, "Current Scene: %s", sceneManager.currentScene->name);
            DrawText(sceneText, 10, 130, 16, BLUE);
        }
        
        // Display player coordinates and debug info
        char coordText[200];
        sprintf(coordText, "Position: X=%.1f Y=%.1f Z=%.1f", 
            camera.position.x, camera.position.y, camera.position.z);
        DrawText(coordText, 10, 150, 16, DARKGREEN);
        
        char debugText[200];
        sprintf(debugText, "Target: X=%.1f Y=%.1f Z=%.1f", 
            camera.target.x, camera.target.y, camera.target.z);
        DrawText(debugText, 10, 170, 16, DARKGREEN);
        
        sprintf(debugText, "Graphics: AA:%s Shading:%s Specular:%.1f Lights:%d", 
            gfxConfig.antialiasingEnabled ? "ON" : "OFF",
            gfxConfig.advancedShadingEnabled ? "ADV" : "SIM",
            gfxConfig.specularStrength,
            lighting.lightCount);
        DrawText(debugText, 10, 190, 16, DARKGREEN);
        
        sprintf(debugText, "Cursor: %s, Yaw=%.2f, Pitch=%.2f", 
            cursorLocked ? "LOCKED" : "FREE", yaw * 180.0f / PI, pitch * 180.0f / PI);
        DrawText(debugText, 10, 210, 16, DARKGREEN);
        
        DrawFPS(screenWidth - 100, 10);
        
        EndDrawing();
    }
    
    // Cleanup scene manager (this will cleanup all scene resources)
    CleanupSceneManager(&sceneManager);
    
    CloseWindow();
    
    return 0;
}