/*
    OpenGL 4.1 Core - Liquid Simulation: Gravity Shader - Fragment Shader

    This shader is part of the program that simulates the movement of a liquid
    in a 3D environment. This shader applies gravity to the velocity field for 
    those cells that are below the level set surface, due to the assumption of 
    negligible air effects. The gravity acceleration is applied in the negative
    y direction.

    The Gravity Shader program is composed of the following shaders:
    - Vertex Shader:   load_vertices.vert - loads the vertices of the quad
    - Geometry Shader: set_layer.geom - sets the layer of the 3D texture and
      enables the layered rendering
    - Fragment Shader: this shader
*/

#version 410 core

out vec4 FragColor; // Output data

// Input texture samplers
uniform sampler3D VelocityTexture;
uniform sampler3D LevelSetTexture;

uniform vec3 InverseSize; // Inverse size of the simulation grid

uniform float timeStep; // Time step of the simulation
uniform float gravityAcceleration; // Acceleration due to gravity
uniform float levelSetThreshold; // Threshold for the level set

in float layer; // Layer of the 3D texture

void main()
{
    // Get the coordinates of the current fragment in the 3D texture
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    // Get the velocity and level set values of the current fragment
    vec3 velocity = texture(VelocityTexture, fragCoord * InverseSize).xyz;
    float levelSet = texture(LevelSetTexture, fragCoord * InverseSize).r;

    // Apply gravity to the velocity field if the fragment is below the level
    // set surface
    if (levelSet < levelSetThreshold)
        velocity.y -= gravityAcceleration * timeStep;

    // Output the new velocity value
    FragColor = vec4(velocity, 0.0);
}