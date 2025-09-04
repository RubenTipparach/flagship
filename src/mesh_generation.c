#include "mesh_generation.h"
#include "lighting.h"
#include "raymath.h"
#include <math.h>

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
// Project a cube vertex to sphere surface
Vector3 ProjectCubeToSphere(Vector3 cubeVertex) {
    float x = cubeVertex.x;
    float y = cubeVertex.y;
    float z = cubeVertex.z;
    
    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;
    
    Vector3 sphereVertex;
    sphereVertex.x = x * sqrtf(1.0f - y2 * 0.5f - z2 * 0.5f + y2 * z2 / 3.0f);
    sphereVertex.y = y * sqrtf(1.0f - z2 * 0.5f - x2 * 0.5f + z2 * x2 / 3.0f);
    sphereVertex.z = z * sqrtf(1.0f - x2 * 0.5f - y2 * 0.5f + x2 * y2 / 3.0f);
    
    return sphereVertex;
}

// Calculate subdivision level based on camera distance
int CalculateSubdivisionLevel(Vector3 sphereCenter, Vector3 cameraPosition, float radius, int maxSubdivisions) {
    float distance = Vector3Distance(sphereCenter, cameraPosition);
    float ratio = distance / radius;
    
    int subdivisions;
    if (ratio < 2.0f) {
        subdivisions = maxSubdivisions;
    } else if (ratio < 5.0f) {
        subdivisions = maxSubdivisions - 1;
    } else if (ratio < 10.0f) {
        subdivisions = maxSubdivisions - 2;
    } else if (ratio < 20.0f) {
        subdivisions = maxSubdivisions - 3;
    } else {
        subdivisions = maxSubdivisions - 4;
    }
    
    return (subdivisions < 1) ? 1 : subdivisions;
}

