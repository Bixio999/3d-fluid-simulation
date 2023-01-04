/*
    OpenGL 4.1 Core - RayData and Raymarching - Vertex Shader

    This shader is part of the programs that create the ray data texture,
    and execute the raymarching through the fluid volume to render the 
    current target fluid.

    This vertex shader is used in the two raydata texture generation programs,
    and the raymarching program. The common tasks are the computation on the 
    texture space coordinates, and the positioning of the fluid cube in clip
    space coordinates for rendering. Also, for the raymarching program this
    shader produces as output data the direction of the light in view space 
    and the vector from the fragment to the camera, which are data used for 
    liquid raymarching to compute the GGX lighting on the surface of the 
    fluid.
*/

#version 410 core

layout (location = 0) in vec3 aPos; 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightVector; // light direction in world coords

out vec3 texPos; // texture space coords
out vec3 ogPos; // cube local space coords

out vec3 lightDir; // light direction in view space
out vec3 vViewPosition; // fragment-camera vector in view space

// Main function 
void main()
{
    // Compute the vertex position in view space
    vec4 pos = view * model * vec4(aPos, 1.0);

    // Calculate the fragment-view vector by negating the vertex
    // position in view space (camera is the origin: camera - vertex = - vertex)
    vViewPosition = -pos.xyz;

    // Calculate the vertex position in clip space
    pos = projection * pos;

    // Calculate the texture space coordinates from cube local
    // space coords
    texPos = (aPos + 1) / 2.0;
    texPos.z = 1 - texPos.z; // Invert the z axis
    texPos = clamp(texPos, 0, 1); // Avoid values outside range

    ogPos = aPos; // Output the vertex position in local space

    // Calculate the light direction in view space
    lightDir = vec3(view * vec4(lightVector, 0.0));

    // Assign the vertex position for rendering
    gl_Position = pos;
}