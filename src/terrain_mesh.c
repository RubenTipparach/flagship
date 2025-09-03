#include "mesh_generation.h"
#include "raylib.h"
#include "raymath.h"

// Calculate the actual maximum height in the terrain
float GetTerrainMaxHeight(const TerrainData* terrain, float heightScale) {
    float maxHeight = 0.0f;
    
    for (int z = 0; z < terrain->size; z++) {
        for (int x = 0; x < terrain->size; x++) {
            float height = terrain->heights[z][x] * heightScale * terrain->heightMultiplier;
            if (height > maxHeight) {
                maxHeight = height;
            }
        }
    }
    
    return maxHeight;
}

// Smooth color interpolation between two colors
Color LerpColor(Color a, Color b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    
    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
}

// Get terrain color based on height with gradual gradients
Color GetTerrainColorByHeight(float height, float maxHeight) {
    if (maxHeight <= 0.0f) {
        // Flat terrain - return base color
        return (Color){100, 150, 100, 255}; // Green
    }
    
    // Normalize height to 0-1 range
    float normalizedHeight = height / maxHeight;
    if (normalizedHeight < 0.0f) normalizedHeight = 0.0f;
    if (normalizedHeight > 1.0f) normalizedHeight = 1.0f;
    
    // Define color stops and thresholds
    Color deepWater = {20, 50, 120, 255};      // Deep blue
    Color shallowWater = {40, 80, 180, 255};   // Light blue
    Color beach = {194, 178, 128, 255};        // Light tan/sand
    Color grass = {85, 140, 45, 255};          // Green
    Color darkGrass = {60, 100, 30, 255};      // Dark green
    Color rock = {80, 70, 60, 255};            // Brown
    Color snow = {240, 245, 255, 255};         // White snow
    
    // Define gradual transitions
    if (normalizedHeight < 0.1f) {
        // Deep water to shallow water
        float t = normalizedHeight / 0.1f;
        return LerpColor(deepWater, shallowWater, t);
    }
    else if (normalizedHeight < 0.2f) {
        // Shallow water to beach
        float t = (normalizedHeight - 0.1f) / 0.1f;
        return LerpColor(shallowWater, beach, t);
    }
    else if (normalizedHeight < 0.35f) {
        // Beach to grass
        float t = (normalizedHeight - 0.2f) / 0.15f;
        return LerpColor(beach, grass, t);
    }
    else if (normalizedHeight < 0.6f) {
        // Grass to dark grass
        float t = (normalizedHeight - 0.35f) / 0.25f;
        return LerpColor(grass, darkGrass, t);
    }
    else if (normalizedHeight < 0.8f) {
        // Dark grass to rock/brown
        float t = (normalizedHeight - 0.6f) / 0.2f;
        return LerpColor(darkGrass, rock, t);
    }
    else {
        // Rock to snow
        float t = (normalizedHeight - 0.8f) / 0.2f;
        return LerpColor(rock, snow, t);
    }
}