// Generate a cube projected to sphere with dynamic tessellation
Mesh GenMeshCubeSphere(float radius, int subdivisions, Vector3 center) {
    int segmentsPerFace = (1 << subdivisions); // 2^subdivisions
    int verticesPerFace = (segmentsPerFace + 1) * (segmentsPerFace + 1);
    int totalVertices = verticesPerFace * 6;
    int trianglesPerFace = segmentsPerFace * segmentsPerFace * 2;
    int totalTriangles = trianglesPerFace * 6;
    
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
    
    // Define the 6 cube faces
    Vector3 faceNormals[6] = {
        {0, 0, 1},   // Front (+Z)
        {0, 0, -1},  // Back (-Z)
        {-1, 0, 0},  // Left (-X)
        {1, 0, 0},   // Right (+X)
        {0, 1, 0},   // Top (+Y)
        {0, -1, 0}   // Bottom (-Y)
    };
    
    Vector3 faceU[6] = {
        {1, 0, 0}, {-1, 0, 0}, {0, 0, -1}, {0, 0, 1}, {1, 0, 0}, {1, 0, 0}
    };
    
    Vector3 faceV[6] = {
        {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 0, 1}, {0, 0, -1}
    };
    
    // Generate vertices for each face
    for (int face = 0; face < 6; face++) {
        Vector3 normal = faceNormals[face];
        Vector3 u = faceU[face];
        Vector3 v = faceV[face];
        
        for (int j = 0; j <= segmentsPerFace; j++) {
            for (int i = 0; i <= segmentsPerFace; i++) {
                float s = (float)i / segmentsPerFace;
                float t = (float)j / segmentsPerFace;
                
                // Create cube vertex on face
                Vector3 cubeVertex = Vector3Add(normal, 
                    Vector3Add(Vector3Scale(u, (s - 0.5f) * 2.0f),
                              Vector3Scale(v, (t - 0.5f) * 2.0f)));
                
                // Project to sphere
                Vector3 sphereVertex = ProjectCubeToSphere(cubeVertex);
                sphereVertex = Vector3Scale(sphereVertex, radius);
                sphereVertex = Vector3Add(sphereVertex, center);
                
                mesh.vertices[vCounter++] = sphereVertex.x;
                mesh.vertices[vCounter++] = sphereVertex.y;
                mesh.vertices[vCounter++] = sphereVertex.z;
                
                // Normal is the normalized sphere vertex direction
                Vector3 sphereNormal = Vector3Normalize(Vector3Subtract(sphereVertex, center));
                mesh.normals[nCounter++] = sphereNormal.x;
                mesh.normals[nCounter++] = sphereNormal.y;
                mesh.normals[nCounter++] = sphereNormal.z;
                
                // Texture coordinates
                mesh.texcoords[tcCounter++] = s;
                mesh.texcoords[tcCounter++] = t;
                
                // Color based on face and position for visual variety
                Color baseColor;
                switch (face) {
                    case 0: baseColor = (Color){255, 100, 100, 255}; break; // Red front
                    case 1: baseColor = (Color){100, 255, 100, 255}; break; // Green back  
                    case 2: baseColor = (Color){100, 100, 255, 255}; break; // Blue left
                    case 3: baseColor = (Color){255, 255, 100, 255}; break; // Yellow right
                    case 4: baseColor = (Color){255, 100, 255, 255}; break; // Magenta top
                    case 5: baseColor = (Color){100, 255, 255, 255}; break; // Cyan bottom
                }
                
                // Add some variation based on position
                float variation = 0.8f + 0.4f * sinf(s * 10.0f) * cosf(t * 10.0f);
                baseColor.r = (unsigned char)(baseColor.r * variation);
                baseColor.g = (unsigned char)(baseColor.g * variation);
                baseColor.b = (unsigned char)(baseColor.b * variation);
                
                Color litColor = CalculateSimpleLighting(sphereVertex, sphereNormal, baseColor);
                
                mesh.colors[cCounter++] = litColor.r;
                mesh.colors[cCounter++] = litColor.g;
                mesh.colors[cCounter++] = litColor.b;
                mesh.colors[cCounter++] = litColor.a;
            }
        }
        
        // Generate indices for this face
        for (int j = 0; j < segmentsPerFace; j++) {
            for (int i = 0; i < segmentsPerFace; i++) {
                int topLeft = vertexIndex + j * (segmentsPerFace + 1) + i;
                int topRight = topLeft + 1;
                int bottomLeft = topLeft + (segmentsPerFace + 1);
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
        
        vertexIndex += verticesPerFace;
    }
    
    UploadMesh(&mesh, false);
    return mesh;
}

// Generate a subdivided cube that can morph towards a sphere
Mesh GenMeshSubdividedCube(float size, int subdivisions, float morphFactor) {
    int segmentsPerFace = subdivisions + 1; // Number of segments per edge
    int verticesPerFace = (segmentsPerFace + 1) * (segmentsPerFace + 1);
    int totalVertices = verticesPerFace * 6;
    int trianglesPerFace = segmentsPerFace * segmentsPerFace * 2;
    int totalTriangles = trianglesPerFace * 6;
    
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
    
    // Define the 6 cube faces
    Vector3 faceNormals[6] = {
        {0, 0, -1},   // Front (+Z)
        {0, 0, 1},  // Back (-Z)
        {-1, 0, 0},  // Left (-X)
        {1, 0, 0},   // Right (+X)
        {0, 1, 0},   // Top (+Y)
        {0, -1, 0}   // Bottom (-Y)
    };
    
    Vector3 faceU[6] = {
        {1, 0, 0}, {-1, 0, 0}, {0, 0, -1}, {0, 0, 1}, {1, 0, 0}, {1, 0, 0}
    };
    
    Vector3 faceV[6] = {
        {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 0, 1}, {0, 0, -1}
    };
    
    // Colors for each face to visualize the cube structure
    Color faceColors[6] = {
        {255, 100, 100, 255}, // Red front
        {100, 255, 100, 255}, // Green back  
        {100, 100, 255, 255}, // Blue left
        {255, 255, 100, 255}, // Yellow right
        {255, 100, 255, 255}, // Magenta top
        {100, 255, 255, 255}  // Cyan bottom
    };
    
    // Generate vertices for each face
    for (int face = 0; face < 6; face++) {
        Vector3 normal = faceNormals[face];
        Vector3 u = faceU[face];
        Vector3 v = faceV[face];
        Color baseColor = faceColors[face];
        
        for (int j = 0; j <= segmentsPerFace; j++) {
            for (int i = 0; i <= segmentsPerFace; i++) {
                float s = (float)i / segmentsPerFace;
                float t = (float)j / segmentsPerFace;
                
                // Calculate cube vertex position
                Vector3 cubePos;
                cubePos.x = normal.x * halfSize + u.x * halfSize * (s - 0.5f) * 2.0f + v.x * halfSize * (t - 0.5f) * 2.0f;
                cubePos.y = normal.y * halfSize + u.y * halfSize * (s - 0.5f) * 2.0f + v.y * halfSize * (t - 0.5f) * 2.0f;
                cubePos.z = normal.z * halfSize + u.z * halfSize * (s - 0.5f) * 2.0f + v.z * halfSize * (t - 0.5f) * 2.0f;
                
                // Calculate sphere position (normalized to halfSize radius)
                float length = sqrtf(cubePos.x * cubePos.x + cubePos.y * cubePos.y + cubePos.z * cubePos.z);
                Vector3 spherePos = {
                    cubePos.x / length * halfSize,
                    cubePos.y / length * halfSize,
                    cubePos.z / length * halfSize
                };
                
                // Interpolate between cube and sphere based on morphFactor
                Vector3 finalPos = {
                    cubePos.x * (1.0f - morphFactor) + spherePos.x * morphFactor,
                    cubePos.y * (1.0f - morphFactor) + spherePos.y * morphFactor,
                    cubePos.z * (1.0f - morphFactor) + spherePos.z * morphFactor
                };
                
                mesh.vertices[vCounter++] = finalPos.x;
                mesh.vertices[vCounter++] = finalPos.y;
                mesh.vertices[vCounter++] = finalPos.z;
                
                // Calculate normal - interpolate between face normal and position normal
                Vector3 posNormal = {
                    finalPos.x / halfSize,
                    finalPos.y / halfSize,
                    finalPos.z / halfSize
                };
                float normalLength = sqrtf(posNormal.x * posNormal.x + posNormal.y * posNormal.y + posNormal.z * posNormal.z);
                if (normalLength > 0.001f) {
                    posNormal.x /= normalLength;
                    posNormal.y /= normalLength;
                    posNormal.z /= normalLength;
                }
                
                Vector3 finalNormal = {
                    normal.x * (1.0f - morphFactor) + posNormal.x * morphFactor,
                    normal.y * (1.0f - morphFactor) + posNormal.y * morphFactor,
                    normal.z * (1.0f - morphFactor) + posNormal.z * morphFactor
                };
                
                mesh.normals[nCounter++] = finalNormal.x;
                mesh.normals[nCounter++] = finalNormal.y;
                mesh.normals[nCounter++] = finalNormal.z;
                
                // Texture coordinates
                mesh.texcoords[tcCounter++] = s;
                mesh.texcoords[tcCounter++] = t;
                
                // Color with some variation
                float variation = 0.8f + 0.4f * sinf(s * 10.0f) * cosf(t * 10.0f);
                Color finalColor = {
                    (unsigned char)(baseColor.r * variation),
                    (unsigned char)(baseColor.g * variation),
                    (unsigned char)(baseColor.b * variation),
                    baseColor.a
                };
                
                // Apply simple lighting
                Color litColor = CalculateSimpleLighting(finalPos, finalNormal, finalColor);
                
                mesh.colors[cCounter++] = litColor.r;
                mesh.colors[cCounter++] = litColor.g;
                mesh.colors[cCounter++] = litColor.b;
                mesh.colors[cCounter++] = litColor.a;
            }
        }
        
        // Generate indices for this face
        for (int j = 0; j < segmentsPerFace; j++) {
            for (int i = 0; i < segmentsPerFace; i++) {
                int topLeft = vertexIndex + j * (segmentsPerFace + 1) + i;
                int topRight = topLeft + 1;
                int bottomLeft = topLeft + (segmentsPerFace + 1);
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
        
        vertexIndex += verticesPerFace;
    }
    
    UploadMesh(&mesh, false);
    return mesh;
}
