
#version 410 core

// layout (location = 0) in vec3 position;
in vec4 position;

out int vInstance;

void main()
{
    gl_Position = position;
    vInstance = gl_InstanceID;
}