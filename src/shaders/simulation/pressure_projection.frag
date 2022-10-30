#version 410 core

out vec3 newVelocity;

uniform sampler3D VelocityTexture;
uniform sampler3D PressureTexture;
uniform vec3 InverseSize;

in float layer;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    float pLeft = texture(PressureTexture, (fragCoord + vec3(-1, 0, 0)) * InverseSize).x;
    float pRight = texture(PressureTexture, (fragCoord + vec3(1, 0, 0)) * InverseSize).x;
    float pBack = texture(PressureTexture, (fragCoord + vec3(0, -1, 0)) * InverseSize).x;
    float pFront = texture(PressureTexture, (fragCoord + vec3(0, 1, 0)) * InverseSize).x;
    float pBottom = texture(PressureTexture, (fragCoord + vec3(0, 0, -1)) * InverseSize).x;
    float pTop = texture(PressureTexture, (fragCoord + vec3(0, 0, 1)) * InverseSize).x;

    vec3 gradient = 0.5f * vec3(pRight - pLeft, pFront - pBack, pTop - pBottom);

    vec3 oldVelocity = texture(VelocityTexture, fragCoord * InverseSize).xyz;

    newVelocity = oldVelocity - gradient;

    if (length(newVelocity) < 0.0001)
        newVelocity = vec3(0.0);
}