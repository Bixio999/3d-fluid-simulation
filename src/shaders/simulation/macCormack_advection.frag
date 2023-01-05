/*
    OpenGL 4.1 Core - Fluid Simulation: Mac Cormack Advection - Fragment Shader

    This shader is used to advect a fluid field using the Mac Cormack method.
    This method is a second order accurate method which is very fast. 
    The method is based on two advection steps. The first step is a forward
    advection step. The second step is a backward advection step. The final 
    value is then computed by taking the average of the two advection steps. 
    The first step, also called the predictor step, is used to compute the
    value at the next time step. The second step, also called the corrector
    step, is used to compute the value at the current time step by using the
    values from the first step through a backward advection (negative time
    step). The final step is to compute the new value by adding to the 
    first step value the difference between the current and the second step
    values, divided by two. This allows to reduce the error of the first
    step by using the second step to correct it. 
    Because the Mac Cormack method is not stable as the Semi-Lagrangian
    method, it is necessary to compute a range of values for the field
    which is advected, and to clamp the final value to this range. Those
    values are computed by finding the minimum and maximum values of the
    field in the neighborhood of the current fragment. 
    The advection in the two steps is performed with the Semi-Lagrangian
    method, which is used as operator to advect the field.

    To handle the obstacles, the advection is performed only if the current
    fragment is not an obstacle. 

    The Mac Cormack Advection program is composed by the following shaders:
    - Vertex Shader:   load_vertices.vert - load the vertices of the quad
    - Geometry Shader: set_layer.geom - set the layer of the quad and enable
      the layered rendering
    - Fragment Shader: this shader
*/

#version 410 core

out vec4 FragColor; // Output data

// Input texture samplers
uniform sampler3D VelocityTexture;
uniform sampler3D SourceTexture; // Source field to advect
uniform sampler3D Phi1HatTexture;
uniform sampler3D Phi2HatTexture;

uniform sampler3D ObstacleTexture;

uniform float timeStep; // Time step
uniform vec3 InverseSize; // Inverse of the size of the simulation grid
uniform float dissipation; // Dissipation factor

in float layer; // Layer of the 3D texture

void main()
{
    // Compute the coordinates of the current fragment in the 3D texture
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    // Get the obstacle value
    float obstacle = texture(ObstacleTexture, InverseSize * fragCoord).r;

    // Initialize the new value to zero
    vec3 newVal = vec3(0.0);

    // If the current fragment is not an obstacle, perform the advection
    if (obstacle < 1.0)
    {
        // Sample the velocity field at the current fragment coordinates
        vec3 u = texture(VelocityTexture, InverseSize * fragCoord).xyz;

        // Compute the coordinates of the fragment in the previous time step
        vec3 coord = (fragCoord - timeStep * u);

        // Find the corner of the cell containing the fragment
        coord = floor(coord + vec3(0.5));

        // Sample the source field in the neighborhood of the fragment
        vec3 neighborhood[8];

        neighborhood[0] = texture(SourceTexture, InverseSize * (coord + vec3(-0.5, -0.5, -0.5))).xyz;
        neighborhood[1] = texture(SourceTexture, InverseSize * (coord + vec3(-0.5, -0.5,  0.5))).xyz;
        neighborhood[2] = texture(SourceTexture, InverseSize * (coord + vec3(-0.5,  0.5, -0.5))).xyz;
        neighborhood[3] = texture(SourceTexture, InverseSize * (coord + vec3(-0.5,  0.5,  0.5))).xyz;
        neighborhood[4] = texture(SourceTexture, InverseSize * (coord + vec3( 0.5, -0.5, -0.5))).xyz;
        neighborhood[5] = texture(SourceTexture, InverseSize * (coord + vec3( 0.5, -0.5,  0.5))).xyz;
        neighborhood[6] = texture(SourceTexture, InverseSize * (coord + vec3( 0.5,  0.5, -0.5))).xyz;
        neighborhood[7] = texture(SourceTexture, InverseSize * (coord + vec3( 0.5,  0.5,  0.5))).xyz;

        // Find the minimum and maximum values in the neighborhood
        vec3 minVal = neighborhood[0];
        vec3 maxVal = neighborhood[0];
        for (int i = 1; i < 8; i++)
        {
            minVal = min(minVal, neighborhood[i]);
            maxVal = max(maxVal, neighborhood[i]);
        }

        // Sample the first and second step values of the Mac Cormack method
        vec3 phi1_hat = texture(Phi1HatTexture, InverseSize * fragCoord).xyz;
        vec3 phi2_hat = texture(Phi2HatTexture, InverseSize * fragCoord).xyz;

        // Sample the source field at the current fragment coordinates
        vec3 val = texture(SourceTexture, InverseSize * fragCoord).xyz;

        // Compute the new value
        newVal = phi1_hat + 0.5 * (val - phi2_hat);

        // Clamp the new value to the range of values in the neighborhood
        newVal = clamp(newVal, minVal, maxVal);
    }

    // Set the output color
    FragColor = vec4(newVal, 1.0);

    // If the new value is very small, set it to zero
    if (length(FragColor) < 0.0001)
        FragColor = vec4(0.0);
}