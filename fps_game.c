#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

typedef struct {
    Vector3 position;
    Color color;
    Model model;
    bool hasModel;
} GameObject;

typedef struct {
    bool antialiasingEnabled;
    float wireframeThickness;
    bool highQualityRendering;
} GraphicsConfig;

// Calculate lighting for a vertex based on sun position
Color CalculateVertexLighting(Vector3 vertexPos, Vector3 normal, Color baseColor)
{
    Vector3 sunPos = { SUN_POSITION_X, SUN_POSITION_Y, SUN_POSITION_Z };
    Vector3 lightDir = Vector3Normalize(Vector3Subtract(sunPos, vertexPos));
    
    // Calculate dot product for diffuse lighting (Lambert)
    float NdotL = Vector3DotProduct(normal, lightDir);
    NdotL = fmaxf(0.0f, NdotL); // Clamp to 0-1
    
    // Add ambient lighting (0.3) plus diffuse (0.7 * NdotL)
    float lightIntensity = 0.3f + 0.7f * NdotL;
    
    Color litColor;
    litColor.r = (unsigned char)(baseColor.r * lightIntensity);
    litColor.g = (unsigned char)(baseColor.g * lightIntensity);
    litColor.b = (unsigned char)(baseColor.b * lightIntensity);
    litColor.a = baseColor.a;
    
    return litColor;
}

// Generate a custom floor mesh with vertex colors
Mesh GenMeshFloorWithColors(float width, float height, int resX, int resZ)
{
    int vertexCount = resX * resZ;
    int triangleCount = (resX-1) * (resZ-1) * 2;
    
    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    
    mesh.vertices = (float *)MemAlloc(vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float *)MemAlloc(vertexCount * 2 * sizeof(float));
    mesh.normals = (float *)MemAlloc(vertexCount * 3 * sizeof(float));
    mesh.colors = (unsigned char *)MemAlloc(vertexCount * 4 * sizeof(unsigned char));
    mesh.indices = (unsigned short *)MemAlloc(triangleCount * 3 * sizeof(unsigned short));
    
    int vCounter = 0;
    int tcCounter = 0;
    int nCounter = 0;
    int cCounter = 0;
    
    for (int z = 0; z < resZ; z++)
    {
        for (int x = 0; x < resX; x++)
        {
            // Vertices
            mesh.vertices[vCounter] = (float)x/(resX-1) * width - width/2;
            mesh.vertices[vCounter+1] = 0.0f;
            mesh.vertices[vCounter+2] = (float)z/(resZ-1) * height - height/2;
            
            // Normals
            mesh.normals[nCounter] = 0.0f;
            mesh.normals[nCounter+1] = 1.0f;
            mesh.normals[nCounter+2] = 0.0f;
            
            // Texcoords
            mesh.texcoords[tcCounter] = (float)x/(resX-1);
            mesh.texcoords[tcCounter+1] = (float)z/(resZ-1);
            
            // Vertex colors - create a checkerboard pattern with lighting
            Color baseColor;
            if ((x + z) % 2 == 0) {
                baseColor = (Color){ 100, 150, 100, 255 };
            } else {
                baseColor = (Color){ 80, 120, 80, 255 };
            }
            
            Vector3 vertexPos = { mesh.vertices[vCounter], mesh.vertices[vCounter+1], mesh.vertices[vCounter+2] };
            Vector3 normal = { mesh.normals[nCounter], mesh.normals[nCounter+1], mesh.normals[nCounter+2] };
            Color litColor = CalculateVertexLighting(vertexPos, normal, baseColor);
            
            mesh.colors[cCounter] = litColor.r;
            mesh.colors[cCounter+1] = litColor.g;
            mesh.colors[cCounter+2] = litColor.b;
            mesh.colors[cCounter+3] = litColor.a;
            
            vCounter += 3;
            nCounter += 3;
            tcCounter += 2;
            cCounter += 4;
        }
    }
    
    // Generate indices
    int tCounter = 0;
    for (int quad = 0; quad < (resX-1)*(resZ-1); quad++)
    {
        int currentVertex = quad + quad/(resX-1);
        
        mesh.indices[tCounter] = currentVertex;
        mesh.indices[tCounter+1] = currentVertex + resX + 1;
        mesh.indices[tCounter+2] = currentVertex + 1;
        
        mesh.indices[tCounter+3] = currentVertex;
        mesh.indices[tCounter+4] = currentVertex + resX;
        mesh.indices[tCounter+5] = currentVertex + resX + 1;
        
        tCounter += 6;
    }
    
    UploadMesh(&mesh, false);
    return mesh;
}

