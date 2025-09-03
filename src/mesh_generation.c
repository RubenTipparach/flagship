#include "mesh_generation.h"
#include "lighting.h"

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
            Color litColor = CalculateSimpleLighting(vertexPos, normal, baseColor);
            
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

// Generate a cube mesh for maze walls with vertex colors and lighting
Mesh GenMeshMazeWallCube(float size, const LightingSystem* lighting, const GraphicsConfig* config)
{
    // Create a cube with subdivisions for better lighting
    int res = 4; // 4x4 subdivisions per face for smoother lighting
    int faceVertexCount = res * res;
    int facetriangleCount = (res-1) * (res-1) * 2;
    int totalVertices = faceVertexCount * 6; // 6 faces
    int totalTriangles = facetriangleCount * 6;
    
    Mesh mesh = { 0 };
    mesh.vertexCount = totalVertices;
    mesh.triangleCount = totalTriangles;
    
    mesh.vertices = (float *)MemAlloc(totalVertices * 3 * sizeof(float));
    mesh.texcoords = (float *)MemAlloc(totalVertices * 2 * sizeof(float));
    mesh.normals = (float *)MemAlloc(totalVertices * 3 * sizeof(float));
    mesh.colors = (unsigned char *)MemAlloc(totalVertices * 4 * sizeof(unsigned char));
    mesh.indices = (unsigned short *)MemAlloc(totalTriangles * 3 * sizeof(unsigned short));
    
    int vCounter = 0;
    int tcCounter = 0;
    int nCounter = 0;
    int cCounter = 0;
    int iCounter = 0;
    int vertexIndex = 0;
    
    float halfSize = size * 0.5f;
    
    // Define face data: position offset, normal, and texture coordinates
    Vector3 facePositions[6] = {
        {0, 0, halfSize},   // Front
        {0, 0, -halfSize},  // Back  
        {-halfSize, 0, 0},  // Left
        {halfSize, 0, 0},   // Right
        {0, halfSize, 0},   // Top
        {0, -halfSize, 0}   // Bottom
    };
    
    Vector3 faceNormals[6] = {
        {0, 0, 1},   // Front
        {0, 0, -1},  // Back
        {-1, 0, 0},  // Left
        {1, 0, 0},   // Right
        {0, 1, 0},   // Top
        {0, -1, 0}   // Bottom
    };
    
    Vector3 faceU[6] = { // U direction for each face
        {1, 0, 0}, {-1, 0, 0}, {0, 0, -1}, {0, 0, 1}, {1, 0, 0}, {1, 0, 0}
    };
    
    Vector3 faceV[6] = { // V direction for each face  
        {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 0, 1}, {0, 0, -1}
    };
    
    // Generate each face
    for (int face = 0; face < 6; face++) {
        Vector3 faceCenter = facePositions[face];
        Vector3 normal = faceNormals[face];
        Vector3 u = Vector3Scale(faceU[face], halfSize);
        Vector3 v = Vector3Scale(faceV[face], halfSize);
        
        // Generate vertices for this face
        for (int j = 0; j < res; j++) {
            for (int i = 0; i < res; i++) {
                float s = (float)i / (res - 1);
                float t = (float)j / (res - 1);
                
                // Calculate vertex position
                Vector3 vertexPos = Vector3Add(faceCenter, 
                    Vector3Add(Vector3Scale(u, (s - 0.5f) * 2.0f), 
                              Vector3Scale(v, (t - 0.5f) * 2.0f)));
                
                mesh.vertices[vCounter++] = vertexPos.x;
                mesh.vertices[vCounter++] = vertexPos.y;
                mesh.vertices[vCounter++] = vertexPos.z;
                
                // Set normal
                mesh.normals[nCounter++] = normal.x;
                mesh.normals[nCounter++] = normal.y;
                mesh.normals[nCounter++] = normal.z;
                
                // Set texture coordinates
                mesh.texcoords[tcCounter++] = s;
                mesh.texcoords[tcCounter++] = t;
                
                // Calculate lighting - choose base color based on face
                Color baseColor;
                if (face == 4) { // Top face
                    baseColor = (Color){ 150, 100, 100, 255 }; // Lighter brick for top
                } else if (face == 5) { // Bottom face
                    baseColor = (Color){ 80, 60, 60, 255 }; // Darker for bottom
                } else { // Side faces with brick pattern
                    int brickX = (int)(s * 8) % 6;
                    int brickY = (int)(t * 6) % 4;
                    if (brickY % 4 < 2 && brickX < 3) {
                        baseColor = (Color){ 140, 70, 70, 255 };  // brick red
                    } else if (brickY % 4 >= 2 && (brickX + 3) % 6 < 3) {
                        baseColor = (Color){ 140, 70, 70, 255 };  // brick red
                    } else {
                        baseColor = (Color){ 180, 180, 180, 255 }; // mortar
                    }
                }
                
                // Apply lighting
                Color litColor;
                if (config && config->advancedShadingEnabled && lighting) {
                    Vector3 viewDir = { 0.0f, 0.0f, 1.0f };
                    litColor = CalculateVertexLighting(vertexPos, normal, viewDir, baseColor, lighting, config);
                } else {
                    litColor = CalculateSimpleLighting(vertexPos, normal, baseColor);
                }
                
                mesh.colors[cCounter++] = litColor.r;
                mesh.colors[cCounter++] = litColor.g;
                mesh.colors[cCounter++] = litColor.b;
                mesh.colors[cCounter++] = litColor.a;
            }
        }
        
        // Generate indices for this face
        for (int j = 0; j < res-1; j++) {
            for (int i = 0; i < res-1; i++) {
                int topLeft = vertexIndex + j * res + i;
                int topRight = topLeft + 1;
                int bottomLeft = topLeft + res;
                int bottomRight = bottomLeft + 1;
                
                // First triangle
                mesh.indices[iCounter++] = topLeft;
                mesh.indices[iCounter++] = bottomLeft;
                mesh.indices[iCounter++] = topRight;
                
                // Second triangle
                mesh.indices[iCounter++] = topRight;
                mesh.indices[iCounter++] = bottomLeft;
                mesh.indices[iCounter++] = bottomRight;
            }
        }
        
        vertexIndex += faceVertexCount;
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
            Color litColor = CalculateSimpleLighting(vertexPos, normal, baseColor);
            
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