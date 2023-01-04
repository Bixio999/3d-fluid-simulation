/*
    OpenGL 4.1 Core - RayData - Fragment Shader

    This shader is part of the program that creates the ray data texture, 
    which is used to render the fluid with the ray marching technique.

    The RayData texture is a 2D texture that stores the position and 
    direction of each ray that is going to be casted during the ray marching
    process. Because the camera can move, it may clip the fluid volume, so
    we need to create two textures, one for the front faces and one for the
    back faces, splitting the ray data texture into two parts and postpone the
    computation of the ray direction until the ray marching process.
    Each position stored is computed to be in the so-called "texture space",
    which is a space where the origin is the bottom left corner of the
    fluid volume and the axes are aligned with the axes of the fluid volume.
    The texture space is used to avoid the need to transform the position
    of the ray into the 3D texture space during the ray marching process.
    The transformation from the world space to the texture space can be done
    thanks to the assumption of the fluid volume being a cube with vertices
    in the range [-1, 1], so once the position of the ray is transformed into
    the fluid volume local space, it can be transformed into the texture space
    by simply adding 1 to each component and dividing by 2. Two final steps for
    the transformation is to invert the z component, because the z axis of the
    texture space is inverted with respect to the z axis of the world space, and
    to clamp the position to the range [0.5, grid_size - 0.5], because the 
    texture space is a discrete space, so the position of the ray must be
    clamped to the nearest voxel center. 
    To handle the case where the camera is inside the fluid volume, we store
    in the w component of the texture the negate depth of the position in the 
    raydata back and we copy this value when creating the raydata front, so that
    during the ray marching process we can check if the ray is inside the fluid
    volume or not just by checking if the w component is negative or zero after
    the subtraction of the two texture values.

    Also, the ray data back texture is useful to determine the depth of the 
    fluid volume back faces depth, which is used during the final scene
    blending process.
    In the end, to handle the case where obstacles are inside the fluid volume,
    we check if the scene depth is less than the depth of the ray, and if it is,
    we use that depth to approximate the position of the obstacle along the ray,
    and use that position as the end of the ray.

    This shader is used to create the raydata texture for the back faces,
    and is composed by the following shaders:
    - Vertex Shader: raydata.vert
    - Fragment Shader: this shader

*/

#version 410 core

out vec4 fragColor; // output color

uniform vec3 grid_size; // Number of voxels in each dimension

uniform sampler2D SceneDepthTexture; // Scene depth texture

uniform vec2 InverseSize; // Inverse of the size of the viewport

in vec3 texPos; // Position of the vertex in the texture space

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// Apply the texture space clamping to the given position
// in order to obtain the coordinates in the discrete space
vec3 TextureVoxelClamp(vec3 pos)
{
    pos *= (grid_size - 1.0);
    pos += 0.5;
    pos /= grid_size;
    return pos;
}

// Main function
void main()
{
    // Compute the ray's end position in the texture space
    vec3 end = TextureVoxelClamp(texPos);

    // Read the scene depth
    float sceneDepth = texture(SceneDepthTexture, InverseSize * gl_FragCoord.xy).r;

    // Check if some obstacle is in front of the fluid volume
    // back face, and if it is, use that depth to approximate
    // the position of the obstacle along the ray
    if (sceneDepth < gl_FragCoord.z)
    {
        // Compute the obstacle position in screen space
        vec3 scenePos = 2 * vec3(InverseSize * gl_FragCoord.xy, sceneDepth) - 1.0;
        
        // Compute the obstacle position in the world space
        vec4 localScenePos = inverse(projection * view) * vec4(scenePos, 1.0);
        localScenePos /= localScenePos.w;

        // Compute the obstacle position in the fluid volume local space
        localScenePos = localScenePos * transpose(inverse(model));

        // Compute the obstacle position in the texture space
        scenePos = localScenePos.xyz;
        scenePos = (scenePos + 1.0) / 2.0;
        scenePos.z = 1 - scenePos.z;

        // Clamp the position to the nearest voxel center
        end = TextureVoxelClamp(scenePos);
    }
 
    // Store the end position and the negate depth of the position
    fragColor = vec4(end, - gl_FragCoord.z);
}