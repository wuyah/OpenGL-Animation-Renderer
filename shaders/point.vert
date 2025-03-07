#version 330 core

// Rendering Guiding Point of IK System
uniform mat4 viewProj;
uniform vec3 cameraPos;     // Make point interact with the camera.
uniform mat4 model;

layout (location = 0) in vec3 aPos;

void main()
{
    float distance_ = length(cameraPos - aPos);
    float size = clamp(100.0 / distance_, 2.0, 15.0);   
    gl_PointSize = floor(size);
    
    // Position
    gl_Position = viewProj * vec4(aPos, 1.0);
}

