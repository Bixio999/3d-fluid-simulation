/*
    OpenGL 4.1 Core - Obstacle Grid Borders drawing shader

    This shader is part of the program that draws the borders of the fluid
    simulation grid in the obstacle grid.

    Because the simulation is done in a 3D grid stored in a 3D texture, the
    borders have to be drawn with the correct perspective, meaning that the 
    borders are line strips along the edges of the 3D grid and full screen
    quads along the first and last layers of the 3D grid.
    So the algorithm used for drawing the borders is divided into two steps:
    1. draw the line strips along the edges of the 3D grid for each layer
    2. draw the full screen quads in the first and last layers of the 3D grid

    This geometry shader is used for step 1. It outputs the line strip vertices
    for each layer of the 3D grid.

    The Obstacle Grid Borders program for step 1 is composed by the following
    shaders:
    - Vertex Shader:   load_vertices.vert - generic shader used to load the
                       vertices of the line strips
    - Geometry Shader: this shader
    - Fragment Shader: fill.frag - generic shader used to draw geometry with
                       a solid color

    The Obstacle Grid Borders program for step 2 is composed by the following
    shaders:
    - Vertex Shader:   load_vertices.vert - generic shader used to load the
                       vertices of the full screen quads
    - Geometry Shader: none
    - Fragment Shader: fill.frag - generic shader used to draw geometry with
                       a solid color

    The program for step 2 doesn't need any particular operation because we
    only need to draw a full screen quad in the two layers, so we can use 
    simple shaders to do that, even without layered rendering. Another 
    difference between the two programs is the input VAO: the program for step
    1 uses the "borderVAO" which contains the vertices of the line strips,
    while the program for step 2 uses the "quadVAO" which contains the vertices
    of the full screen quads.
*/

#version 410 core

layout(lines) in;
layout(line_strip, max_vertices = 2) out; 
 
// Instance ID
in int vInstance[2];
out float layer; // we need to output the layer to the fragment shader because
                 // is used to calculate the texture coordinates
 
// main function
void main()
{
    gl_Layer = vInstance[0];
    layer = float(gl_Layer) + 0.5; // we add 0.5 to the layer to get the center
                                   // of the layer in the texture coordinates

    // we output the vertices of the line strip
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    EndPrimitive();
}