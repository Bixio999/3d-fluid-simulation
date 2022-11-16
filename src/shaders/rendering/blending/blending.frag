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
    vec4 sceneColor = texture(SceneTexture, gl_FragCoord.xy * InverseSize);
    vec4 fluidColor = texture(FluidTexture, gl_FragCoord.xy * InverseSize);

    float sceneDepth = texture(SceneDepth, gl_FragCoord.xy * InverseSize).x;
    float fluidDepth = texture(FluidDepth, gl_FragCoord.xy * InverseSize).x;
    float rayDataDepth = - texture(RayDataDepth, gl_FragCoord.xy * InverseSize).w;

    vec4 finalColor;

    if (sceneDepth < rayDataDepth && sceneDepth < fluidDepth)
    {   
        float alpha = sceneColor.a;
        finalColor = vec4(sceneColor.rgb * alpha + fluidColor.rgb * (1 - alpha), alpha);
    }
    else
    {
        float alpha = fluidColor.a;
        finalColor = vec4(fluidColor.rgb * alpha + sceneColor.rgb * (1 - alpha), alpha);
    }
    FragColor = finalColor;
    gl_FragDepth = min(sceneDepth, fluidDepth);
}
