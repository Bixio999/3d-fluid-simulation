#version 410 core

out float outColor;

uniform float timeStep;
uniform vec3 InverseSize;

uniform sampler3D DensityTexture;
uniform float dyeIntensity;
uniform vec3 center;
uniform float radius;

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

    float gaussianSplat = exp(-1 * (dx + dy + dz) / (radius));
    // float gaussianSplat = exp(-1 * (dx + dy + dz) / (radius * radius * 2));
    // FragColor = vec4(gaussianSplat, 0, 0, 1.0f);

    // outColor = length(force);

    float newDye = dyeIntensity * gaussianSplat; 
    // float newDye = dyeIntensity * timeStep * gaussianSplat; 
    float finalDensity = newDye + texture(DensityTexture, InverseSize * fragCoord).x;

    outColor = finalDensity;
}