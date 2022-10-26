#version 410 core

out vec4 FragColor;

uniform vec2 InverseSize;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 grid_size;

uniform sampler2D RayDataTexture;
uniform sampler3D DensityTexture;

in vec4 mvpPos;
in vec3 ogPos;

void main()
{
    vec3 end = texture(RayDataTexture, InverseSize * gl_FragCoord.xy).xyz;

    vec3 fluidColor = vec3(1,1, 1);
    vec4 finalColor = vec4(0,0,0,0);

    vec3 start = ogPos * (grid_size - 1.0);
    start += 0.5;
    start /= grid_size;

    vec3 dir = end - start;

    // FragColor = vec4(dir, 1);

    float t = 0.5 / grid_size.x;

    float sampledDensity = 0;
    for(float i = 0; i <= 1; i += t)
    {
        vec3 p = start + dir * i;

        // vec3 v = texture(DensityTexture, p).xyz;
        // sampledDensity = length(v);

        // v = normalize(v) * 255;

        // finalColor.xyz = v;

        // sampledDensity = length(texture(DensityTexture, p).xyz);
        sampledDensity = texture(DensityTexture, p).x;
        finalColor.xyz += fluidColor * sampledDensity * (1.0 - finalColor.w);
        finalColor.w += sampledDensity * (1.0 - finalColor.w);

        if (finalColor.w > 0.99)
            break;
    }

    FragColor = finalColor;

}