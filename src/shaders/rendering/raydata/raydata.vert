#version 410 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 mvpPos;
out vec3 ogPos;

void main()
{
    vec4 pos = projection * view * model * vec4(aPos, 1.0);
    mvpPos = pos;
    ogPos = (aPos + 1) / 2.0f;
    gl_Position = pos;
}