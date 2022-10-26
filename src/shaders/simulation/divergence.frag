#version 410

out float FragColor;

uniform sampler3D VelocityTexture;
uniform vec3 InverseSize;

in float layer;

void main() {
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    vec3 vL = texture(VelocityTexture, InverseSize * (fragCoord - vec3(1, 0, 0))).xyz;
    vec3 vR = texture(VelocityTexture, InverseSize * (fragCoord + vec3(1, 0, 0))).xyz;
    vec3 vB = texture(VelocityTexture, InverseSize * (fragCoord - vec3(0, 1, 0))).xyz;
    vec3 vT = texture(VelocityTexture, InverseSize * (fragCoord + vec3(0, 1, 0))).xyz;
    vec3 vU = texture(VelocityTexture, InverseSize * (fragCoord - vec3(0, 0, 1))).xyz;
    vec3 vD = texture(VelocityTexture, InverseSize * (fragCoord + vec3(0, 0, 1))).xyz;

    // float coefficient = 0.5 / (1 / InverseSize.x);

    // FragColor = coefficient * (vR.x - vL.x + vT.y - vB.y + vU.z - vD.z);
    FragColor = 0.5 * (vR.x - vL.x + vT.y - vB.y + vU.z - vD.z);

    // FragColor = abs(vR.x - vL.x) * 100;
    // FragColor = 1.0f;
}