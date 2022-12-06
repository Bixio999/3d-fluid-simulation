#version 410 core

out vec4 FragColor;

uniform sampler3D VelocityTexture;
uniform sampler3D LevelSetTexture;
uniform vec3 InverseSize;

uniform float timeStep;
uniform float gravityAcceleration;
uniform float levelSetThreshold;

in float layer;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    vec3 velocity = texture(VelocityTexture, fragCoord * InverseSize).xyz;
    float levelSet = texture(LevelSetTexture, fragCoord * InverseSize).r;

    if (levelSet < levelSetThreshold)
        velocity.y -= gravityAcceleration * timeStep;

    FragColor = vec4(velocity, 0.0);
}