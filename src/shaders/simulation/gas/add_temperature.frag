/*
    OpenGL 4.1 Core - Gas Simulation: Temperature Splat Shader - Fragment Shader

    This shader is used to splat temperature into the temperature texture.
    The splat is a gaussian distribution centered in the position of the
    current emitter, and with a radius of the emitter's radius.

    The splat is applied to the temperature texture, and the temperature
    is added to the current temperature in the texture.

    The Temperature Splat program is composed by the following shaders:
    - Vertex Shader:   load_vertices.vert - load the vertices of the quad
    - Geometry Shader: set_layer.geom - set the layer of the quad and enable
      the layered rendering
    - Fragment Shader: this shader
*/

#version 410 core

out vec4 FragColor; // Output data

uniform vec3 InverseSize; // Inverse size of the simulation grid

uniform sampler3D TemperatureTexture; // Temperature texture

uniform float temperature; // Value to splat
uniform vec3 center; // Center of the splat
uniform float radius; // Radius of the splat

in float layer; // Layer of the 3D texture

void main()
{
    // Compute the frame coordinates for the 3D texture
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    // Calculate the difference between the current position and the center
    float dx = (fragCoord.x - center.x);
    float dy = (fragCoord.y - center.y);
    float dz = (fragCoord.z - center.z);

    // Square the difference
    dx = dx * dx;
    dy = dy * dy;
    dz = dz * dz;

    // Calculate the gaussian splat
    float gaussianSplat = exp(-1 * (dx + dy + dz) / (radius * radius * 2.0));

    // Apply the splat to the temperature texture
    float appliedTemp = temperature * gaussianSplat; 
    float finalTemp = appliedTemp + texture(TemperatureTexture, fragCoord * InverseSize).x;

    // Set the output color
    FragColor = vec4(finalTemp, 0, 0, 1.0);

    // If the color is too small, set it to zero
    if (length(FragColor) < 0.0001)
        FragColor = vec4(0.0);
}