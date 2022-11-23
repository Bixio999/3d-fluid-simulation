#version 410 core

layout (location = 0) in vec3 position;

uniform float grid_depth;
uniform float scaling_factor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out int vInstance;

void main()
{
    vInstance = gl_InstanceID;

    float nearPlane = 2 * scaling_factor * ((gl_InstanceID + 1) / grid_depth) + 1.0;
    float farPlane = 100;

    float newProj1 = -2.0 / (farPlane - nearPlane);
    float newProj2 = -(farPlane + nearPlane) / (farPlane - nearPlane);

    mat4 newProj = mat4(projection);

    newProj[2][2] = newProj1;
    newProj[3][2] = newProj2;

    gl_Position = newProj * view * model * vec4(position, 1.0);
}