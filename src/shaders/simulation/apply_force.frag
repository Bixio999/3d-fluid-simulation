#version 410 core

out vec4 FragColor;

uniform float timeStep;
uniform vec3 InverseSize;

uniform sampler3D VelocityTexture;
uniform vec3 force;
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

    float gaussianSplat = exp(-1 * (dx + dy + dz) / (radius));
    // float gaussianSplat = exp(-1 * (dx + dy + dz) / (radius * radius));
    // FragColor = vec4(gaussianSplat, 0, 0, 1.0f);

    vec3 vForce = force * gaussianSplat; 
    vec3 finalVel = vForce + texture(VelocityTexture, fragCoord * InverseSize).xyz;

    // finalVel = min(length(force), length(finalVel)) * normalize(finalVel);

    // vec3 finalVel = vForce;

    FragColor = vec4(finalVel, 1.0f);
    // FragColor = vec3(1,0,0);
}