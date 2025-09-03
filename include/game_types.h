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
} GraphicsConfig;

// Maze data structure
#define MAX_MAZE_WIDTH 50
#define MAX_MAZE_HEIGHT 50
typedef struct {
    char data[MAX_MAZE_HEIGHT][MAX_MAZE_WIDTH];
    int width;
    int height;
} Maze;

#endif // GAME_TYPES_H