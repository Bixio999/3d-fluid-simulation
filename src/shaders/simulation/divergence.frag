/*
    OpenGL 4.1 Core - Fluid Simulation: Divergence - Fragment Shader

    This shader is used to calculate the divergence of the velocity field.
    The divergence is used to calculate the pressure field. The divergence
    is calculated by taking the difference between the velocity in each
    direction and dividing by the cell size. The result is then summed
    together to give the divergence.

    To handle the interaction with obstacles, the shader considers the
    neighbouring cells of the obstacle texture. If a neighbouring cell
    is an obstacle, the velocity of the obstacle is used instead of the
    velocity of the neighbouring cell. This is done by checking the
    obstacle texture and using the obstacle velocity texture if the
    neighbouring cell is an obstacle.

    The Divergence program is composed by the following shaders:
    - Vertex Shader:   load_vertices.vert - load the vertices of the quad
    - Geometry Shader: set_layer.geom - set the layer of the quad and enable
      the layered rendering
    - Fragment Shader: this shader
*/

#version 410 core

out float FragColor; // Output data

// Input texture samplers
uniform sampler3D VelocityTexture;
uniform sampler3D ObstacleTexture;
uniform sampler3D ObstacleVelocityTexture;

uniform vec3 InverseSize; // Inverse size of the simulation grid

in float layer; // Layer of the 3D texture

// Main function
void main() 
{
    // Calculate the fragment coordinates in the 3D texture
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    // Sample the velocity texture to get the velocity for each neighbour cell
    vec3 vL = texture(VelocityTexture, InverseSize * (fragCoord - vec3(1, 0, 0))).xyz;
    vec3 vR = texture(VelocityTexture, InverseSize * (fragCoord + vec3(1, 0, 0))).xyz;
    vec3 vB = texture(VelocityTexture, InverseSize * (fragCoord - vec3(0, 1, 0))).xyz;
    vec3 vT = texture(VelocityTexture, InverseSize * (fragCoord + vec3(0, 1, 0))).xyz;
    vec3 vU = texture(VelocityTexture, InverseSize * (fragCoord - vec3(0, 0, 1))).xyz;
    vec3 vD = texture(VelocityTexture, InverseSize * (fragCoord + vec3(0, 0, 1))).xyz;

    // Sample the obstacle texture to get the obstacle for each neighbour cell
    float obsL = texture(ObstacleTexture, InverseSize * (fragCoord - vec3(1, 0, 0))).x;
    float obsR = texture(ObstacleTexture, InverseSize * (fragCoord + vec3(1, 0, 0))).x;
    float obsB = texture(ObstacleTexture, InverseSize * (fragCoord - vec3(0, 1, 0))).x;
    float obsT = texture(ObstacleTexture, InverseSize * (fragCoord + vec3(0, 1, 0))).x;
    float obsU = texture(ObstacleTexture, InverseSize * (fragCoord - vec3(0, 0, 1))).x;
    float obsD = texture(ObstacleTexture, InverseSize * (fragCoord + vec3(0, 0, 1))).x;

    // Sample the obstacle velocity texture to get the obstacle velocity for each neighbour cell
    vec3 obsVelL = texture(ObstacleVelocityTexture, InverseSize * (fragCoord - vec3(1, 0, 0))).xyz;
    vec3 obsVelR = texture(ObstacleVelocityTexture, InverseSize * (fragCoord + vec3(1, 0, 0))).xyz;
    vec3 obsVelB = texture(ObstacleVelocityTexture, InverseSize * (fragCoord - vec3(0, 1, 0))).xyz;
    vec3 obsVelT = texture(ObstacleVelocityTexture, InverseSize * (fragCoord + vec3(0, 1, 0))).xyz;
    vec3 obsVelU = texture(ObstacleVelocityTexture, InverseSize * (fragCoord - vec3(0, 0, 1))).xyz;
    vec3 obsVelD = texture(ObstacleVelocityTexture, InverseSize * (fragCoord + vec3(0, 0, 1))).xyz;

    // If the neighbour cell is an obstacle, use the obstacle velocity 
    // instead of the neighbour cell velocity
    if (obsL > 0.0) vL = obsVelL;
    if (obsR > 0.0) vR = obsVelR;
    if (obsB > 0.0) vB = obsVelB;
    if (obsT > 0.0) vT = obsVelT;
    if (obsU > 0.0) vU = obsVelU;
    if (obsD > 0.0) vD = obsVelD;

    // Calculate the divergence
    FragColor = 0.5 * (vR.x - vL.x + vT.y - vB.y + vD.z - vU.z);

    // If the divergence is very small, set it to zero
    if (abs(FragColor) < 0.0001)
        FragColor = 0.0;
}