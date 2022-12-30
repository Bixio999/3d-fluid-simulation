/*
    OpenGL 4.1 - Dynamic Obstacle Voxelization - Vertex Shader

    This shader is part of the program that voxelizes the obstacles in the scene
    in order to define those simulation grid cells that are occupied by obstacles.

    The Inside-Outside Voxelization algorithm is based on the Stencil Shadow Volume
    algorithm, which uses the stencil to obtain a layered voxelization of the model. 
    The model is rendered three times: 
    - First, the model is rendered with the stencil test and front face culling 
      enabled, and the stencil operation set to increment the stencil value. 
    - Second, the model is rendered with the stencil test and back face culling 
      enabled, and the stencil operation set to decrement the stencil value.
    After these two passes, the stencil buffer contains a non-zero value for
    those grid cells that are occupied by the model. The third pass renders the
    model to the color buffer, but this time with the stencil write disabled. 

    This vertex shader is used to define the correct projection matrix for each
    slice of the layered rendering. The projection matrix is defined by setting
    the near plane to a value that is proportional to the slice number, and the
    far plane to a high value. This way, the model is culled by the near plane,
    in order to render only those triangles that are visible in the current slice.

    The Voxelization program is composed by the following shaders:
    - Vertex Shader: This shader
    - Geometry Shader: set_layer.geom - Sets the layer and enable layered rendering
    - Fragment Shader: fill.frag - Write in the grid cells that are occupied by the model
*/

#version 410 core

layout (location = 0) in vec3 position;

uniform float grid_depth; // Number of slices
uniform float scaling_factor; // Scale of the model

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection; // Unmodified projection matrix

out int vInstance; // Instance ID

void main()
{
    vInstance = gl_InstanceID; // Set the instance ID

    // Define the projection matrix for the current slice of the layered rendering

    // The near plane is proportional to the slice number, with a value inside the 
    // range [0, 2 * scaling_factor], where scaling_factor is also equal to half
    // the edge of the fluid cube. The 1.0 value is added to compensate for the
    // camera position, which is set to (0, 0, 1) compared to the fluid cube front face.
    float nearPlane = 2 * scaling_factor * ((gl_InstanceID + 1) / grid_depth) + 1.0;
    float farPlane = 100;

    float newProj1 = -2.0 / (farPlane - nearPlane);
    float newProj2 = -(farPlane + nearPlane) / (farPlane - nearPlane);

    mat4 newProj = mat4(projection);

    newProj[2][2] = newProj1;
    newProj[3][2] = newProj2;

    // Set the vertex position in the current slice with the new projection matrix
    gl_Position = newProj * view * model * vec4(position, 1.0);
}