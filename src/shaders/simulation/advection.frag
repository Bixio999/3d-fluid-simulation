
#version 410 core

out vec4 FragColor;

uniform sampler3D VelocityTexture;
uniform sampler3D SourceTexture;
uniform sampler3D ObstacleTexture;

uniform float timeStep;
uniform vec3 InverseSize;
uniform float dissipation;

in float layer;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    float obstacle = texture(ObstacleTexture, InverseSize * fragCoord).r;

    vec4 finalColor = vec4(0.0);

    if (obstacle < 1.0)
    {
        vec3 u = texture(VelocityTexture, InverseSize * fragCoord).xyz;
        vec3 coord = InverseSize * (fragCoord - timeStep * u);

        float obs = texture(ObstacleTexture, coord).r;
        if (obs < 1.0)
            finalColor = dissipation * texture(SourceTexture, coord);
        else
            finalColor = dissipation * vec4(u, 1.0);

        if (length(finalColor) < 0.0001)
            finalColor = vec4(0.0);
    }

    FragColor = finalColor;
}