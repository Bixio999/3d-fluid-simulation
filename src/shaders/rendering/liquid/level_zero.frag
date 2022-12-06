#version 410 core

out vec4 FragColor;

uniform sampler3D LevelSetTexture;
uniform vec3 InverseSize;

in float layer;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.x, layer, gl_FragCoord.y);

    float levelSet = texture(LevelSetTexture, fragCoord * InverseSize).r;

    if (levelSet != 0.0)
        discard;

    gl_FragDepth = gl_FragCoord.y;
    FragColor = vec4(fragCoord, 1.0);
}