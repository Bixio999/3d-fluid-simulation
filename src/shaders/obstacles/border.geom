
#version 410 core

layout(lines) in;
layout(line_strip, max_vertices = 2) out;
 
in int vInstance[2];
out float layer;
 
void main()
{
    gl_Layer = vInstance[0];
    layer = float(gl_Layer) + 0.5;

    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    EndPrimitive();
}