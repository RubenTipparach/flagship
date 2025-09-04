#include "scene_manager.h"
#include "maze.h"
#include "mesh_generation.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Maze scene specific data
typedef struct {
    Maze maze;
    Model mazeWallModel;
    Model floorModel;
    Model advancedFloorModel;
    bool advancedMeshGenerated;
} MazeSceneData;

// Terrain scene specific data
typedef struct {
    TerrainData terrain;
    Model floorModel;
} TerrainSceneData;

// Cube-Sphere scene specific data
typedef struct {
    CubeSphereData cubeSphere;
} CubeSphereSceneData;

// Scene manager functions
SceneManager InitSceneManager(void) {
    SceneManager manager = {0};
    manager.sceneCount = 0;
    manager.currentSceneIndex = -1;
    manager.currentScene = NULL;
    return manager;
}

void AddScene(SceneManager* manager, Scene scene) {
    if (manager->sceneCount < MAX_SCENES) {
        manager->scenes[manager->sceneCount] = scene;
        manager->sceneCount++;
    }
}

void SwitchScene(SceneManager* manager, int sceneIndex) {
    if (sceneIndex >= 0 && sceneIndex < manager->sceneCount) {
        // Cleanup current scene if switching
        if (manager->currentScene && manager->currentScene->cleanup) {
            manager->currentScene->cleanup(manager->currentScene);
        }
        
        manager->currentSceneIndex = sceneIndex;
        manager->currentScene = &manager->scenes[sceneIndex];
        
        printf("Switched to scene: %s\n", manager->currentScene->name);
    }
}

void UpdateCurrentScene(SceneManager* manager, float deltaTime, Camera3D* camera) {
    if (manager->currentScene && manager->currentScene->update) {
        manager->currentScene->update(manager->currentScene, deltaTime, camera);
    }
}

void RenderCurrentScene(SceneManager* manager, Camera3D camera, GraphicsConfig* gfxConfig) {
    if (manager->currentScene && manager->currentScene->render) {
        manager->currentScene->render(manager->currentScene, camera, gfxConfig);
    }
}

void CleanupSceneManager(SceneManager* manager) {
    for (int i = 0; i < manager->sceneCount; i++) {
        if (manager->scenes[i].cleanup) {
            manager->scenes[i].cleanup(&manager->scenes[i]);
        }
    }
    manager->sceneCount = 0;
    manager->currentScene = NULL;
}

// Maze scene functions
void InitMazeScene(Scene* scene, LightingSystem* lighting, GraphicsConfig* gfxConfig) {
    MazeSceneData* data = (MazeSceneData*)malloc(sizeof(MazeSceneData));
    scene->sceneData = data;
    
    // Load maze
    data->maze = LoadMazeFromFile("maze.txt");
    
    // Generate floor mesh
    Mesh floorMesh = GenMeshFloorWithColors(WORLD_SIZE * 2, WORLD_SIZE * 2, FLOOR_SEGMENTS, FLOOR_SEGMENTS);
    data->floorModel = LoadModelFromMesh(floorMesh);
    
    // Generate maze wall mesh
    Mesh mazeWallMesh = GenMeshMazeWallCube(10.0f, lighting, gfxConfig);
    data->mazeWallModel = LoadModelFromMesh(mazeWallMesh);
    
    data->advancedMeshGenerated = false;
    scene->initialized = true;
}

void UpdateMazeScene(Scene* scene, float deltaTime, Camera3D* camera) {
    // Maze scene doesn't need much updating
}

