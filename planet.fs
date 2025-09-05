#version 300 es

precision mediump float;

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec3 barycentric;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float wireframeMode;

out vec4 finalColor;

void main()
{
    // Base color from vertex colors and texture
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    vec4 baseColor = fragColor * colDiffuse * texelColor;
    
    if (wireframeMode > 0.5) {
        // True wireframe using barycentric coordinates
        // Find the minimum distance to any triangle edge
        float minDist = min(min(barycentric.x, barycentric.y), barycentric.z);
        //float minDist = min(barycentric.x, barycentric.y);
        float pix = max(fwidth(minDist), 1e-6);
        minDist = minDist/ pix;

        // Line width for wireframe edges (adjust for thickness)
        float lineWidth = 0.1;
        float wireframe = 1.0 - smoothstep(0.1- 0.5, lineWidth + 0.5, minDist);
        
        // Only render edges, discard interior pixels
        if (wireframe > 0.5) {
            finalColor = vec4(0.0, 1.0, 1.0, 1.0); // Bright cyan wireframe edges
        } else {
            discard; // Make interior faces transparent
            //finalColor = vec4(0.0, 1.0, 1.0, 0.1); // Bright cyan wireframe edges

        }
    } else {
        // Normal mode: just use base colors
        //finalColor = baseColor;
    }
    
    // Simple lighting
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(normalize(fragNormal), lightDir), 0.0);
    finalColor.rgb *= (0.3 + 0.7 * NdotL);
}