#version 410 core

out vec4 FragColor;

uniform sampler2D SceneTexture;
uniform sampler2D SceneDepth;

uniform sampler2D FluidTexture;
uniform sampler2D FluidDepth;

uniform sampler2D RayDataDepth;

uniform vec2 InverseSize;

void main()
{
    vec3 sceneColor = texture(SceneTexture, gl_FragCoord.xy * InverseSize).xyz;

    float sceneDepth = texture(SceneDepth, gl_FragCoord.xy * InverseSize).x;
    float fluidDepth = texture(FluidDepth, gl_FragCoord.xy * InverseSize).x;
    float rayDataDepth = - texture(RayDataDepth, gl_FragCoord.xy * InverseSize).w;

    if (sceneDepth < rayDataDepth && sceneDepth < fluidDepth)
        FragColor = vec4(sceneColor, 1.0);
    else
    {
        vec4 fluidColor = texture(FluidTexture, gl_FragCoord.xy * InverseSize);
        
        float alpha = fluidColor.a;
        vec3 finalColor = fluidColor.rgb * alpha + sceneColor * (1 - alpha);
        FragColor = vec4(finalColor, 1.0);

    }
}
