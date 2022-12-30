/*
    OpenGL 4.1 Core - Vertex Shader

    This shader is used to render the instanced quad.

    The vertex shader is responsible for setting the position of the vertex
    and passing the instance ID to the geometry shader, which will enable the
    layered rendering.
*/

#version 410 core

in vec4 position;

out int vInstance;

void main()
{
    gl_Position = position;
    vInstance = gl_InstanceID;
}