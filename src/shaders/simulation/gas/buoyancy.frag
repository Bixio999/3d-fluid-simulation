/*
    OpenGL 4.1 Core - Gas Simulation: Buoyancy Force - Fragment Shader

    This fragment shader is used to calculate the buoyancy force for the 
    gas simulation. The buoyancy force is calculated by comparing the
    temperature of the current cell with the ambient temperature. If the
    temperature is higher than the ambient temperature, the buoyancy force
    is calculated by the formula:

    buoyancy = (temperature - ambientTemperature) * gasBuoyancy * timeStep - density * gasWeight;

    The buoyancy force is then added to the current velocity of the cell.
    This physics force represents the ability of the gas to rise up in
    the air if the temperature is higher than the ambient temperature.

    The fragment shader is executed in a 3D texture with the size of the
    simulation grid. The fragment shader is executed for each layer of the
    3D texture. 

    The Buoyancy Force program is composed by the following shaders:
    - Vertex Shader: load_vertices.vert - load the quad vertices
    - Geometry Shader: set_layer - set the current layer of the 3D texture
      and enable the layered rendering
    - Fragment Shader: this shader
*/

#version 410 core

out vec4 FragColor; // Output data

// Input texture samplers
uniform sampler3D VelocityTexture;
uniform sampler3D DensityTexture;
uniform sampler3D TemperatureTexture;

uniform float ambientTemperature; // Ambient temperature
uniform float timeStep; // Time step
uniform float gasBuoyancy; // Damping factor
uniform float gasWeight; // Weight of the gas

uniform vec3 InverseSize; // Inverse size of the simulation grid

in float layer; // Current layer of the 3D texture

// Main function
void main()
{
    // Calculate the current cell coordinates in the 3D texture
    vec3 FragCoord = vec3(gl_FragCoord.xy, layer);

    // Sample the current velocity and temperature
    vec3 currentVel = texture(VelocityTexture, FragCoord * InverseSize).xyz; 
    float temp = texture(TemperatureTexture, FragCoord * InverseSize).x;

    // Check if the temperature is higher than the ambient temperature
    if (temp > ambientTemperature)
    {
        // Calculate the buoyancy force
        float dens = texture(DensityTexture, FragCoord * InverseSize).x;
        float buoyancy = (temp - ambientTemperature) * gasBuoyancy * timeStep - dens * gasWeight;
        vec3 bForce = vec3(0, 1, 0) * buoyancy;

        // Add the buoyancy force to the current velocity
        FragColor = vec4(currentVel + bForce, 1.0);
    }
    else 
    {
        // If the temperature is lower than the ambient temperature, 
        // keep the current velocity
        FragColor = vec4(currentVel, 1.0);
    }

    // If the values are too small, set them to zero
    if (length(FragColor) < 0.0001)
        FragColor = vec4(0.0);
}