#version 410 core

out float outColor;

uniform float timeStep;
uniform vec3 InverseSize;

uniform sampler3D DensityTexture;
uniform float dyeIntensity;
uniform vec3 center;
uniform float radius;

uniform bool isLiquidSimulation;

in float layer;

void main() {
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    float dx = (fragCoord.x - center.x);
    float dy = (fragCoord.y - center.y);
    float dz = (fragCoord.z - center.z);

    dx = dx * dx;
    dy = dy * dy;
    dz = dz * dz;

    // FragColor = vec4(dx, dy, dz, 1.0f);
    // FragColor = vec4(0,1,0,1);

    float gaussianSplat = exp(-1 * (dx + dy + dz) / (radius * radius * 2));
    // float gaussianSplat = exp(-1 * (dx + dy + dz) / (radius * radius * 2));
    // FragColor = vec4(gaussianSplat, 0, 0, 1.0f);

    // outColor = length(force);

    gaussianSplat = clamp(gaussianSplat, 0.0, 1.0);

    float currDensity = texture(DensityTexture, fragCoord * InverseSize).r;
    float finalDensity = currDensity;

    if (isLiquidSimulation)
    {
        if (gaussianSplat > 0.01) finalDensity = gaussianSplat * dyeIntensity;
    }
    else
    {
        finalDensity += gaussianSplat * dyeIntensity;
        finalDensity = clamp(finalDensity, 0.0, 1.0);
    }

    outColor = finalDensity;
}