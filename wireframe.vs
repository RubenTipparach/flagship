#version 100

// Input vertex attributes (from vertex buffer)
attribute vec3 vertexPosition;    // vertex position in local space
attribute vec2 vertexTexCoord;    // vertex texture coordinates
attribute vec3 vertexNormal;      // vertex normal
attribute vec4 vertexColor;       // vertex color

// Input uniform values
uniform mat4 mvp;          // model-view-projection matrix
uniform mat4 matModel;     // model matrix
uniform mat4 matNormal;    // normal matrix

// Output values to fragment shader
varying vec3 fragPosition;     // vertex position in world space  
varying vec2 fragTexCoord;     // vertex texture coordinates
varying vec4 fragColor;        // vertex color
varying vec3 fragNormal;       // vertex normal in world space

void main()
{
    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));
    
    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}