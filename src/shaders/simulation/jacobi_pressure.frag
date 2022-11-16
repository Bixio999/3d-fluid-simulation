#version 410 core

out float newPressure;

uniform sampler3D Divergence;
uniform sampler3D Pressure;
uniform sampler3D Obstacle;
uniform vec3 InverseSize;

in float layer;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    float divergence = texture(Divergence, fragCoord * InverseSize).r;
    float pressure = texture(Pressure, fragCoord * InverseSize).r;

    float pLeft = texture(Pressure, (fragCoord + vec3(-1.0, 0.0, 0.0)) * InverseSize).r;
    float pRight = texture(Pressure, (fragCoord + vec3(1.0, 0.0, 0.0)) * InverseSize).r;
    float pDown = texture(Pressure, (fragCoord + vec3(0.0, -1.0, 0.0)) * InverseSize).r;
    float pUp = texture(Pressure, (fragCoord + vec3(0.0, 1.0, 0.0)) * InverseSize).r;
    float pBottom = texture(Pressure, (fragCoord + vec3(0.0, 0.0, -1.0)) * InverseSize).r;
    float pTop = texture(Pressure, (fragCoord + vec3(0.0, 0.0, 1.0)) * InverseSize).r;

    float obsLeft = texture(Obstacle, (fragCoord + vec3(-1.0, 0.0, 0.0)) * InverseSize).r;
    float obsRight = texture(Obstacle, (fragCoord + vec3(1.0, 0.0, 0.0)) * InverseSize).r;
    float obsDown = texture(Obstacle, (fragCoord + vec3(0.0, -1.0, 0.0)) * InverseSize).r;
    float obsUp = texture(Obstacle, (fragCoord + vec3(0.0, 1.0, 0.0)) * InverseSize).r;
    float obsBottom = texture(Obstacle, (fragCoord + vec3(0.0, 0.0, -1.0)) * InverseSize).r;
    float obsTop = texture(Obstacle, (fragCoord + vec3(0.0, 0.0, 1.0)) * InverseSize).r;

    if (obsLeft > 0.0) pLeft = pressure;
    if (obsRight > 0.0) pRight = pressure;
    if (obsDown > 0.0) pDown = pressure;
    if (obsUp > 0.0) pUp = pressure;
    if (obsBottom > 0.0) pBottom = pressure;
    if (obsTop > 0.0) pTop = pressure;

    newPressure = (pLeft + pRight + pDown + pUp + pBottom + pTop - divergence) / 6.0f;

    if (abs(newPressure) < 0.0001)
        newPressure = 0.0;
}