/*
    OpenGL 4.1 Core - Fluid Simulation: Density Splatting - Fragment Shader

    This shader is used to splat a density value at a given position in the
    simulation grid. The density value is splatted using a gaussian function
    with a given radius. The density value is hard-coded to 1.2f for gas 
    simulation, otherwise is set to the negate value of the emitter's radius
    in order to follow the level set property (values represent the distance
    to the surface) with the gaussian splat (written values will be in the
    range [0.0, -radius] along each radius of the drawn sphere). 
    The density splatting is used to add fluid to the simulation.

    The Density Splatting program is composed by the following shaders:
    - Vertex Shader:   load_vertices.vert - load the vertices of the quad
    - Geometry Shader: set_layer.geom - set the layer of the quad and enable
      the layered rendering
    - Fragment Shader: this shader
*/

#version 410 core

out float outColor; // Output data

uniform vec3 InverseSize; // Inverse size of the simulation grid

uniform sampler3D DensityTexture;

uniform float dyeIntensity; // Intensity of the fluid to splat
uniform vec3 center; // Position of the emitter
uniform float radius; // Radius of the emitter

uniform bool isLiquidSimulation; // Is the simulation liquid or gas

in float layer; // Layer of the 3D texture

// Main function
void main() 
{
    // Calculate the position of the fragment in the 3D texture
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    // Calculate the distance from the center of the emitter
    float dx = (fragCoord.x - center.x);
    float dy = (fragCoord.y - center.y);
    float dz = (fragCoord.z - center.z);

    // Square the distance
    dx = dx * dx;
    dy = dy * dy;
    dz = dz * dz;

    // Calculate the gaussian splat
    float gaussianSplat = exp(-1 * (dx + dy + dz) / (radius * radius * 2));

    // Clamp the splat
    gaussianSplat = clamp(gaussianSplat, 0.0, 1.0);

    // Sample the density texture
    float currDensity = texture(DensityTexture, fragCoord * InverseSize).r;

    // Calculate the final density
    float finalDensity = currDensity;
    if (isLiquidSimulation)
    {
        // If the simulation is liquid, the density is overwritten with the
        // splat value
        if (gaussianSplat > 0.01) finalDensity = gaussianSplat * dyeIntensity;
    }
    else
    {
        // Otherwise add the splat value to the current density
        finalDensity += gaussianSplat * dyeIntensity;

        // Clamp the density
        finalDensity = min(finalDensity, 1.0);
    }

    // Write the final density to the output
    outColor = finalDensity;
}