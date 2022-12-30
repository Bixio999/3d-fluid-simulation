/*
    Simple OpenGL 4.1 fragment shader: 
    - sets the output color to a uniform color
*/

#version 410 core

out vec4 FragColor;

uniform vec4 color;

void main()
{
    FragColor = vec4(color);
}