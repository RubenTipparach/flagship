#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include "raylib.h"

#define MAX_CUBES 50
#define CUBE_SIZE 10.0f
#define PLAYER_SPEED 8.0f
#define MOUSE_SENSITIVITY 0.01f
#define WORLD_SIZE 200.0f
#define FLOOR_SEGMENTS 50
#define WALL_HEIGHT 5.0f
#define SUN_POSITION_X 50.0f
#define SUN_POSITION_Y 100.0f
#define SUN_POSITION_Z 50.0f
#define DEFAULT_WIREFRAME_THICKNESS 2.0f
#define MAX_WIREFRAME_THICKNESS 5.0f
#define MAX_LIGHTS 8

typedef struct {
    Vector3 position;
    Color color;
    Model model;
    bool hasModel;
} GameObject;

typedef enum {
    LIGHT_DIRECTIONAL,
    LIGHT_POINT,
    LIGHT_SPOT
} LightType;

typedef struct {
    LightType type;
    Vector3 position;
    Vector3 direction;
    Color color;
    float intensity;
    float range;
    float spotAngle;
    bool enabled;
} Light;

typedef struct {
    Light lights[MAX_LIGHTS];
    int lightCount;
    Color ambientColor;
    float ambientIntensity;
} LightingSystem;

typedef struct {
    bool antialiasingEnabled;
    float wireframeThickness;
    bool highQualityRendering;
    bool advancedShadingEnabled;
    bool normalMappingEnabled;
    float specularStrength;
    float shininess;
    bool wireframeShaderEnabled;    // Toggle for wireframe shader mode
    Color wireframeColor;           // Color for wireframe lines
} GraphicsConfig;

// Maze data structure
#define MAX_MAZE_WIDTH 50
#define MAX_MAZE_HEIGHT 50
typedef struct {
    char data[MAX_MAZE_HEIGHT][MAX_MAZE_WIDTH];
    int width;
    int height;
} Maze;

// Terrain data structure
#define TERRAIN_SIZE 1024
typedef struct {
    float heights[TERRAIN_SIZE][TERRAIN_SIZE];
    int size;
    Model terrainModel;
    Texture2D heightTexture;
    bool loaded;
    float heightMultiplier;  // Dynamic height scaling
    bool needsRebuild;       // Flag to rebuild mesh
} TerrainData;

// Cube-Sphere data structure
#define MAX_SPHERE_SUBDIVISIONS 8
typedef struct {
    Mesh sphereMesh;
    Model sphereModel;
    bool loaded;
    int subdivisionLevel;
    float radius;
    Vector3 center;
    bool needsRebuild;
    float lastCameraDistance;
    int dynamicSubdivisions;
    float morphFactor; // 0.0 = cube, 1.0 = sphere
    bool wireframeMode; // Toggle between solid and wireframe rendering
    Shader planetShader; // Wireframe shader for planet rendering
    bool shaderLoaded; // Whether planet shader is loaded
    int wireframeModeLocation; // Uniform location for wireframe mode
} CubeSphereData;

// Scene types
typedef enum {
    SCENE_MAZE,
    SCENE_TERRAIN,
    SCENE_CUBE_SPHERE
} SceneType;

// Forward declarations
typedef struct Scene Scene;
struct WireframeShader;

// Scene function pointers
typedef void (*SceneInitFunc)(Scene* scene, LightingSystem* lighting, GraphicsConfig* gfxConfig);
typedef void (*SceneUpdateFunc)(Scene* scene, float deltaTime, Camera3D* camera);
typedef void (*SceneRenderFunc)(Scene* scene, Camera3D camera, GraphicsConfig* gfxConfig, struct WireframeShader* wireframeShader);
typedef void (*SceneCleanupFunc)(Scene* scene);

// Scene structure
typedef struct Scene {
    SceneType type;
    char name[64];
    bool initialized;
    void* sceneData;  // Scene-specific data
    
    SceneInitFunc init;
    SceneUpdateFunc update;
    SceneRenderFunc render;
    SceneCleanupFunc cleanup;
} Scene;

// Scene manager
#define MAX_SCENES 10
typedef struct {
    Scene scenes[MAX_SCENES];
    int sceneCount;
    int currentSceneIndex;
    Scene* currentScene;
} SceneManager;

#endif // GAME_TYPES_H