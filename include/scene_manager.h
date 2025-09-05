#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "game_types.h"
#include "lighting.h"

// Scene manager functions
SceneManager InitSceneManager(void);
void AddScene(SceneManager* manager, Scene scene);
void SwitchScene(SceneManager* manager, int sceneIndex);
void UpdateCurrentScene(SceneManager* manager, float deltaTime, Camera3D* camera);
void RenderCurrentScene(SceneManager* manager, Camera3D camera, GraphicsConfig* gfxConfig, struct WireframeShader* wireframeShader);
void CleanupSceneManager(SceneManager* manager);

// Scene creation functions
Scene CreateMazeScene(void);
Scene CreateTerrainScene(void);
Scene CreateCubeSphereScene(void);

#endif // SCENE_MANAGER_H