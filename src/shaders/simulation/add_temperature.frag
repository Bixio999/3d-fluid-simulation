#version 410 core

out vec4 FragColor;

uniform float timeStep;
uniform vec3 InverseSize;

uniform sampler3D TemperatureTexture;
uniform float temperature;
uniform vec3 center;
uniform float radius;

in float layer;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    float dx = (fragCoord.x - center.x);
    float dy = (fragCoord.y - center.y);
    float dz = (fragCoord.z - center.z);

    dx = dx * dx;
    dy = dy * dy;
    dz = dz * dz;

    // FragColor = vec4(dx, dy, dz, 1.0f);
    // FragColor = vec4(0,1,0,1);

    float gaussianSplat = exp(-1 * (dx + dy + dz) / (radius * radius * 2.0));
    // float gaussianSplat = exp(-1 * (dx + dy + dz) / (radius * radius));
    // FragColor = vec4(gaussianSplat, 0, 0, 1.0f);

    float appliedTemp = temperature * gaussianSplat; 
    float finalTemp = appliedTemp + texture(TemperatureTexture, fragCoord * InverseSize).x;

    // finalVel = min(length(force), length(finalVel)) * normalize(finalVel);

    // vec3 finalVel = vForce;

    FragColor = vec4(finalTemp, 0, 0, 1.0);
    // FragColor = vec3(1,0,0);

    if (length(FragColor) < 0.0001)
        FragColor = vec4(0.0);
}