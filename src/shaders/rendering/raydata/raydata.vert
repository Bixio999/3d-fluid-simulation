#version 410 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightVector;

out vec4 mvpPos;
out vec3 texPos;
out vec3 ogPos;

out vec3 lightDir;
out vec3 vViewPosition;

void main()
{
    vec4 pos = view * model * vec4(aPos, 1.0);
    vViewPosition = -pos.xyz;
    pos = projection * pos;

    mvpPos = pos;
    texPos = (aPos + 1) / 2.0;

    texPos.z = 1 - texPos.z;

    texPos = clamp(texPos, 0, 1);

    ogPos = aPos;
    lightDir = vec3(view * vec4(lightVector, 0.0));
    gl_Position = pos;
}