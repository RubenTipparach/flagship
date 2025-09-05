#version 300 es

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;

out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec3 barycentric;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = vertexNormal;
    
    // Create barycentric coordinates using gl_VertexID (available in ES 3.0)
    int vertexId = gl_VertexID % 3;
    if (vertexId == 0)
        barycentric = vec3(1.0, 0.0, 0.0);
    else if (vertexId == 1)
        barycentric = vec3(0.0, 1.0, 0.0);
    else
        barycentric = vec3(0.0, 0.0, 1.0);
    
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}