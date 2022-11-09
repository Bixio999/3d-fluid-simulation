#version 410 core

out vec4 fragColor;

uniform vec3 grid_size;

uniform sampler2D RayDataTexture;

in vec4 mvpPos;
in vec3 texPos;

uniform vec2 InverseSize;

vec3 TextureVoxelClamp(vec3 pos)
{
    pos *= (grid_size - 1.0);
    pos += 0.5;
    pos /= grid_size;
    return pos;
}

void main()
{
    // vec3 pos = ogPos * (grid_size - 1.0);
    // pos += 0.5;
    // pos /= grid_size;

    // float depth = gl_FragCoord.z * (grid_size.z - 1.0);
    // depth += 0.5;
    // depth /= grid_size.z;

    vec4 raydata = texture(RayDataTexture, InverseSize * gl_FragCoord.xy);

    fragColor = vec4(TextureVoxelClamp(texPos), raydata.w);
}