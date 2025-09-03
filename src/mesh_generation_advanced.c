#include "mesh_generation.h"
#include "lighting.h"
#include <math.h>

// Generate floor mesh with advanced lighting
Mesh GenMeshFloorWithAdvancedLighting(float width, float height, int resX, int resZ, const LightingSystem* lighting, const GraphicsConfig* config)
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
            
            // Enhanced vertex colors with procedural patterns
            Color baseColor;
            float noiseValue = (sinf(x * 0.5f) * cosf(z * 0.5f) + 1.0f) * 0.5f;
            
            if ((x + z) % 2 == 0) {
                baseColor = (Color){ 
                    (unsigned char)(80 + noiseValue * 40), 
                    (unsigned char)(140 + noiseValue * 20), 
                    (unsigned char)(80 + noiseValue * 40), 
                    255 
                };
            } else {
                baseColor = (Color){ 
                    (unsigned char)(60 + noiseValue * 30), 
                    (unsigned char)(120 + noiseValue * 15), 
                    (unsigned char)(60 + noiseValue * 30), 
                    255 
                };
            }
            
            Vector3 vertexPos = { mesh.vertices[vCounter], mesh.vertices[vCounter+1], mesh.vertices[vCounter+2] };
            Vector3 normal = { mesh.normals[nCounter], mesh.normals[nCounter+1], mesh.normals[nCounter+2] };
            Vector3 viewDir = { 0.0f, 1.0f, 0.0f }; // Default view direction for pre-calculated lighting
            
            Color litColor = CalculateVertexLighting(vertexPos, normal, viewDir, baseColor, lighting, config);
            
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

// Generate wall mesh with advanced lighting
Mesh GenMeshWallWithAdvancedLighting(float width, float height, int resX, int resY, const LightingSystem* lighting, const GraphicsConfig* config)
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
            
            // Enhanced brick pattern with weathering effects
            Color baseColor;
            float weathering = (sinf(x * 0.3f + y * 0.7f) + 1.0f) * 0.5f * 0.3f;
            
            if (y % 4 < 2 && x % 6 < 3) {
                baseColor = (Color){ 
                    (unsigned char)(140 - weathering * 30), 
                    (unsigned char)(70 - weathering * 20), 
                    (unsigned char)(70 - weathering * 20), 
                    255 
                };
            } else if (y % 4 >= 2 && (x + 3) % 6 < 3) {
                baseColor = (Color){ 
                    (unsigned char)(140 - weathering * 30), 
                    (unsigned char)(70 - weathering * 20), 
                    (unsigned char)(70 - weathering * 20), 
                    255 
                };
            } else {
                baseColor = (Color){ 
                    (unsigned char)(180 - weathering * 40), 
                    (unsigned char)(180 - weathering * 40), 
                    (unsigned char)(180 - weathering * 40), 
                    255 
                };
            }
            
            Vector3 vertexPos = { mesh.vertices[vCounter], mesh.vertices[vCounter+1], mesh.vertices[vCounter+2] };
            Vector3 normal = { mesh.normals[nCounter], mesh.normals[nCounter+1], mesh.normals[nCounter+2] };
            Vector3 viewDir = { 0.0f, 0.0f, 1.0f }; // Default view direction for pre-calculated lighting
            
            Color litColor = CalculateVertexLighting(vertexPos, normal, viewDir, baseColor, lighting, config);
            
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