#version 410

out float FragColor;

uniform sampler3D VelocityTexture;
uniform vec3 InverseSize;

in float layer;

void main() {
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    vec3 vL = texture(VelocityTexture, (fragCoord - vec3(1, 0, 0)) * InverseSize).xyz;
    vec3 vR = texture(VelocityTexture, (fragCoord + vec3(1, 0, 0)) * InverseSize).xyz;
    vec3 vB = texture(VelocityTexture, (fragCoord - vec3(0, 1, 0)) * InverseSize).xyz;
    vec3 vT = texture(VelocityTexture, (fragCoord + vec3(0, 1, 0)) * InverseSize).xyz;
    vec3 vU = texture(VelocityTexture, (fragCoord - vec3(0, 0, 1)) * InverseSize).xyz;
    vec3 vD = texture(VelocityTexture, (fragCoord + vec3(0, 0, 1)) * InverseSize).xyz;

    FragColor = 0.5f * (vR.x - vL.x + vT.y - vB.y + vU.z - vD.z);

    // FragColor = abs(vR.x - vL.x) * 100;
    // FragColor = 1.0f;
}