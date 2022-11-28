#version 410 core

out vec4 FragColor;

in float layer;
in vec3 velocity;

uniform sampler3D ObstacleVelocity;
uniform vec3 InverseSize;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    vec3 currVelocity = texture(ObstacleVelocity, fragCoord * InverseSize).xyz;

    FragColor = vec4(currVelocity + velocity, 1.0);
}