// Generate terrain mesh from height map with proper square quads
Mesh GenMeshTerrainFromHeightMap(const TerrainData* terrain, float scale, float heightScale)
{
    // Use a smaller resolution for clearer quad structure
    int resolution = 128; // 128x128 grid of quads
    int vertexCount = (resolution + 1) * (resolution + 1);  // Need one extra vertex per dimension
    int quadCount = resolution * resolution;
    int triangleCount = quadCount * 2;  // 2 triangles per quad
    
    Mesh mesh = { 0 };
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = triangleCount;
    
    mesh.vertices = (float *)MemAlloc(vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float *)MemAlloc(vertexCount * 2 * sizeof(float));
    mesh.normals = (float *)MemAlloc(vertexCount * 3 * sizeof(float));
    mesh.colors = (unsigned char *)MemAlloc(vertexCount * 4 * sizeof(unsigned char));
    mesh.indices = (unsigned short *)MemAlloc(triangleCount * 3 * sizeof(unsigned short));
    
    // Calculate plane dimensions to create equal width/height quads
    float planeWidth = 100.0f;   // Total plane width
    float planeHeight = 100.0f;  // Total plane height (same as width for square)
    float quadSize = planeWidth / resolution;  // Size of each quad
    
    int vCounter = 0;
    int tcCounter = 0;
    int nCounter = 0;
    int cCounter = 0;
    
    // Calculate the maximum height for proper color scaling
    float maxTerrainHeight = GetTerrainMaxHeight(terrain, heightScale);
    
    // Generate vertices in a grid pattern for square plane
    for (int z = 0; z <= resolution; z++)  // Note: <= to get the extra row/column of vertices
    {
        for (int x = 0; x <= resolution; x++)
        {
            // Calculate world position for this vertex
            float worldX = (float)x * quadSize - planeWidth * 0.5f;   // X position centered at origin
            float worldZ = (float)z * quadSize - planeHeight * 0.5f;  // Z position centered at origin
            
            // Bilinear interpolation for smooth height sampling
            float heightMapX = (float)x / resolution * (terrain->size - 1);
            float heightMapZ = (float)z / resolution * (terrain->size - 1);
            
            // Get integer and fractional parts
            int x0 = (int)floor(heightMapX);
            int z0 = (int)floor(heightMapZ);
            int x1 = x0 + 1;
            int z1 = z0 + 1;
            
            float fx = heightMapX - x0;
            float fz = heightMapZ - z0;
            
            // Clamp to height map bounds
            x0 = (x0 < 0) ? 0 : ((x0 >= terrain->size) ? terrain->size - 1 : x0);
            x1 = (x1 < 0) ? 0 : ((x1 >= terrain->size) ? terrain->size - 1 : x1);
            z0 = (z0 < 0) ? 0 : ((z0 >= terrain->size) ? terrain->size - 1 : z0);
            z1 = (z1 < 0) ? 0 : ((z1 >= terrain->size) ? terrain->size - 1 : z1);
            
            // Sample four neighboring heights
            float h00 = terrain->heights[z0][x0];
            float h10 = terrain->heights[z0][x1];
            float h01 = terrain->heights[z1][x0];
            float h11 = terrain->heights[z1][x1];
            
            // Bilinear interpolation
            float h0 = h00 + (h10 - h00) * fx;  // Top edge
            float h1 = h01 + (h11 - h01) * fx;  // Bottom edge
            float interpolatedHeight = h0 + (h1 - h0) * fz;  // Final interpolated height
            
            float height = interpolatedHeight * heightScale * terrain->heightMultiplier;
            
            // Set vertex position (XZ plane with Y height)
            mesh.vertices[vCounter] = worldX;     // X coordinate
            mesh.vertices[vCounter+1] = height;   // Y coordinate (height displacement)
            mesh.vertices[vCounter+2] = worldZ;   // Z coordinate
            
            // Texture coordinates
            mesh.texcoords[tcCounter] = (float)x / resolution;
            mesh.texcoords[tcCounter+1] = (float)z / resolution;
            
            // Calculate smooth normal using neighboring interpolated heights
            Vector3 normal = {0, 1, 0}; // Default to up (flat plane)
            
            if (x > 0 && x < resolution && z > 0 && z < resolution) {
                // Sample neighboring heights with interpolation for smooth normals
                float offset = 1.0f;
                
                // Left and right heights
                float leftX = ((float)(x-1) / resolution) * (terrain->size - 1);
                float rightX = ((float)(x+1) / resolution) * (terrain->size - 1);
                float currentZ = ((float)z / resolution) * (terrain->size - 1);
                
                // Get interpolated left and right heights
                float hL = 0.0f, hR = 0.0f;
                if (leftX >= 0 && rightX < terrain->size) {
                    // Simplified sampling for normal calculation
                    int lx = (int)leftX, rx = (int)rightX, cz = (int)currentZ;
                    if (lx >= 0 && rx < terrain->size && cz >= 0 && cz < terrain->size) {
                        hL = terrain->heights[cz][lx] * heightScale * terrain->heightMultiplier;
                        hR = terrain->heights[cz][rx] * heightScale * terrain->heightMultiplier;
                    }
                }
                
                // Down and up heights
                float downZ = ((float)(z-1) / resolution) * (terrain->size - 1);
                float upZ = ((float)(z+1) / resolution) * (terrain->size - 1);
                float currentX = ((float)x / resolution) * (terrain->size - 1);
                
                float hD = 0.0f, hU = 0.0f;
                if (downZ >= 0 && upZ < terrain->size) {
                    int cx = (int)currentX, dz = (int)downZ, uz = (int)upZ;
                    if (cx >= 0 && cx < terrain->size && dz >= 0 && uz < terrain->size) {
                        hD = terrain->heights[dz][cx] * heightScale * terrain->heightMultiplier;
                        hU = terrain->heights[uz][cx] * heightScale * terrain->heightMultiplier;
                    }
                }
                
                // Calculate normal using cross product
                Vector3 horizontal = {2.0f * quadSize, hR - hL, 0.0f};
                Vector3 vertical = {0.0f, hU - hD, 2.0f * quadSize};
                
                normal = Vector3CrossProduct(horizontal, vertical);
                normal = Vector3Normalize(normal);
            }
            
            mesh.normals[nCounter] = normal.x;
            mesh.normals[nCounter+1] = normal.y;
            mesh.normals[nCounter+2] = normal.z;
            
            // Get terrain color using new gradual gradient system
            Color vertexColor = GetTerrainColorByHeight(height, maxTerrainHeight);
            
            mesh.colors[cCounter] = vertexColor.r;
            mesh.colors[cCounter+1] = vertexColor.g;
            mesh.colors[cCounter+2] = vertexColor.b;
            mesh.colors[cCounter+3] = vertexColor.a;
            
            vCounter += 3;
            nCounter += 3;
            tcCounter += 2;
            cCounter += 4;
        }
    }
    
    // Generate indices for proper quads (each quad = 2 triangles)
    int tCounter = 0;
    int verticesPerRow = resolution + 1;  // Number of vertices per row/column
    
    for (int z = 0; z < resolution; z++)  // For each row of quads
    {
        for (int x = 0; x < resolution; x++)  // For each quad in the row
        {
            // Calculate vertex indices for this quad
            int topLeft = z * verticesPerRow + x;
            int topRight = z * verticesPerRow + x + 1;
            int bottomLeft = (z + 1) * verticesPerRow + x;
            int bottomRight = (z + 1) * verticesPerRow + x + 1;
            
            // First triangle of the quad (top-left, bottom-left, top-right)
            mesh.indices[tCounter] = topLeft;
            mesh.indices[tCounter+1] = bottomLeft;
            mesh.indices[tCounter+2] = topRight;
            
            // Second triangle of the quad (top-right, bottom-left, bottom-right)
            mesh.indices[tCounter+3] = topRight;
            mesh.indices[tCounter+4] = bottomLeft;
            mesh.indices[tCounter+5] = bottomRight;
            
            tCounter += 6;
        }
    }
    
    UploadMesh(&mesh, false);
    return mesh;
}