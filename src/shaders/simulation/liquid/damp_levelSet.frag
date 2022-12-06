#version 410 core

out vec4 FragColor;

uniform sampler3D LevelSetTexture;
uniform sampler3D ObstacleTexture;
uniform vec3 InverseSize;

uniform float dampingFactor;
uniform float equilibriumHeight;
uniform float grid_height;

in float layer;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    float obstacle = texture(ObstacleTexture, fragCoord * InverseSize).r;
    float newLevelSet = 0.0;

    if (obstacle <= 0.0)
    {
        float currLevelSet = texture(LevelSetTexture, fragCoord * InverseSize).r;
        float equilibriumLevelSet = fragCoord.y - grid_height * equilibriumHeight;

        if (equilibriumLevelSet < 0.0)
            newLevelSet = (1 - dampingFactor) * currLevelSet + dampingFactor * (equilibriumLevelSet);
        else
            newLevelSet = currLevelSet;

        if (abs(newLevelSet) < 0.0001)
            newLevelSet = 0.0;
    }

    FragColor = vec4(newLevelSet, 0.0, 0.0, 1.0);
}