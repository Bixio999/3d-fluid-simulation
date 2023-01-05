/*
    OpenGL 4.1 Core - Fluid Simulation: Jacobi Pressure Solver - Fragment Shader

    This fragment shader computes the Jacobi pressure solver. It is used to
    solve the pressure equation. The pressure is computed in a way that
    the divergence of the velocity field is zero. This is done by
    iteratively solving the pressure equation. Each iteration uses the 
    pressure values of the previous iteration to compute the new pressure
    values. Those values are computed by averaging the pressure values
    of the six neighboring cells.

    To handle the interaction with the obstacles, the pressure values
    of the cells that are inside the obstacle are set to the pressure
    value of the cell in the current position. This is done by checking
    if the obstacle texture contains a value greater than zero at the
    position of the cell. 

    The Jacobi Pressure Solver program is composed by the following shaders:
    - Vertex Shader:   load_vertices.vert - load the vertices of the quad
    - Geometry Shader: set_layer.geom - set the layer of the quad and enable
      the layered rendering
    - Fragment Shader: this shader
*/

#version 410 core

out vec4 FragColor; // Output data

// Input texture samplers
uniform sampler3D Divergence;
uniform sampler3D Pressure;
uniform sampler3D Obstacle;

uniform vec3 InverseSize; // Inverse size of the simulation grid

in float layer; // Layer of the 3D texture

// Main function
void main()
{
    // Compute the position of the current cell in the 3D texture
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    // Sample the divergence and pressure value of the previous iteration
    float divergence = texture(Divergence, fragCoord * InverseSize).r;
    float pressure = texture(Pressure, fragCoord * InverseSize).r;

    // Sample the pressure values of the six neighboring cells
    float pLeft = texture(Pressure, (fragCoord + vec3(-1.0, 0.0, 0.0)) * InverseSize).r;
    float pRight = texture(Pressure, (fragCoord + vec3(1.0, 0.0, 0.0)) * InverseSize).r;
    float pDown = texture(Pressure, (fragCoord + vec3(0.0, -1.0, 0.0)) * InverseSize).r;
    float pUp = texture(Pressure, (fragCoord + vec3(0.0, 1.0, 0.0)) * InverseSize).r;
    float pBottom = texture(Pressure, (fragCoord + vec3(0.0, 0.0, -1.0)) * InverseSize).r;
    float pTop = texture(Pressure, (fragCoord + vec3(0.0, 0.0, 1.0)) * InverseSize).r;

    // Sample the obstacle texture 
    float obsLeft = texture(Obstacle, (fragCoord + vec3(-1.0, 0.0, 0.0)) * InverseSize).r;
    float obsRight = texture(Obstacle, (fragCoord + vec3(1.0, 0.0, 0.0)) * InverseSize).r;
    float obsDown = texture(Obstacle, (fragCoord + vec3(0.0, -1.0, 0.0)) * InverseSize).r;
    float obsUp = texture(Obstacle, (fragCoord + vec3(0.0, 1.0, 0.0)) * InverseSize).r;
    float obsBottom = texture(Obstacle, (fragCoord + vec3(0.0, 0.0, -1.0)) * InverseSize).r;
    float obsTop = texture(Obstacle, (fragCoord + vec3(0.0, 0.0, 1.0)) * InverseSize).r;

    // If a neighboring cell is inside an obstacle, set its pressure value 
    // to the pressure value of the current cell
    if (obsLeft > 0.0) pLeft = pressure;
    if (obsRight > 0.0) pRight = pressure;
    if (obsDown > 0.0) pDown = pressure;
    if (obsUp > 0.0) pUp = pressure;
    if (obsBottom > 0.0) pBottom = pressure;
    if (obsTop > 0.0) pTop = pressure;

    // Compute the new pressure value
    float newPressure = (pLeft + pRight + pDown + pUp + pBottom + pTop - divergence) / 6.0f;

    // If the new pressure value is very small, set it to zero
    if (abs(newPressure) < 0.0001)
        newPressure = 0.0;

    // Output the new pressure value
    FragColor = vec4(newPressure, 0.0, 0.0, 1.0);
}