#version 100

precision mediump float;

// Input from vertex shader
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int enableWireframe;

void main()
{
    // Completely ignore everything else and just make it red or green based on enableWireframe
    if (enableWireframe == 1) {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Bright red
    } else {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); // Bright green
    }
}