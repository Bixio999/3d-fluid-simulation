/*
    OpenGL 4.1 Core - Blending Fluid and Scene - Fragment Shader

    This shader is part of the program that blends the fluid and the scene
    together. It is used to render the final image.

    This blending is done by comparing the depth of the fluid and the scene
    in order to determine which one is in front of the other. The use of this
    shader is mandatory because of the possibility of objects being inside the
    fluid volume. Due to this case, the depth of the scene is compared with the
    depth of the fluid and the depth of the raydata, which is the depth of the
    back faces of the fluid volume. The depth of the fluid can be equal to the
    depth of the fluid volume front faces or the depth of points in the camera
    near plane, when the camera clips the fluid volume. 

    The Blending program is composed by the following shaders:
    - Vertex Shader: load_vertices.vert - generic shader used to load the
                     vertices of the full screen quads
    - Fragment Shader: this shader
*/

#version 410 core

out vec4 FragColor; // output color

// Scene
uniform sampler2D SceneTexture; 
uniform sampler2D SceneDepth; 

// Fluid
uniform sampler2D FluidTexture;
uniform sampler2D FluidDepth;

// RayData
uniform sampler2D RayDataDepth;

uniform vec2 InverseSize; // inverse of the screen size

void main()
{
    // get the color and depth of the scene and the fluid
    vec4 sceneColor = texture(SceneTexture, gl_FragCoord.xy * InverseSize);
    vec4 fluidColor = texture(FluidTexture, gl_FragCoord.xy * InverseSize);

    float sceneDepth = texture(SceneDepth, gl_FragCoord.xy * InverseSize).x;
    float fluidDepth = texture(FluidDepth, gl_FragCoord.xy * InverseSize).x;

    // get the depth of the raydata, which is the depth of the back faces of the fluid volume
    // negate the value because the depth is stored as a negative value
    float rayDataDepth = - texture(RayDataDepth, gl_FragCoord.xy * InverseSize).w;

    vec4 finalColor;
     
    // if the scene fragment is in front of the fluid and the raydata, then the scene color is used
    if (sceneDepth < rayDataDepth && sceneDepth < fluidDepth)
    {   
        float alpha = sceneColor.a;
        finalColor = vec4(sceneColor.rgb * alpha + fluidColor.rgb * (1 - alpha), alpha);
    }
    else // otherwise, the fluid color is used
    {
        float alpha = fluidColor.a;
        finalColor = vec4(fluidColor.rgb * alpha + sceneColor.rgb * (1 - alpha), alpha);
    }
    FragColor = finalColor; // output the final color
    gl_FragDepth = min(sceneDepth, fluidDepth); // set the depth of nearest fragment
}
