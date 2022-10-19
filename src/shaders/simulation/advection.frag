
#version 410 core

out vec4 FragColor;

uniform sampler3D VelocityTexture;

uniform float timeStep;
uniform vec3 InverseSize;

in float layer;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    vec3 u = texture(VelocityTexture, InverseSize * fragCoord).xyz;

    vec3 coord = InverseSize * (fragCoord - timeStep * u);

    FragColor = texture(VelocityTexture, coord);
    // FragColor = vec4(coord, 1.0);
}