/*
    OpenGL 4.1 Core - Vertex Shader

    This shader is used to transform the vertices of the model
    to the screen space. It is used in the vertex shader stage.
*/

#version 410 core

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
}
