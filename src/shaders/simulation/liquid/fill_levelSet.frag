#version 410

out vec4 FragColor;

uniform float initialHeight;
uniform float grid_height;

in float layer;

void main()
{
    float levelSet = floor(gl_FragCoord.y - initialHeight * grid_height);

    FragColor = vec4(levelSet, 0.0, 0.0, 1.0);
}
