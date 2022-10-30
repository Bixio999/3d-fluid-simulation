
#version 410 core

out vec4 FragColor;

uniform sampler3D VelocityTexture;
uniform sampler3D SourceTexture;

uniform float timeStep;
uniform vec3 InverseSize;
uniform float dissipation;

in float layer;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    vec3 u = texture(VelocityTexture, InverseSize * fragCoord).xyz;

    vec3 coord = InverseSize * (fragCoord - timeStep * u);

    // FragColor = texture(SourceTexture, coord);
    FragColor = dissipation * texture(SourceTexture, coord);

    if (length(FragColor) < 0.0001)
        FragColor = vec4(0.0);

    // float decay = 1.0 + dissipation * timeStep;
    // FragColor = texture(SourceTexture, coord) / decay;
    // FragColor = vec4(coord, 1.0);
    // FragColor = vec4 (1,0,0,1);
}