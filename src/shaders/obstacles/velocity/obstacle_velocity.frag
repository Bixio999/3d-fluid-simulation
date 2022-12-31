/*
    OpenGL 4.1 Core - Dynamic Obstacle Velocity Voxelization - Vertex Shader

    This shader is part of the program that computes and stores the velocity of
    the dynamic obstacles in the voxel grid. 

    The Velocity Voxelization algorithm calculates the velocity of the dynamic
    obstacles in the voxel grid. This is done by rendering the dynamic obstacle
    mesh and calculating the velocity of each vertex by comparing the current
    position of the vertex with the position of the vertex in the previous frame,
    and dividing the difference by the time elapsed between the two frames. The
    previous frame position is obtained by transforming the vertex position by
    the previous model matrix. From the per-vertex velocity, the velocity of the
    voxel is calculated by interpolation between the velocities of the vertices
    that compose the triangle that the voxel is inside. The velocity of the voxel
    for each slice of the grid is calculated in the geometry stage by the 
    intersection between each triangle of the mesh with the plane that represents 
    the current slice of the grid: if the entire triangle lays in the plane, it is
    entirely outputted, otherwise if the intersection is a segment, it is outputted
    as a quad obtained by extruding the segment along the normal projection of the
    triangle normal on the plane normal, and with an offset equal to the diagonal
    of the voxel.

    This fragment shader simply takes the velocity of the voxel computed in the
    geometry stage and adds it to the velocity of the voxel stored in the texture.

    The Velocity Voxelization program is composed by the following shaders:
    - Vertex Shader: Computes the velocity of the vertices of the mesh, and the 
        plane that represents the current slice of the grid.
    - Geometry Shader: obstacle_velocity.geom - Computes the intersection between
        the triangles of the mesh and the plane that represents the current slice
        of the grid, and outputs the intersection as a quad.
    - Fragment Shader: this shader 
*/

#version 410 core

out vec4 FragColor; // output color

in float layer; // layer of the voxel grid - z coordinate of texture space
in vec3 velocity; // velocity of the voxel

uniform sampler3D ObstacleVelocity; // obstacle velocity texture
uniform vec3 InverseSize; // inverse of the size of the voxel grid

// main function
void main()
{
    // compute the fragment coordinate in the texture space
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    // get the velocity of the voxel stored in the texture
    vec3 currVelocity = texture(ObstacleVelocity, fragCoord * InverseSize).xyz;

    // add the velocity of the voxel computed in the geometry stage to the
    // velocity of the voxel stored in the texture
    FragColor = vec4(currVelocity + velocity, 1.0);
}