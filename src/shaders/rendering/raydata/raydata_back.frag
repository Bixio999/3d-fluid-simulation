#version 410 core

out vec4 fragColor;

uniform vec3 grid_size;

uniform sampler2D SceneDepthTexture;

uniform vec2 InverseSize;

in vec4 mvpPos;
in vec3 texPos;
in vec3 ogPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

vec3 TextureVoxelClamp(vec3 pos)
{
    pos *= (grid_size - 1.0);
    pos += 0.5;
    pos /= grid_size;
    return pos;
}


void main()
{
    // vec3 pos = ogPos * (grid_size - 1.0);
    // pos += 0.5;
    // pos /= grid_size;

    // float depth = gl_FragCoord.z * (grid_size.z - 1.0);
    // depth += 0.5;
    // depth /= grid_size.z;

    vec3 end = TextureVoxelClamp(texPos);

    float sceneDepth = texture(SceneDepthTexture, InverseSize * gl_FragCoord.xy).r;

    if (sceneDepth < gl_FragCoord.z)
    {
        vec3 scenePos = 2 * vec3(InverseSize * gl_FragCoord.xy, sceneDepth) - 1.0;
        
        vec4 localScenePos = inverse(projection * view) * vec4(scenePos, 1.0);
        localScenePos /= localScenePos.w;
        localScenePos = localScenePos * transpose(inverse(model));

        scenePos = localScenePos.xyz;

        scenePos = (scenePos + 1.0) / 2.0;
        scenePos.z = 1 - scenePos.z;
        // scenePos = clamp(scenePos, 0.0, 1.0);

        // scenePos = TextureVoxelClamp(scenePos);

        end = scenePos;
    }
 
    fragColor = vec4(end, - gl_FragCoord.z);
}