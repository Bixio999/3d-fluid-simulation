#version 410 core

out vec4 FragColor;

uniform sampler3D VelocityTexture;
uniform sampler3D DensityTexture;
uniform sampler3D TemperatureTexture;

uniform float ambientTemperature;
uniform float timeStep;
uniform float smokeBuoyancy;
uniform float smokeWeight;

uniform vec3 InverseSize;

in float layer;

void main()
{
    vec3 FragCoord = vec3(gl_FragCoord.xy, layer);

    vec3 currentVel = texture(VelocityTexture, FragCoord * InverseSize).xyz; 
    float temp = texture(TemperatureTexture, FragCoord * InverseSize).x;

    if (temp > ambientTemperature)
    {
        float dens = texture(DensityTexture, FragCoord * InverseSize).x;
        float buoyancy = (temp - ambientTemperature) * smokeBuoyancy * timeStep - dens * smokeWeight;
        vec3 bForce = vec3(0, 1, 0) * buoyancy;

        FragColor = vec4(currentVel + bForce, 1.0);
    }
    else
    {
        FragColor = vec4(currentVel, 1.0);
    }

    if (length(FragColor) < 0.0001)
        FragColor = vec4(0.0);
}