#version 410 core

layout (location = 0) in vec3 position;

uniform float grid_depth;

uniform vec3 firstLayerPoint;
uniform vec3 layersDir;

uniform float deltaTime;

uniform mat4 prevModel;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out int vInstance;

out vec3 planePoint;
out vec3 planeNormal;

out vec3 vertVelocity;

out vec3 mPos;

void main()
{
    vInstance = gl_InstanceID;

    planePoint = firstLayerPoint + layersDir * ((gl_InstanceID + 1) / grid_depth);
    planeNormal = normalize(layersDir);

    vec3 oldPos = (projection * view * prevModel * vec4(position, 1.0)).xyz;

    vec4 newPos = model * vec4(position, 1.0);
    mPos = newPos.xyz;
    newPos = projection * view * newPos;

    vertVelocity = (newPos.xyz - oldPos) / deltaTime;

    gl_Position = newPos;
}