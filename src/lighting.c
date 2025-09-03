#include "lighting.h"
#include <math.h>

// Initialize the lighting system
LightingSystem InitLightingSystem(void)
{
    LightingSystem lighting = {0};
    lighting.lightCount = 0;
    lighting.ambientColor = (Color){30, 30, 40, 255};
    lighting.ambientIntensity = 0.2f;
    return lighting;
}

// Add a light to the lighting system
void AddLight(LightingSystem* lighting, LightType type, Vector3 position, Vector3 direction, Color color, float intensity, float range, float spotAngle)
{
    if (lighting->lightCount >= MAX_LIGHTS) return;
    
    Light* light = &lighting->lights[lighting->lightCount];
    light->type = type;
    light->position = position;
    light->direction = Vector3Normalize(direction);
    light->color = color;
    light->intensity = intensity;
    light->range = range;
    light->spotAngle = spotAngle;
    light->enabled = true;
    
    lighting->lightCount++;
}

// Update lighting system (for dynamic lights)
void UpdateLightingSystem(LightingSystem* lighting, float deltaTime)
{
    // Example: Animate first light if it exists
    if (lighting->lightCount > 0) {
        static float time = 0.0f;
        time += deltaTime;
        
        // Make the first light orbit around the center
        if (lighting->lights[0].type == LIGHT_POINT) {
            float radius = 80.0f;
            lighting->lights[0].position.x = cosf(time * 0.5f) * radius;
            lighting->lights[0].position.z = sinf(time * 0.5f) * radius;
        }
    }
}

// Calculate Blinn-Phong shading
static Color CalculateBlinnPhong(Vector3 vertexPos, Vector3 normal, Vector3 viewDir, Vector3 lightDir, Color lightColor, float lightIntensity, Color baseColor, float specularStrength, float shininess)
{
    // Diffuse lighting (Lambert)
    float NdotL = fmaxf(0.0f, Vector3DotProduct(normal, lightDir));
    
    // Specular lighting (Blinn-Phong)
    Vector3 halfDir = Vector3Normalize(Vector3Add(lightDir, viewDir));
    float NdotH = fmaxf(0.0f, Vector3DotProduct(normal, halfDir));
    float specular = powf(NdotH, shininess) * specularStrength;
    
    // Combine diffuse and specular
    float totalIntensity = lightIntensity * NdotL;
    float specularContribution = lightIntensity * specular;
    
    Color result;
    result.r = (unsigned char)fminf(255, (baseColor.r * totalIntensity) + (lightColor.r * specularContribution));
    result.g = (unsigned char)fminf(255, (baseColor.g * totalIntensity) + (lightColor.g * specularContribution));
    result.b = (unsigned char)fminf(255, (baseColor.b * totalIntensity) + (lightColor.b * specularContribution));
    result.a = baseColor.a;
    
    return result;
}

// Calculate lighting for a vertex with multiple lights
Color CalculateVertexLighting(Vector3 vertexPos, Vector3 normal, Vector3 viewDir, Color baseColor, const LightingSystem* lighting, const GraphicsConfig* config)
{
    if (!config->advancedShadingEnabled) {
        return CalculateSimpleLighting(vertexPos, normal, baseColor);
    }
    
    // Start with ambient lighting
    float ambientR = lighting->ambientColor.r * lighting->ambientIntensity / 255.0f;
    float ambientG = lighting->ambientColor.g * lighting->ambientIntensity / 255.0f;
    float ambientB = lighting->ambientColor.b * lighting->ambientIntensity / 255.0f;
    
    float totalR = baseColor.r * ambientR;
    float totalG = baseColor.g * ambientG;
    float totalB = baseColor.b * ambientB;
    
    // Process each light
    for (int i = 0; i < lighting->lightCount; i++) {
        const Light* light = &lighting->lights[i];
        if (!light->enabled) continue;
        
        Vector3 lightDir;
        float attenuation = 1.0f;
        
        switch (light->type) {
            case LIGHT_DIRECTIONAL:
                lightDir = Vector3Scale(light->direction, -1.0f); // Light direction points toward light
                break;
                
            case LIGHT_POINT: {
                Vector3 toLight = Vector3Subtract(light->position, vertexPos);
                float distance = Vector3Length(toLight);
                if (distance > light->range) continue; // Outside light range
                
                lightDir = Vector3Scale(toLight, 1.0f / distance); // Normalize
                attenuation = 1.0f / (1.0f + 0.09f * distance + 0.032f * distance * distance);
                break;
            }
            
            case LIGHT_SPOT: {
                Vector3 toLight = Vector3Subtract(light->position, vertexPos);
                float distance = Vector3Length(toLight);
                if (distance > light->range) continue; // Outside light range
                
                lightDir = Vector3Scale(toLight, 1.0f / distance); // Normalize
                
                // Check if within spot cone
                float spotCos = Vector3DotProduct(Vector3Scale(lightDir, -1.0f), light->direction);
                float spotCutoff = cosf(light->spotAngle * PI / 180.0f);
                if (spotCos < spotCutoff) continue; // Outside spot cone
                
                attenuation = 1.0f / (1.0f + 0.09f * distance + 0.032f * distance * distance);
                attenuation *= spotCos; // Additional spot attenuation
                break;
            }
        }
        
        // Calculate lighting contribution from this light
        Color lightContrib = CalculateBlinnPhong(vertexPos, normal, viewDir, lightDir, 
                                                light->color, light->intensity * attenuation, 
                                                baseColor, config->specularStrength, config->shininess);
        
        totalR += lightContrib.r;
        totalG += lightContrib.g;
        totalB += lightContrib.b;
    }
    
    // Clamp values
    Color result;
    result.r = (unsigned char)fminf(255, totalR);
    result.g = (unsigned char)fminf(255, totalG);
    result.b = (unsigned char)fminf(255, totalB);
    result.a = baseColor.a;
    
    return result;
}

// Calculate simple lighting for backward compatibility
Color CalculateSimpleLighting(Vector3 vertexPos, Vector3 normal, Color baseColor)
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

// Draw all light sources in the scene
void DrawLights(const LightingSystem* lighting)
{
    for (int i = 0; i < lighting->lightCount; i++) {
        const Light* light = &lighting->lights[i];
        if (!light->enabled) continue;
        
        switch (light->type) {
            case LIGHT_DIRECTIONAL:
                // Draw an arrow or symbol to represent directional light
                DrawSphere(Vector3Add(light->position, Vector3Scale(light->direction, -50.0f)), 3.0f, light->color);
                break;
                
            case LIGHT_POINT:
                // Draw a sphere for point lights
                DrawSphere(light->position, 2.0f, light->color);
                DrawSphereWires(light->position, light->range * 0.1f, 8, 8, ColorAlpha(light->color, 0.3f));
                break;
                
            case LIGHT_SPOT:
                // Draw a cone for spot lights
                DrawSphere(light->position, 2.0f, light->color);
                // Simple cone representation
                Vector3 coneEnd = Vector3Add(light->position, Vector3Scale(light->direction, 10.0f));
                DrawLine3D(light->position, coneEnd, light->color);
                break;
        }
    }
}

// Draw sun/light source in the scene (legacy)
void DrawSunLight(void)
{
    DrawSphere((Vector3){ SUN_POSITION_X, SUN_POSITION_Y, SUN_POSITION_Z }, 5.0f, YELLOW);
}