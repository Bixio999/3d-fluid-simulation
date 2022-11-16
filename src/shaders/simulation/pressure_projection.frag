#version 410 core

out vec3 newVelocity;

uniform sampler3D VelocityTexture;
uniform sampler3D PressureTexture;
uniform sampler3D ObstacleTexture;
uniform vec3 InverseSize;

in float layer;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    float obsCenter = texture(ObstacleTexture, fragCoord * InverseSize).r;

    if (obsCenter > 0.0)
        newVelocity = vec3(0.0);
    else
    {
        float pCenter = texture(PressureTexture, fragCoord * InverseSize).r;

        float pLeft = texture(PressureTexture, (fragCoord + vec3(-1, 0, 0)) * InverseSize).x;
        float pRight = texture(PressureTexture, (fragCoord + vec3(1, 0, 0)) * InverseSize).x;
        float pBack = texture(PressureTexture, (fragCoord + vec3(0, -1, 0)) * InverseSize).x;
        float pFront = texture(PressureTexture, (fragCoord + vec3(0, 1, 0)) * InverseSize).x;
        float pBottom = texture(PressureTexture, (fragCoord + vec3(0, 0, -1)) * InverseSize).x;
        float pTop = texture(PressureTexture, (fragCoord + vec3(0, 0, 1)) * InverseSize).x;

        vec3 obsLeft = texture(ObstacleTexture, (fragCoord + vec3(-1, 0, 0)) * InverseSize).rgb;
        vec3 obsRight = texture(ObstacleTexture, (fragCoord + vec3(1, 0, 0)) * InverseSize).rgb;
        vec3 obsBack = texture(ObstacleTexture, (fragCoord + vec3(0, -1, 0)) * InverseSize).rgb;
        vec3 obsFront = texture(ObstacleTexture, (fragCoord + vec3(0, 1, 0)) * InverseSize).rgb;
        vec3 obsBottom = texture(ObstacleTexture, (fragCoord + vec3(0, 0, -1)) * InverseSize).rgb;
        vec3 obsTop = texture(ObstacleTexture, (fragCoord + vec3(0, 0, 1)) * InverseSize).rgb;

        vec3 obsVelocity = vec3(0.0);
        vec3 obsMask = vec3(1.0);

        if (obsLeft.r > 0.0)
        {
            pLeft = pCenter;
            obsVelocity.x = obsLeft.x;
            obsMask.x = 0.0;
        }
        if (obsRight.r > 0.0)
        {
            pRight = pCenter;
            obsVelocity.x = obsRight.x;
            obsMask.x = 0.0;
        }
        if (obsBack.r > 0.0)
        {
            pBack = pCenter;
            obsVelocity.y = obsBack.y;
            obsMask.y = 0.0;
        }
        if (obsFront.r > 0.0)
        {
            pFront = pCenter;
            obsVelocity.y = obsFront.y;
            obsMask.y = 0.0;
        }
        if (obsBottom.r > 0.0)
        {
            pBottom = pCenter;
            obsVelocity.z = obsBottom.z;
            obsMask.z = 0.0;
        }
        if (obsTop.r > 0.0)
        {
            pTop = pCenter;
            obsVelocity.z = obsTop.z;
            obsMask.z = 0.0;
        }

        vec3 gradient = 0.5f * vec3(pRight - pLeft, pFront - pBack, pTop - pBottom);

        vec3 oldVelocity = texture(VelocityTexture, fragCoord * InverseSize).xyz;

        newVelocity = oldVelocity - gradient;
        newVelocity = (obsMask * newVelocity) + obsVelocity;

        if (length(newVelocity) < 0.0001)
            newVelocity = vec3(0.0);
    }

}