/*
    OpenGL 4.1 Core - Fluid Simulation: Pressure Projection - Fragment Shader

    This shader is used to project the pressure field onto the velocity field.
    To do this, we use the pressure gradient to calculate the new velocity 
    field. The gradient is calculated using the pressure values of the 6 
    neighboring cells. The new velocity is calculated by subtracting the 
    gradient from the current velocity.  

    In order to handle the obstacles, we first need to check if the current cell 
    is an obstacle. If it is, we set as the new velocity the velocity of the
    obstacle at that cell. If it is not, we calculate the new velocity as
    described above, but we also need to check if the neighboring cells are
    obstacles. If they are, we set the pressure of the obstacle cell to the
    pressure of the current cell. This way, the pressure gradient will be 0
    for the obstacle cells direction, and the new velocity will not be affected
    by them. Also, we compute a mask that will be used to substitute the
    velocity vector components of the field with the obstacle velocity vector
    components. This way, we can easily perform free-slip boundary conditions 
    by setting the mask to 0 for the obstacle cells direction. This way, the
    fluid will not be able to flow through the obstacle cells.

    The Pressure Projection program is composed by the following shaders:
    - Vertex Shader:   load_vertices.vert - load the vertices of the quad
    - Geometry Shader: set_layer.geom - set the layer of the quad and enable
      the layered rendering
    - Fragment Shader: this shader
*/

#version 410 core

out vec3 FragColor; // Output data

// Input texture samplers
uniform sampler3D VelocityTexture;
uniform sampler3D PressureTexture;
uniform sampler3D ObstacleTexture;
uniform sampler3D ObstacleVelocityTexture;

uniform vec3 InverseSize; // Inverse size of the simulation grid

in float layer; // Layer of the 3D texture

// Main function
void main()
{
    // Calculate the fragment coordinates in the 3D texture
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    // Sample the obstacle texture
    float obsCenter = texture(ObstacleTexture, fragCoord * InverseSize).r;

    // Sample the velocity texture
    FragColor = texture(VelocityTexture, fragCoord * InverseSize).xyz;

    // If the current cell is not an obstacle, apply the pressure projection
    if (obsCenter < 1.0)
    {
        // Sample the pressure texture in the current cell
        float pCenter = texture(PressureTexture, fragCoord * InverseSize).r;

        // Sample the pressure texture in the 6 neighboring cells
        float pLeft = texture(PressureTexture, (fragCoord + vec3(-1, 0, 0)) * InverseSize).x;
        float pRight = texture(PressureTexture, (fragCoord + vec3(1, 0, 0)) * InverseSize).x;
        float pBack = texture(PressureTexture, (fragCoord + vec3(0, -1, 0)) * InverseSize).x;
        float pFront = texture(PressureTexture, (fragCoord + vec3(0, 1, 0)) * InverseSize).x;
        float pBottom = texture(PressureTexture, (fragCoord + vec3(0, 0, -1)) * InverseSize).x;
        float pTop = texture(PressureTexture, (fragCoord + vec3(0, 0, 1)) * InverseSize).x;

        // Sample the obstacle texture in the 6 neighboring cells
        vec3 obsLeft = texture(ObstacleTexture, (fragCoord + vec3(-1, 0, 0)) * InverseSize).rgb;
        vec3 obsRight = texture(ObstacleTexture, (fragCoord + vec3(1, 0, 0)) * InverseSize).rgb;
        vec3 obsBack = texture(ObstacleTexture, (fragCoord + vec3(0, -1, 0)) * InverseSize).rgb;
        vec3 obsFront = texture(ObstacleTexture, (fragCoord + vec3(0, 1, 0)) * InverseSize).rgb;
        vec3 obsBottom = texture(ObstacleTexture, (fragCoord + vec3(0, 0, -1)) * InverseSize).rgb;
        vec3 obsTop = texture(ObstacleTexture, (fragCoord + vec3(0, 0, 1)) * InverseSize).rgb;

        // Sample the obstacle velocity texture in the 6 neighboring cells
        vec3 obsVelocityLeft = texture(ObstacleVelocityTexture, (fragCoord + vec3(-1, 0, 0)) * InverseSize).rgb;
        vec3 obsVelocityRight = texture(ObstacleVelocityTexture, (fragCoord + vec3(1, 0, 0)) * InverseSize).rgb;
        vec3 obsVelocityBack = texture(ObstacleVelocityTexture, (fragCoord + vec3(0, -1, 0)) * InverseSize).rgb;
        vec3 obsVelocityFront = texture(ObstacleVelocityTexture, (fragCoord + vec3(0, 1, 0)) * InverseSize).rgb;
        vec3 obsVelocityBottom = texture(ObstacleVelocityTexture, (fragCoord + vec3(0, 0, -1)) * InverseSize).rgb;
        vec3 obsVelocityTop = texture(ObstacleVelocityTexture, (fragCoord + vec3(0, 0, 1)) * InverseSize).rgb;

        // Initialize the merged obstacle velocity
        vec3 obsVelocity = vec3(0.0);

        // Initialize the mask
        vec3 obsMask = vec3(1.0);

        // Check if the neighboring cells are obstacles, and if they are, set
        // the pressure of the obstacle cell to the pressure of the current cell
        // and set the merged obstacle velocity component to the obstacle velocity
        // component of the neighboring cell. Also, set the mask component to 0
        // for the obstacle cells direction.

        if (obsLeft.r > 0.0)
        {
            pLeft = pCenter;
            obsVelocity.x = obsVelocityLeft.x;
            obsMask.x = 0.0;
        }
        if (obsRight.r > 0.0)
        {
            pRight = pCenter;
            obsVelocity.x = obsVelocityRight.x;
            obsMask.x = 0.0;
        }
        if (obsBack.r > 0.0)
        {
            pBack = pCenter;
            obsVelocity.y = obsVelocityBack.y;
            obsMask.y = 0.0;
        }
        if (obsFront.r > 0.0)
        {
            pFront = pCenter;
            obsVelocity.y = obsVelocityFront.y;
            obsMask.y = 0.0;
        }
        if (obsBottom.r > 0.0)
        {
            pBottom = pCenter;
            obsVelocity.z = obsVelocityBottom.z;
            obsMask.z = 0.0;
        }
        if (obsTop.r > 0.0)
        {
            pTop = pCenter;
            obsVelocity.z = obsVelocityTop.z;
            obsMask.z = 0.0;
        }

        // Calculate the gradient of the pressure field with centered differences
        vec3 gradient = 0.5 * vec3(pRight - pLeft, pFront - pBack, pTop - pBottom);

        // Calculate the new velocity
        vec3 newVelocity = FragColor - gradient;

        // Merge the obstacle velocity
        newVelocity = (obsMask * newVelocity) + obsVelocity;

        // If the new velocity is very small, set it to zero
        if (length(newVelocity) < 0.0001)
            newVelocity = vec3(0.0);

        // Write the new velocity to the velocity texture
        FragColor = newVelocity;
    }
    else // If the current cell is an obstacle, set the velocity to the obstacle velocity
        FragColor = texture(ObstacleVelocityTexture, fragCoord * InverseSize).xyz;

}