// Generate wall mesh with vertex colors
Mesh GenMeshWallWithColors(float width, float height, int resX, int resY)
{
    int vertexCount = resX * resY;
    int triangleCount = (resX-1) * (resY-1) * 2;
    
    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    
    mesh.vertices = (float *)MemAlloc(vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float *)MemAlloc(vertexCount * 2 * sizeof(float));
    mesh.normals = (float *)MemAlloc(vertexCount * 3 * sizeof(float));
    mesh.colors = (unsigned char *)MemAlloc(vertexCount * 4 * sizeof(unsigned char));
    mesh.indices = (unsigned short *)MemAlloc(triangleCount * 3 * sizeof(unsigned short));
    
    int vCounter = 0;
    int tcCounter = 0;
    int nCounter = 0;
    int cCounter = 0;
    
    for (int y = 0; y < resY; y++)
    {
        for (int x = 0; x < resX; x++)
        {
            // Vertices
            mesh.vertices[vCounter] = (float)x/(resX-1) * width - width/2;
            mesh.vertices[vCounter+1] = (float)y/(resY-1) * height;
            mesh.vertices[vCounter+2] = 0.0f;
            
            // Normals (facing forward)
            mesh.normals[nCounter] = 0.0f;
            mesh.normals[nCounter+1] = 0.0f;
            mesh.normals[nCounter+2] = 1.0f;
            
            // Texcoords
            mesh.texcoords[tcCounter] = (float)x/(resX-1);
            mesh.texcoords[tcCounter+1] = (float)y/(resY-1);
            
            // Vertex colors - brick pattern with lighting
            Color baseColor;
            if (y % 4 < 2 && x % 6 < 3) {
                baseColor = (Color){ 140, 70, 70, 255 };  // brick red
            } else if (y % 4 >= 2 && (x + 3) % 6 < 3) {
                baseColor = (Color){ 140, 70, 70, 255 };  // brick red
            } else {
                baseColor = (Color){ 180, 180, 180, 255 }; // mortar
            }
            
            Vector3 vertexPos = { mesh.vertices[vCounter], mesh.vertices[vCounter+1], mesh.vertices[vCounter+2] };
            Vector3 normal = { mesh.normals[nCounter], mesh.normals[nCounter+1], mesh.normals[nCounter+2] };
            Color litColor = CalculateVertexLighting(vertexPos, normal, baseColor);
            
            mesh.colors[cCounter] = litColor.r;
            mesh.colors[cCounter+1] = litColor.g;
            mesh.colors[cCounter+2] = litColor.b;
            mesh.colors[cCounter+3] = litColor.a;
            
            vCounter += 3;
            nCounter += 3;
            tcCounter += 2;
            cCounter += 4;
        }
    }
    
    // Generate indices
    int tCounter = 0;
    for (int quad = 0; quad < (resX-1)*(resY-1); quad++)
    {
        int currentVertex = quad + quad/(resX-1);
        
        mesh.indices[tCounter] = currentVertex;
        mesh.indices[tCounter+1] = currentVertex + resX + 1;
        mesh.indices[tCounter+2] = currentVertex + 1;
        
        mesh.indices[tCounter+3] = currentVertex;
        mesh.indices[tCounter+4] = currentVertex + resX;
        mesh.indices[tCounter+5] = currentVertex + resX + 1;
        
        tCounter += 6;
    }
    
    UploadMesh(&mesh, false);
    return mesh;
}

