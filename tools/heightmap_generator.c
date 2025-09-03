#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define HEIGHTMAP_SIZE 1024
#define OUTPUT_FILENAME "heightmap.png"

// Simple hash function for random values
float hash(int x, int y, int seed) {
    int n = x + y * 57 + seed * 131;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

// Smooth interpolation function
float smootherStep(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

// Bilinear interpolation
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

// Improved Perlin-style noise function
float perlinNoise(float x, float y, int seed) {
    int xi = (int)floor(x);
    int yi = (int)floor(y);
    
    float xf = x - xi;
    float yf = y - yi;
    
    // Get random values at grid corners
    float a = hash(xi, yi, seed);
    float b = hash(xi + 1, yi, seed);
    float c = hash(xi, yi + 1, seed);
    float d = hash(xi + 1, yi + 1, seed);
    
    // Smooth interpolation
    float u = smootherStep(xf);
    float v = smootherStep(yf);
    
    // Bilinear interpolation
    float x1 = lerp(a, b, u);
    float x2 = lerp(c, d, u);
    
    return lerp(x1, x2, v);
}

// Fractal Brownian Motion - multiple octaves of noise
float fbm(float x, float y, int octaves, float persistence, float scale, int seed) {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = scale;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; i++) {
        value += perlinNoise(x * frequency, y * frequency, seed + i) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    return value / maxValue;
}

// Generate smooth Perlin noise based terrain
void GenerateIslandHeightMap(unsigned char* heightData) {
    printf("Generating Perlin noise terrain...\n");
    
    float centerX = HEIGHTMAP_SIZE / 2.0f;
    float centerY = HEIGHTMAP_SIZE / 2.0f;
    float maxDistance = sqrt(centerX * centerX + centerY * centerY);
    
    int seed = rand();
    
    for (int y = 0; y < HEIGHTMAP_SIZE; y++) {
        for (int x = 0; x < HEIGHTMAP_SIZE; x++) {
            int index = y * HEIGHTMAP_SIZE + x;
            
            // Calculate distance from center
            float dx = x - centerX;
            float dy = y - centerY;
            float distance = sqrt(dx * dx + dy * dy);
            float normalizedDistance = distance / maxDistance;
            
            // Island falloff - creates higher center, lower edges
            float islandFactor = 1.0f - pow(normalizedDistance, 2.0f);
            if (islandFactor < 0.0f) islandFactor = 0.0f;
            
            // Multiple octaves of Perlin noise for varied terrain
            float nx = (float)x / HEIGHTMAP_SIZE;
            float ny = (float)y / HEIGHTMAP_SIZE;
            
            // Large scale terrain features
            float largeFeatures = fbm(nx, ny, 4, 0.5f, 2.0f, seed);
            
            // Medium scale features (hills and valleys)
            float mediumFeatures = fbm(nx, ny, 6, 0.4f, 8.0f, seed + 1000);
            
            // Small scale details (surface roughness)
            float smallFeatures = fbm(nx, ny, 8, 0.3f, 32.0f, seed + 2000);
            
            // Combine different scales
            float terrainHeight = largeFeatures * 0.6f + 
                                 mediumFeatures * 0.3f + 
                                 smallFeatures * 0.1f;
            
            // Apply island falloff to make center higher than edges
            float finalHeight = terrainHeight * islandFactor;
            
            // Add a base elevation to center to ensure it's higher
            float centerBoost = (1.0f - normalizedDistance) * 0.3f;
            finalHeight += centerBoost;
            
            // Normalize to 0-1 range
            finalHeight = (finalHeight + 1.0f) * 0.5f;
            if (finalHeight < 0.0f) finalHeight = 0.0f;
            if (finalHeight > 1.0f) finalHeight = 1.0f;
            
            // Invert colors: white = high, black = low
            unsigned char heightValue = (unsigned char)(finalHeight * 255.0f);
            heightData[index] = heightValue;  // Direct assignment (white = high)
        }
        
        // Progress indicator
        if (y % 100 == 0) {
            printf("Progress: %d%%\n", (y * 100) / HEIGHTMAP_SIZE);
        }
    }
}

int main(int argc, char* argv[]) {
    printf("Island Height Map Generator\n");
    printf("Generating %dx%d height map...\n", HEIGHTMAP_SIZE, HEIGHTMAP_SIZE);
    
    // Initialize random seed
    srand(time(NULL));
    
    // If a seed is provided as argument, use it
    if (argc > 1) {
        unsigned int seed = (unsigned int)atoi(argv[1]);
        srand(seed);
        printf("Using seed: %u\n", seed);
    }
    
    // Allocate memory for height data
    unsigned char* heightData = (unsigned char*)malloc(HEIGHTMAP_SIZE * HEIGHTMAP_SIZE);
    if (!heightData) {
        printf("Error: Could not allocate memory for height data\n");
        return -1;
    }
    
    // Generate the height map
    GenerateIslandHeightMap(heightData);
    
    // Create image from height data
    Image heightImage = {
        .data = heightData,
        .width = HEIGHTMAP_SIZE,
        .height = HEIGHTMAP_SIZE,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
    };
    
    // Initialize raylib (minimal initialization for image operations)
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(1, 1, "Hidden Window");
    
    // Export the height map as PNG
    if (ExportImage(heightImage, OUTPUT_FILENAME)) {
        printf("Height map saved as: %s\n", OUTPUT_FILENAME);
        printf("Height map specifications:\n");
        printf("  Size: %dx%d pixels\n", HEIGHTMAP_SIZE, HEIGHTMAP_SIZE);
        printf("  Format: 8-bit grayscale PNG\n");
        printf("  Black (0) = Sea level\n");
        printf("  White (255) = Maximum height\n");
        printf("  Island shape with beaches, hills, and ridges\n");
    } else {
        printf("Error: Could not save height map to %s\n", OUTPUT_FILENAME);
        free(heightData);
        CloseWindow();
        return -1;
    }
    
    // Cleanup
    CloseWindow();
    free(heightData);
    
    printf("Height map generation complete!\n");
    printf("Usage in game: Place %s in the game directory\n", OUTPUT_FILENAME);
    printf("The terrain scene will automatically load and use this height map.\n");
    
    return 0;
}