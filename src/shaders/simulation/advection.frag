/*
    OpenGL 4.1 Core - Fluid Simulation: Semi-Lagrangian Advection - Fragment Shader

    This fragment shader is used to advect the fluid fields using the
    semi-Lagrangian method. The semi-Lagrangian method is a simple
    advection method that is very fast and stable. It is also very
    easy to implement. The semi-Lagrangian method is based on the
    idea of tracing the fluid particles backwards in time. However,
    it is not very accurate. The semi-Lagrangian method is often
    used as a pre-processing step for more accurate advection
    methods, and that's the case for the used advection method in 
    the implemented fluid simulation (see MacCormack advection).

    The semi-Lagrangian method is based on the following equation:

    x(t) = x(t - dt) + dt * u(x(t - dt))
    where x is the position of the fluid particle, t is the time,
    dt is the time step, and u is the velocity field.

    The new position is then used to sample the source field at the 
    new position. The sampled value is then used as the new value
    for the current position.

    The Semi-Lagrangian method is also known as the Backward Euler
    method.

    The Semi-Lagrangian Advection program is composed by the following
    shaders: 
    - Vertex Shader:   load_vertices.vert - load the vertices of the quad
    - Geometry Shader: set_layer.geom - set the layer of the quad and enable
      the layered rendering
    - Fragment Shader: this shader
*/

#version 410 core

out vec4 FragColor; // Output data

// Input texture samplers
uniform sampler3D VelocityTexture; 
uniform sampler3D SourceTexture;
uniform sampler3D ObstacleTexture;

uniform float timeStep; // Time step
uniform vec3 InverseSize; // Inverse size of the simulation grid
uniform float dissipation; // Dissipation factor

in float layer; // Layer of the 3D texture

// Main function
void main()
{
    // Calculate the current position in the 3D texture
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    // Sample the obstacle texture
    float obstacle = texture(ObstacleTexture, InverseSize * fragCoord).r;

    // Initialize the final color
    vec4 finalColor = vec4(0.0);

    // If the current position is not an obstacle, advect the fluid
    if (obstacle < 1.0)
    {
        // Sample the velocity field at the current position
        vec3 u = texture(VelocityTexture, InverseSize * fragCoord).xyz;

        // Calculate the new position using the semi-Lagrangian method
        vec3 coord = InverseSize * (fragCoord - timeStep * u);

        // Sample the source field at the new position 
        finalColor = dissipation * texture(SourceTexture, coord);

        // Avoid numerical errors
        if (length(finalColor) < 0.0001)
            finalColor = vec4(0.0);
    }

    // Set the final color
    FragColor = finalColor;
}