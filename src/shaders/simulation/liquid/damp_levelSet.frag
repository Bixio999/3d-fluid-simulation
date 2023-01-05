/*
    OpenGL 4.1 Core - Liquid Simulation: Level Set Damping - Fragment Shader

    This shader is part of the liquid simulation shaders. It is used to 
    dampen the level set values to the equilibrium height, due to the 
    impossibility of the pressure solver to compensate for the gravity
    force. This is a solution needed to avoid the simulation to sink
    into the bottom of the volume. The damping factor is a value between
    0 and 1, where 0 means no damping and 1 means full damping. The
    equilibrium height is the height of the liquid surface at the
    equilibrium state, where the pressure is zero. Different damping
    effects can be achieved by changing the equilibrium function. The 
    one used here is the simplest one, where the equilibrium height is
    calculated as the difference between the height of the fragment and 
    the equilibrium height. 

    The Level Set Damping program is composed by the following shaders:
    - Vertex Shader: load_vertices.vert - loads the vertices of the quad
    - Geometry Shader: set_layer.geom - sets the layer of the quad and 
      enables the layered rendering
    - Fragment Shader: this shader
*/

#version 410 core

out vec4 FragColor; // Output data

uniform sampler3D LevelSetTexture;
uniform sampler3D ObstacleTexture;
uniform vec3 InverseSize; // Inverse size of the volume

uniform float dampingFactor; // Damping factor [0, 1]
uniform float equilibriumHeight; // Equilibrium height percentage [0, 1]
uniform float grid_height; // Height of the grid

in float layer; // Layer of the 3D texture

// Main function
void main()
{
    // Calculate the fragment coordinates in the 3D texture
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    // Sample the obstacle texture 
    float obstacle = texture(ObstacleTexture, fragCoord * InverseSize).r;

    // Calculate the new level set value
    float newLevelSet = 0.0;

    // If the fragment is not inside an obstacle
    if (obstacle <= 0.0)
    {
        // Sample the current level set value
        float currLevelSet = texture(LevelSetTexture, fragCoord * InverseSize).r;

        // Calculate the equilibrium level set value
        float equilibriumLevelSet = fragCoord.y - grid_height * equilibriumHeight;

        // Damp the level set value if it is below the equilibrium surface
        if (equilibriumLevelSet < 0.0)
            newLevelSet = (1 - dampingFactor) * currLevelSet + dampingFactor * (equilibriumLevelSet);
        else // Otherwise, keep the current level set value
            newLevelSet = currLevelSet;

        // Avoid numerical errors
        if (abs(newLevelSet) < 0.0001)
            newLevelSet = 0.0;
    }

    // Output the new level set value
    FragColor = vec4(newLevelSet, 0.0, 0.0, 1.0);
}