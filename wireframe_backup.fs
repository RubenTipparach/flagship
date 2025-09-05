#version 100

precision mediump float;

// Input from vertex shader
varying vec3 fragPosition;      // fragment position in world space
varying vec2 fragTexCoord;      // fragment texture coordinates
varying vec4 fragColor;         // fragment color
varying vec3 fragNormal;        // fragment normal in world space

// Input uniform values
uniform sampler2D texture0;        // base texture
uniform vec4 colDiffuse;           // diffuse color
uniform int enableWireframe;       // wireframe toggle (using int instead of bool)
uniform vec4 wireframeColor;       // wireframe line color
uniform float wireframeThickness;  // wireframe line thickness

void main()
{
    // Start with base vertex color
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    vec4 baseColor = fragColor * colDiffuse * texelColor;
    
    if (enableWireframe == 1) {
        // Create wireframe lines based on texture coordinates (simpler approach for GLES)
        vec2 coord = fragTexCoord * 50.0; // Scale for wireframe density
        vec2 grid = abs(fract(coord - 0.5) - 0.5);
        float line = min(grid.x, grid.y);
        float wireframeFactor = 1.0 - smoothstep(0.0, wireframeThickness * 0.02, line);
        
        // Mix base color with wireframe color
        gl_FragColor = mix(baseColor, wireframeColor, wireframeFactor);
    } else {
        // Just use the base color without wireframe
        gl_FragColor = baseColor;
    }
    
    // Apply basic lighting (simple diffuse)
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float NdotL = max(dot(normalize(fragNormal), lightDir), 0.0);
    gl_FragColor.rgb *= (0.3 + 0.7 * NdotL); // ambient + diffuse
}