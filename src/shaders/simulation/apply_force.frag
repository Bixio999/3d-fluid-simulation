/*
    OpenGL 4.1 Core - Fluid Simulation: Application of External Forces - Fragment Shader

    This shader is responsible for applying external forces to the fluid
    with the help of a gaussian splat. The gaussian splat is applied to
    the velocity field and the result is added to the current the velocity 
    field.

    The gaussian splat is applied to the velocity field at the given emitter's
    position, along with its radius and force. The gaussian splat is calculated
    using the following formula:

    e^(-1 * (dx^2 + dy^2 + dz^2) / (radius^2 * 2.0))
    where dx, dy, dz are the distance of the current fragment from the emitter's
    position.

    The result of the gaussian splat is multiplied by the force and added to the
    current velocity field.

    The Application of External Forces program is composed by the folliwing shaders:
    - Vertex Shader:   load_vertices.vert - load the vertices of the quad
    - Geometry Shader: set_layer.geom - set the layer of the quad and enable
      the layered rendering
    - Fragment Shader: this shader
*/

#version 410 core

out vec4 FragColor; // Output data

uniform vec3 InverseSize; // Inverse of the size of the simulation grid

// Velocity texture
uniform sampler3D VelocityTexture;

uniform vec3 force; // Force to be applied
uniform vec3 center; // Center of the gaussian splat
uniform float radius; // Radius of the gaussian splat

in float layer; // Layer of the 3D texture

void main()
{
    // Calculate the position of the current fragment in the 3D texture
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
    float gaussianSplat = exp(-1 * (dx + dy + dz) / (radius * radius * 2.0));

    // Apply the gaussian splat to the emitter's velocity
    vec3 vForce = force * gaussianSplat; 

    // Add the emitter's velocity to the current velocity field
    vec3 finalVec = vForce + texture(VelocityTexture, fragCoord * InverseSize).xyz;
    
    // Set the output color
    FragColor = vec4(finalVec, 1.0);

    // If the output color is too small, set it to zero
    if (length(FragColor) < 0.0001)
        FragColor = vec4(0.0);
}