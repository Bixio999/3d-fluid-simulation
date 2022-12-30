/*
    OpenGL 4.1 - Geometry Shader

    This shader is used to enable the layered rendering by setting the gl_Layer
    variable. The gl_Layer variable is used to set the layer of the current
    primitive. This variable is only available in the geometry shader.
*/

#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
 
in int vInstance[3];
out float layer;
 
void main()
{
    gl_Layer = vInstance[0];
    layer = float(gl_Layer) + 0.5;
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}