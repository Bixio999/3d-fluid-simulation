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
    
    In the end, to handle the case where obstacles are in front of the fluid
    volume front faces, we check if the depth of the scene is less than the
    depth of the fragment, and if it is, we set the position of the ray start
    as a negative value, so that the ray marching process will avoid to cast
    the ray.

    This shader is used to create the raydata texture for the front faces,
    and is composed by the following shaders:
    - Vertex Shader: raydata.vert
    - Fragment Shader: this shader

*/

#version 410 core

out vec4 fragColor; // output color

uniform vec3 grid_size; // Number of voxels in each direction

uniform sampler2D RayDataTexture; // Ray data back texture
uniform sampler2D SceneDepthTexture; // Scene depth texture

in vec3 texPos; // Position in texture space

uniform vec2 InverseSize; // Inverse of the size of the viewport

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
    // Get the ray data back value
    vec4 raydata = texture(RayDataTexture, InverseSize * gl_FragCoord.xy);

    // Get the depth of the scene
    float sceneDepth = texture(SceneDepthTexture, InverseSize * gl_FragCoord.xy).r;

    // Compute the position of the ray start in discrete texture space
    vec3 pos = TextureVoxelClamp(texPos);

    // If the depth of the scene is less than the depth of the fragment,
    // set the position of the ray start as a negative value
    if (sceneDepth < gl_FragCoord.z)
        pos = vec3(-1.0);

    // Set the position of the ray start in xyz component,
    // and the negate depth of the position in the raydata back 
    // in the w component
    fragColor = vec4(pos, raydata.w);
}