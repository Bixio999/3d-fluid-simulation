#version 410

out float FragColor;

uniform sampler3D VelocityTexture;
uniform sampler3D ObstacleTexture;
uniform sampler3D ObstacleVelocityTexture;
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

    vec3 obsL = texture(ObstacleTexture, InverseSize * (fragCoord - vec3(1, 0, 0))).xyz;
    vec3 obsR = texture(ObstacleTexture, InverseSize * (fragCoord + vec3(1, 0, 0))).xyz;
    vec3 obsB = texture(ObstacleTexture, InverseSize * (fragCoord - vec3(0, 1, 0))).xyz;
    vec3 obsT = texture(ObstacleTexture, InverseSize * (fragCoord + vec3(0, 1, 0))).xyz;
    vec3 obsU = texture(ObstacleTexture, InverseSize * (fragCoord - vec3(0, 0, 1))).xyz;
    vec3 obsD = texture(ObstacleTexture, InverseSize * (fragCoord + vec3(0, 0, 1))).xyz;

    vec3 obsVelL = texture(ObstacleVelocityTexture, InverseSize * (fragCoord - vec3(1, 0, 0))).xyz;
    vec3 obsVelR = texture(ObstacleVelocityTexture, InverseSize * (fragCoord + vec3(1, 0, 0))).xyz;
    vec3 obsVelB = texture(ObstacleVelocityTexture, InverseSize * (fragCoord - vec3(0, 1, 0))).xyz;
    vec3 obsVelT = texture(ObstacleVelocityTexture, InverseSize * (fragCoord + vec3(0, 1, 0))).xyz;
    vec3 obsVelU = texture(ObstacleVelocityTexture, InverseSize * (fragCoord - vec3(0, 0, 1))).xyz;
    vec3 obsVelD = texture(ObstacleVelocityTexture, InverseSize * (fragCoord + vec3(0, 0, 1))).xyz;


    // if (obsL.x > 0.0) vL = obsVelL.yzx;
    // if (obsR.x > 0.0) vR = obsVelR.yzx;
    // if (obsB.x > 0.0) vB = obsVelB.yzx;
    // if (obsT.x > 0.0) vT = obsVelT.yzx;
    // if (obsU.x > 0.0) vU = obsVelU.yzx;
    // if (obsD.x > 0.0) vD = obsVelD.yzx;

    if (obsL.x > 0.0) vL = obsVelL.xyz;
    if (obsR.x > 0.0) vR = obsVelR.xyz;
    if (obsB.x > 0.0) vB = obsVelB.xyz;
    if (obsT.x > 0.0) vT = obsVelT.xyz;
    if (obsU.x > 0.0) vU = obsVelU.xyz;
    if (obsD.x > 0.0) vD = obsVelD.xyz;
    
    // if (obsL.x > 0.0) vL = vec3(0.0);
    // if (obsR.x > 0.0) vR = vec3(0.0);
    // if (obsB.x > 0.0) vB = vec3(0.0);
    // if (obsT.x > 0.0) vT = vec3(0.0);
    // if (obsU.x > 0.0) vU = vec3(0.0);
    // if (obsD.x > 0.0) vD = vec3(0.0);

    FragColor = 0.5 * (vR.x - vL.x + vT.y - vB.y + vD.z - vU.z);

    if (abs(FragColor) < 0.0001)
        FragColor = 0.0;
}