void RenderMazeScene(Scene* scene, Camera3D camera, GraphicsConfig* gfxConfig) {
    MazeSceneData* data = (MazeSceneData*)scene->sceneData;
    
    // Draw floor
    if (gfxConfig->advancedShadingEnabled && data->advancedMeshGenerated) {
        DrawModel(data->advancedFloorModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
    } else {
        DrawModel(data->floorModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
    }
    
    // Draw maze walls
    float cellSize = 10.0f;
    float mazeStartX = -(data->maze.width * cellSize) / 2.0f;
    float mazeStartZ = -(data->maze.height * cellSize) / 2.0f;
    
    for (int row = 0; row < data->maze.height; row++) {
        for (int col = 0; col < data->maze.width; col++) {
            if (data->maze.data[row][col] == '#') {
                float wallX = mazeStartX + col * cellSize;
                float wallZ = mazeStartZ + row * cellSize;
                float wallY = WALL_HEIGHT / 2.0f;
                
                Vector3 wallPos = (Vector3){ wallX, wallY, wallZ };
                DrawModel(data->mazeWallModel, wallPos, 1.0f, WHITE);
            }
        }
    }
}

void CleanupMazeScene(Scene* scene) {
    if (scene->sceneData) {
        MazeSceneData* data = (MazeSceneData*)scene->sceneData;
        UnloadModel(data->mazeWallModel);
        UnloadModel(data->floorModel);
        if (data->advancedMeshGenerated) {
            UnloadModel(data->advancedFloorModel);
        }
        free(data);
        scene->sceneData = NULL;
    }
}

// Terrain scene functions
void InitTerrainScene(Scene* scene, LightingSystem* lighting, GraphicsConfig* gfxConfig) {
    TerrainSceneData* data = (TerrainSceneData*)malloc(sizeof(TerrainSceneData));
    scene->sceneData = data;
    
    // Initialize terrain data
    data->terrain.size = TERRAIN_SIZE;
    data->terrain.loaded = false;
    data->terrain.heightMultiplier = 0.0f;  // Start with flat plane (no height)
    data->terrain.needsRebuild = false;
    
    // Try to load height map
    Image heightImage = LoadImage("heightmap.png");
    if (heightImage.data != NULL) {
        // Convert image to height data
        ImageFormat(&heightImage, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
        
        // Scale image to terrain size if needed
        if (heightImage.width != TERRAIN_SIZE || heightImage.height != TERRAIN_SIZE) {
            ImageResize(&heightImage, TERRAIN_SIZE, TERRAIN_SIZE);
        }
        
        // Convert pixel data to height values
        unsigned char* pixels = (unsigned char*)heightImage.data;
        for (int y = 0; y < TERRAIN_SIZE; y++) {
            for (int x = 0; x < TERRAIN_SIZE; x++) {
                int index = y * TERRAIN_SIZE + x;
                data->terrain.heights[y][x] = (float)pixels[index] / 255.0f * 50.0f; // Scale to 0-50 units height
            }
        }
        
        data->terrain.heightTexture = LoadTextureFromImage(heightImage);
        UnloadImage(heightImage);
        data->terrain.loaded = true;
        printf("Loaded height map: heightmap.png\n");
    } else {
        // Generate random terrain if no height map found
        printf("No heightmap.png found, generating random terrain\n");
        for (int y = 0; y < TERRAIN_SIZE; y++) {
            for (int x = 0; x < TERRAIN_SIZE; x++) {
                // Simple noise-like generation
                float distance = sqrtf((x - TERRAIN_SIZE/2.0f) * (x - TERRAIN_SIZE/2.0f) + 
                                     (y - TERRAIN_SIZE/2.0f) * (y - TERRAIN_SIZE/2.0f));
                float normalizedDist = distance / (TERRAIN_SIZE * 0.5f);
                
                // Create island-like terrain
                float height = (1.0f - normalizedDist) * 30.0f;
                if (height < 0) height = 0;
                
                // Add some randomness
                height += (float)(rand() % 100) / 100.0f * 5.0f - 2.5f;
                if (height < 0) height = 0;
                
                data->terrain.heights[y][x] = height;
            }
        }
    }
    
    // Generate terrain mesh from height data
    if (data->terrain.loaded || data->terrain.size > 0) {
        // Create 102.4x102.4 unit terrain plane centered at origin
        float terrainScale = 0.1f;  // Each pixel = 0.1 units (creates 102.4x102.4 unit terrain)
        float heightScale = 5.0f;   // Maximum height of 5 units
        
        Mesh terrainMesh = GenMeshTerrainFromHeightMap(&data->terrain, terrainScale, heightScale);
        data->terrain.terrainModel = LoadModelFromMesh(terrainMesh);
        printf("Generated terrain mesh with %d vertices\n", terrainMesh.vertexCount);
    } else {
        // Fallback basic floor
        Mesh floorMesh = GenMeshFloorWithColors(WORLD_SIZE * 2, WORLD_SIZE * 2, FLOOR_SEGMENTS, FLOOR_SEGMENTS);
        data->floorModel = LoadModelFromMesh(floorMesh);
    }
    
    scene->initialized = true;
}

void UpdateTerrainScene(Scene* scene, float deltaTime, Camera3D* camera) {
    TerrainSceneData* data = (TerrainSceneData*)scene->sceneData;
    
    // Handle terrain height adjustment
    bool heightChanged = false;
    
    if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {  // + key
        data->terrain.heightMultiplier += 0.1f;
        if (data->terrain.heightMultiplier > 2.0f) data->terrain.heightMultiplier = 2.0f;
        heightChanged = true;
        printf("Terrain height multiplier: %.1f\n", data->terrain.heightMultiplier);
    }
    
    if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {  // - key
        data->terrain.heightMultiplier -= 0.1f;
        if (data->terrain.heightMultiplier < 0.0f) data->terrain.heightMultiplier = 0.0f;
        heightChanged = true;
        printf("Terrain height multiplier: %.1f\n", data->terrain.heightMultiplier);
    }
    
    // Rebuild terrain if height changed
    if (heightChanged && (data->terrain.loaded || data->terrain.size > 0)) {
        // Unload old model
        if (data->terrain.terrainModel.meshCount > 0) {
            UnloadModel(data->terrain.terrainModel);
        }
        
        // Regenerate terrain with new height
        float terrainScale = 0.1f;  // Each pixel = 0.1 units (creates 102.4x102.4 unit terrain)
        float heightScale = 5.0f;   // Base height scaling
        
        Mesh terrainMesh = GenMeshTerrainFromHeightMap(&data->terrain, terrainScale, heightScale);
        data->terrain.terrainModel = LoadModelFromMesh(terrainMesh);
    }
}

void RenderTerrainScene(Scene* scene, Camera3D camera, GraphicsConfig* gfxConfig) {
    TerrainSceneData* data = (TerrainSceneData*)scene->sceneData;
    
    // Draw terrain or fallback floor
    if (data->terrain.terrainModel.meshCount > 0) {
        DrawModel(data->terrain.terrainModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
    } else if (data->floorModel.meshCount > 0) {
        DrawModel(data->floorModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, GREEN);
    }
    
    // Draw some visual indication this is terrain scene
    DrawCube((Vector3){0, 25, 0}, 5, 5, 5, BROWN);
}

void CleanupTerrainScene(Scene* scene) {
    if (scene->sceneData) {
        TerrainSceneData* data = (TerrainSceneData*)scene->sceneData;
        if (data->terrain.terrainModel.meshCount > 0) {
            UnloadModel(data->terrain.terrainModel);
        }
        if (data->floorModel.meshCount > 0) {
            UnloadModel(data->floorModel);
        }
        if (data->terrain.loaded && data->terrain.heightTexture.id > 0) {
            UnloadTexture(data->terrain.heightTexture);
        }
        free(data);
        scene->sceneData = NULL;
    }
}

// Scene creation functions
Scene CreateMazeScene(void) {
    Scene scene = {0};
    scene.type = SCENE_MAZE;
    strcpy(scene.name, "Maze Scene");
    scene.initialized = false;
    scene.sceneData = NULL;
    
    scene.init = InitMazeScene;
    scene.update = UpdateMazeScene;
    scene.render = RenderMazeScene;
    scene.cleanup = CleanupMazeScene;
    
    return scene;
}

Scene CreateTerrainScene(void) {
    Scene scene = {0};
    scene.type = SCENE_TERRAIN;
    strcpy(scene.name, "Terrain Scene");
    scene.initialized = false;
    scene.sceneData = NULL;
    
    scene.init = InitTerrainScene;
    scene.update = UpdateTerrainScene;
    scene.render = RenderTerrainScene;
    scene.cleanup = CleanupTerrainScene;
    
    return scene;
}

// Cube-Sphere scene functions
void InitCubeSphereScene(Scene* scene, LightingSystem* lighting, GraphicsConfig* gfxConfig) {
    CubeSphereSceneData* data = (CubeSphereSceneData*)malloc(sizeof(CubeSphereSceneData));
    scene->sceneData = data;
    
    // Initialize cube-sphere data
    data->cubeSphere.radius = 50.0f;
    data->cubeSphere.center = (Vector3){0.0f, 0.0f, 0.0f};
    data->cubeSphere.subdivisionLevel = 8; // Higher subdivision for smoother morphing
    data->cubeSphere.dynamicSubdivisions = 8;
    data->cubeSphere.lastCameraDistance = 0.0f;
    data->cubeSphere.needsRebuild = true;
    data->cubeSphere.loaded = false;
    data->cubeSphere.morphFactor = 0.0f; // Start as a cube
    data->cubeSphere.wireframeMode = false; // Start in solid mode
    
    // Generate initial mesh using the new morphing function
    Mesh sphereMesh = GenMeshSubdividedCube(data->cubeSphere.radius, data->cubeSphere.subdivisionLevel, data->cubeSphere.morphFactor);
    data->cubeSphere.sphereModel = LoadModelFromMesh(sphereMesh);
    data->cubeSphere.sphereMesh = sphereMesh;
    data->cubeSphere.loaded = true;
    data->cubeSphere.needsRebuild = false;
    
    scene->initialized = true;
    printf("Initialized Cube-Sphere scene with radius %.1f and subdivision level %d\n", 
           data->cubeSphere.radius, data->cubeSphere.subdivisionLevel);
}

void UpdateCubeSphereScene(Scene* scene, float deltaTime, Camera3D* camera) {
    CubeSphereSceneData* data = (CubeSphereSceneData*)scene->sceneData;
    
    bool morphChanged = false;
    
    // Handle morphing controls
    if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {  // + key
        data->cubeSphere.morphFactor += 0.1f;
        if (data->cubeSphere.morphFactor > 1.0f) data->cubeSphere.morphFactor = 1.0f;
        morphChanged = true;
        printf("Morph factor: %.1f\n", data->cubeSphere.morphFactor);
    }
    
    if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {  // - key
        data->cubeSphere.morphFactor -= 0.1f;
        if (data->cubeSphere.morphFactor < 0.0f) data->cubeSphere.morphFactor = 0.0f;
        morphChanged = true;
        printf("Morph factor: %.1f\n", data->cubeSphere.morphFactor);
    }
    
    // Handle wireframe toggle
    // if (IsKeyPressed(KEY_W)) {
    //     data->cubeSphere.wireframeMode = !data->cubeSphere.wireframeMode;
    //     printf("Wireframe mode: %s\n", data->cubeSphere.wireframeMode ? "ON" : "OFF");
    // }
    
    // Rebuild mesh if morph factor changed
    if (morphChanged && data->cubeSphere.loaded) {
        // Unload old model
        if (data->cubeSphere.sphereModel.meshCount > 0) {
            UnloadModel(data->cubeSphere.sphereModel);
        }
        
        // Generate new mesh with current morph factor
        Mesh newSphereMesh = GenMeshSubdividedCube(data->cubeSphere.radius, data->cubeSphere.subdivisionLevel, 
                                                 data->cubeSphere.morphFactor);
        data->cubeSphere.sphereModel = LoadModelFromMesh(newSphereMesh);
        data->cubeSphere.sphereMesh = newSphereMesh;
        data->cubeSphere.needsRebuild = false;
        
        printf("Rebuilt morphing mesh with factor %.1f (%d vertices)\n", 
               data->cubeSphere.morphFactor, newSphereMesh.vertexCount);
    }
}

void RenderCubeSphereScene(Scene* scene, Camera3D camera, GraphicsConfig* gfxConfig) {
    CubeSphereSceneData* data = (CubeSphereSceneData*)scene->sceneData;
    
    if (!data->cubeSphere.loaded) return;
    
    if (data->cubeSphere.wireframeMode) {
        // Draw only wireframe
        DrawCubeSphereWires(data->cubeSphere.center, data->cubeSphere.radius, 
                           data->cubeSphere.subdivisionLevel, LIME, gfxConfig);
    } else {
        // Draw solid model with optional wireframe overlay
        DrawModel(data->cubeSphere.sphereModel, data->cubeSphere.center, 1.0f, WHITE);
        
        // Optionally draw a subtle wireframe overlay
        DrawCubeSphereWires(data->cubeSphere.center, data->cubeSphere.radius, 
                           data->cubeSphere.subdivisionLevel, (Color){100, 255, 100, 80}, gfxConfig);
    }
    
    // Draw some reference objects to show scale
    DrawCube((Vector3){data->cubeSphere.radius + 20.0f, 0, 0}, 5, 5, 5, RED);
    DrawCube((Vector3){0, data->cubeSphere.radius + 20.0f, 0}, 5, 5, 5, GREEN);
    DrawCube((Vector3){0, 0, data->cubeSphere.radius + 20.0f}, 5, 5, 5, BLUE);
    
    // Draw UI info
    DrawText(TextFormat("Morph Factor: %.1f (0=Cube, 1=Sphere)", data->cubeSphere.morphFactor), 10, 10, 20, WHITE);
    DrawText(TextFormat("Wireframe Mode: %s", data->cubeSphere.wireframeMode ? "ON" : "OFF"), 10, 35, 20, WHITE);
    DrawText(TextFormat("Subdivision Level: %d", data->cubeSphere.subdivisionLevel), 10, 60, 20, WHITE);
    DrawText(TextFormat("Vertices: %d", data->cubeSphere.sphereMesh.vertexCount), 10, 85, 20, WHITE);
    DrawText("Press +/- keys to morph, W to toggle wireframe", 10, 110, 20, YELLOW);
}

void CleanupCubeSphereScene(Scene* scene) {
    if (scene->sceneData) {
        CubeSphereSceneData* data = (CubeSphereSceneData*)scene->sceneData;
        if (data->cubeSphere.loaded && data->cubeSphere.sphereModel.meshCount > 0) {
            UnloadModel(data->cubeSphere.sphereModel);
        }
        free(data);
        scene->sceneData = NULL;
    }
}

Scene CreateCubeSphereScene(void) {
    Scene scene = {0};
    scene.type = SCENE_CUBE_SPHERE;
    strcpy(scene.name, "Cube-Sphere Scene");
    scene.initialized = false;
    scene.sceneData = NULL;
    
    scene.init = InitCubeSphereScene;
    scene.update = UpdateCubeSphereScene;
    scene.render = RenderCubeSphereScene;
    scene.cleanup = CleanupCubeSphereScene;
    
    return scene;
}