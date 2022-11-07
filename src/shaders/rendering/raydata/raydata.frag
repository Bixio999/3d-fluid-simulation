#version 410 core

out vec4 fragColor;

uniform vec3 grid_size;

in vec4 mvpPos;
in vec3 texPos;

void main()
{
    // vec3 pos = ogPos * (grid_size - 1.0);
    // pos += 0.5;
    // pos /= grid_size;

    // float depth = gl_FragCoord.z * (grid_size.z - 1.0);
    // depth += 0.5;
    // depth /= grid_size.z;

    fragColor = vec4(texPos, -1.0);
}