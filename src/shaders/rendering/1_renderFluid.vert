#version 410 core

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 pos;

// out int vInstance;

void main()
{
    // vec3 pos = position + vec3(1, gl_InstanceID, 0);
    gl_Position = projection * view * model * vec4(position, 1.0f);
    // gl_Position = vec4(pos, 1.0f);
    // vInstance = gl_InstanceID;

    pos = vec4(position, 1.0f);
}