// Maze data structure
#define MAX_MAZE_WIDTH 50
#define MAX_MAZE_HEIGHT 50
typedef struct {
    char data[MAX_MAZE_HEIGHT][MAX_MAZE_WIDTH];
    int width;
    int height;
} Maze;

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

// Function to load maze from ASCII file
Maze LoadMazeFromFile(const char* filename) {
    Maze maze = {0};
    FILE* file = fopen(filename, "r");
    
    if (file == NULL) {
        TraceLog(LOG_ERROR, "Failed to open maze file: %s", filename);
        return maze;
    }
    
    char line[MAX_MAZE_WIDTH + 2]; // +2 for newline and null terminator
    int row = 0;
    
    while (fgets(line, sizeof(line), file) && row < MAX_MAZE_HEIGHT) {
        int col = 0;
        for (int i = 0; line[i] != '\0' && line[i] != '\n' && col < MAX_MAZE_WIDTH; i++) {
            maze.data[row][col] = line[i];
            col++;
        }
        
        if (col > maze.width) maze.width = col;
        row++;
    }
    
    maze.height = row;
    fclose(file);
    
    TraceLog(LOG_INFO, "Loaded maze: %dx%d", maze.width, maze.height);
    return maze;
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    // Initialize graphics configuration
    GraphicsConfig gfxConfig = {0};
    gfxConfig.antialiasingEnabled = true;
    gfxConfig.wireframeThickness = DEFAULT_WIREFRAME_THICKNESS;
    gfxConfig.highQualityRendering = true;
    
    SetConfigFlags(FLAG_VSYNC_HINT | (gfxConfig.antialiasingEnabled ? FLAG_MSAA_4X_HINT : 0));
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(screenWidth, screenHeight, "FPS Cube World - Pi 4");
    
    // We'll handle culling in the render loop if needed
    
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
    camera.position = (Vector3){ 0.0f, 2.5f, 10.0f };  // Move camera back and up to 2.5 units above ground
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };     // Look toward center
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    GameObject objects[MAX_CUBES];
    srand(time(NULL));
    
    // Generate basic 3D models - use simple mesh generation like the working test cubes
    Model cubeModel = LoadModelFromMesh(GenMeshCube(CUBE_SIZE, CUBE_SIZE, CUBE_SIZE));
    Model sphereModel = LoadModelFromMesh(GenMeshSphere(CUBE_SIZE/2.0f, 16, 16));
    Model cylinderModel = LoadModelFromMesh(GenMeshCylinder(CUBE_SIZE/2.0f, CUBE_SIZE, 8));
    
    // Generate custom floor and wall geometry with vertex colors
    Mesh floorMesh = GenMeshFloorWithColors(WORLD_SIZE * 2, WORLD_SIZE * 2, FLOOR_SEGMENTS, FLOOR_SEGMENTS);
    Model floorModel = LoadModelFromMesh(floorMesh);
    
    // Generate wall models for boundaries  
    Mesh wallMesh = GenMeshWallWithColors(WORLD_SIZE * 2, WALL_HEIGHT, 40, 10);
    Model wallModel = LoadModelFromMesh(wallMesh);
    
    // Load ASCII maze
    Maze maze = LoadMazeFromFile("maze.txt");
    
    // Clear all objects - no random primitives in the maze
    for (int i = 0; i < MAX_CUBES; i++)
    {
        objects[i].hasModel = false;
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
                
                // printf("Mouse delta: X=%.2f Y=%.2f, Yaw=%.2f Pitch=%.2f\n", 
                //     mouseDelta.x, mouseDelta.y, yaw, pitch);
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
        
        // Floor collision detection - prevent falling through
        if (camera.position.y < 2.5f) {
            camera.position.y = 2.5f;
            camera.target.y = camera.target.y + (2.5f - (camera.position.y - (camera.target.y - camera.position.y)));
        }
        
        BeginDrawing();
        
        ClearBackground(SKYBLUE);
        
        BeginMode3D(camera);
        
        // Add some basic lighting - draw sun at defined position
        DrawSphere((Vector3){ SUN_POSITION_X, SUN_POSITION_Y, SUN_POSITION_Z }, 5.0f, YELLOW);  // Sun/light source
        
        // Draw custom floor with vertex colors
        DrawModel(floorModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
        
        // No test cubes - clean maze environment
        
        // Draw maze walls from ASCII data
        float cellSize = 10.0f; // Size of each maze cell
        
        // Calculate maze offset to center it
        float mazeStartX = -(maze.width * cellSize) / 2.0f;
        float mazeStartZ = -(maze.height * cellSize) / 2.0f;
        
        for (int row = 0; row < maze.height; row++) {
            for (int col = 0; col < maze.width; col++) {
                if (maze.data[row][col] == '#') {
                    // Calculate wall position - walls should sit on the floor
                    float wallX = mazeStartX + col * cellSize;
                    float wallZ = mazeStartZ + row * cellSize;
                    float wallY = WALL_HEIGHT / 2.0f; // Center the wall vertically, bottom touches floor at y=0
                    
                    // Draw wall block using a scaled cube
                    Vector3 wallPos = (Vector3){ wallX, wallY, wallZ };
                    DrawCube(wallPos, cellSize, WALL_HEIGHT, cellSize, GRAY);
                    DrawCubeWiresThick(wallPos, cellSize, WALL_HEIGHT, cellSize, BLACK, &gfxConfig);
                }
            }
        }
        
        // No objects in the maze - just walls and floor
        
        DrawGrid(100, 1.0f);
        
        EndMode3D();
        
        DrawText("FPS Cube World", 10, 10, 20, BLACK);
        DrawText("WASD to move", 10, 30, 16, DARKGRAY);
        if (cursorLocked)
            DrawText("TAB to unlock cursor, Mouse to look", 10, 50, 16, DARKGRAY);
        else
            DrawText("TAB to lock cursor for mouse look", 10, 50, 16, DARKGRAY);
        
        DrawText("F1: Toggle Antialiasing, F2: Wireframe Thickness, F3: Quality", 10, 70, 14, DARKGRAY);
        
        // Display player coordinates and debug info
        // printf("Player Position: X=%.2f, Y=%.2f, Z=%.2f\n", 
        //        camera.position.x, camera.position.y, camera.position.z);
        char coordText[200];
        sprintf(coordText, "Position: X=%.1f Y=%.1f Z=%.1f", 
            camera.position.x, camera.position.y, camera.position.z);
            DrawText(coordText, 10, 90, 16, DARKGREEN);
            
            char debugText[200];
            sprintf(debugText, "Target: X=%.1f Y=%.1f Z=%.1f", 
                camera.target.x, camera.target.y, camera.target.z);
                DrawText(debugText, 10, 110, 16, DARKGREEN);
                
                sprintf(debugText, "Graphics: AA:%s Thickness:%.1f Quality:%s", 
                    gfxConfig.antialiasingEnabled ? "ON" : "OFF",
                    gfxConfig.wireframeThickness,
                    gfxConfig.highQualityRendering ? "HIGH" : "LOW");
                    DrawText(debugText, 10, 130, 16, DARKGREEN);
                    
                    sprintf(debugText, "Cursor: %s, Yaw=%.2f, Pitch=%.2f", 
                        cursorLocked ? "LOCKED" : "FREE", yaw * 180.0f / PI, pitch * 180.0f / PI);
                        DrawText(debugText, 10, 150, 16, DARKGREEN);
                        
                        DrawFPS(screenWidth - 100, 10);
                        
                        EndDrawing();
                    }
                    
                    // Cleanup models
                    UnloadModel(cubeModel);
                    UnloadModel(sphereModel);
                    UnloadModel(cylinderModel);
                    UnloadModel(floorModel);
                    UnloadModel(wallModel);
                    
                    CloseWindow();
                    
                    return 0;
                }