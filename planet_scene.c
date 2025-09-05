#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    Shader planetShader;
    Model planetModel;
    bool shaderLoaded;
    int wireframeModeLocation;
} PlanetScene;

PlanetScene InitPlanetScene(void) {
    PlanetScene scene = {0};
    
    // Load planet shader
    scene.planetShader = LoadShader("planet.vs", "planet.fs");
    if (scene.planetShader.id != rlGetShaderIdDefault()) {
        scene.shaderLoaded = true;
        scene.wireframeModeLocation = GetShaderLocation(scene.planetShader, "wireframeMode");
        printf("Planet shader loaded! Wireframe location: %d\n", scene.wireframeModeLocation);
    } else {
        scene.shaderLoaded = false;
        printf("Failed to load planet shader!\n");
    }
    
    // Create a simple sphere for the planet
    Mesh sphereMesh = GenMeshSphere(10.0f, 32, 32);
    scene.planetModel = LoadModelFromMesh(sphereMesh);
    
    // Set the shader to the model
    if (scene.shaderLoaded) {
        scene.planetModel.materials[0].shader = scene.planetShader;
    }
    
    printf("Planet scene initialized!\n");
    return scene;
}

void UpdatePlanetScene(PlanetScene* scene) {
    // No updates needed - planet always uses wireframe shader
}

void DrawPlanetScene(PlanetScene* scene, Camera3D camera) {
    if (!scene->shaderLoaded) {
        // Fallback rendering without shader
        DrawModel(scene->planetModel, (Vector3){0, 0, 0}, 1.0f, WHITE);
        return;
    }
    
    // Always enable wireframe mode
    float wireframeValue = 1.0f;
    SetShaderValue(scene->planetShader, scene->wireframeModeLocation, &wireframeValue, SHADER_UNIFORM_FLOAT);
    
    // Draw the planet with wireframe shader
    DrawModel(scene->planetModel, (Vector3){0, 0, 0}, 1.0f, WHITE);
}

void CleanupPlanetScene(PlanetScene* scene) {
    if (scene->shaderLoaded) {
        UnloadShader(scene->planetShader);
    }
    UnloadModel(scene->planetModel);
    printf("Planet scene cleaned up!\n");
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "Simple Planet with Wireframe");
    
    // Setup camera
    Camera3D camera = {0};
    camera.position = (Vector3){0.0f, 0.0f, 30.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    PlanetScene planetScene = InitPlanetScene();
    
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        UpdatePlanetScene(&planetScene);
        
        BeginDrawing();
        ClearBackground(DARKBLUE);
        
        BeginMode3D(camera);
        DrawPlanetScene(&planetScene, camera);
        //DrawGrid(20, 1.0f);
        EndMode3D();
        
        DrawText("Simple Planet with Wireframe Shader", 10, 10, 20, WHITE);
        DrawText("Planet rendered with wireframe shader", 10, 40, 16, LIGHTGRAY);
        
        DrawFPS(screenWidth - 80, 10);
        EndDrawing();
    }
    
    CleanupPlanetScene(&planetScene);
    CloseWindow();
    
    return 0;
}