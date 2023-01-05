/*
    OpenGL 4.1 Core - Liquid Simulation: Level Set Initialization - Fragment Shader

    This shader is used to initialize the level set field. It is used
    to set the initial height of the liquid. The level set field is
    initialized to the floor of the y coordinate minus the initial
    height of the liquid. The initial height is a value between 0 and 1
    that represents the percentage of the grid that is filled with
    liquid. 

    The level set field is used to determine the distance to the
    surface of the liquid.

    The Level Set Initialization program is composed by the following
    shaders:
    - Vertex Shader: load_vertices.vert - loads the vertices of the quad
    - Geometry Shader: set_layer.geom - sets the layer of the quad and 
      enables the layered rendering
    - Fragment Shader: this shader
*/

#version 410

out vec4 FragColor; // Output data

uniform float initialHeight; // Initial height of the liquid [0, 1]
uniform float grid_height; // Height of the grid

in float layer; // Layer of the 3D texture

// Main function
void main()
{
    // Set the initial level set value
    float levelSet = floor(gl_FragCoord.y - initialHeight * grid_height);

    // Set the output color
    FragColor = vec4(levelSet, 0.0, 0.0, 1.0);
}
