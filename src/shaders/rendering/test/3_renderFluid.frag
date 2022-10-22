#version 410 core

// in float gLayer;

// uniform sampler3D velocityTexture;

uniform vec3 InverseSize;

out vec4 FragColor;

float mod(float a, float b)
{
    return a - (b * floor(a/b));
}

void main()
{
    FragColor = vec4(1,0,0,1);
    // vec3 pos = gl_FragCoord.xyz * InverseSize;
    // vec3 color = texture(velocityTexture, pos).xyz;
    // color = mod(pos.x, 2.0f) == 0 ? color : 1.0 - color;
    // FragColor = vec4((color * vec3(1,0,0)), 1.0);
}

