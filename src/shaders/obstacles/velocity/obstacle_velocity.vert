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

    This vertex shader is used to calculate the plane that represents the current
    slide of the grid, identified by a point along the orthogonal projection of
    the fluid cube computed with the instance ID, and the direction of the plane
    normal, which is the same orthogonal direction. This plane is the one used to
    calculate the intersection between the triangles of the mesh in the geometry
    shader. Also the velocity of each vertex is calculated with the method described
    above.

    The Velocity Voxelization program is composed by the following shaders:
    - Vertex Shader: this shader
    - Geometry Shader: obstacle_velocity.geom - Computes the intersection between
        the triangles of the mesh and the plane that represents the current slice
        of the grid, and outputs the intersection as a quad.
    - Fragment Shader: obstacle_velocity.frag - Adds in the obstacle velocity 
        texture the velocity of the voxel.
*/

#version 410 core

layout (location = 0) in vec3 position;

uniform float grid_depth; // Number of slices of the grid

// Point along the orthogonal projection of the fluid cube
// corresponding to a point on the first layer plane
uniform vec3 firstLayerPoint; 
uniform vec3 layersDir; // Direction of the orthogonal projection of the fluid cube

uniform float deltaTime; // Time elapsed between the current and the previous frame

uniform mat4 prevModel; // Model matrix of the previous frame
uniform mat4 model; // Model matrix of the current frame
uniform mat4 view;
uniform mat4 projection;

out int vInstance; // Instance ID

out vec3 planePoint; // Point along the orthogonal projection of the fluid cube
                     // corresponding to a point on the current layer plane
out vec3 planeNormal; // Normalized direction of the orthogonal projection of the fluid cube

out vec3 vertVelocity; // Velocity of the vertex

out vec4 mPos; // Position of the vertex in the model space

void main()
{
    vInstance = gl_InstanceID; // Set the instance ID

    // Calculate the plane that represents the current slice of the grid
    planePoint = firstLayerPoint + layersDir * ((gl_InstanceID + 1) / grid_depth);
    planeNormal = normalize(layersDir);

    // Calculate the old position of the vertex by transforming it by the previous
    // model matrix
    vec3 oldPos = (projection * view * prevModel * vec4(position, 1.0)).xyz;

    // Calculate the current position of the vertex by transforming it by the current
    // model matrix
    vec4 newPos = model * vec4(position, 1.0);
    mPos = newPos;
    newPos = projection * view * newPos;

    // Calculate the velocity of the vertex by comparing the current position with
    // the old position and dividing the difference by the time elapsed between the
    // two frames
    vertVelocity = (newPos.xyz - oldPos) / deltaTime;

    gl_Position = newPos; // Set the vertex position in the clip space
}