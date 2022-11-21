#version 410 core

out vec4 FragColor;

uniform sampler2D StencilTexture;
uniform vec2 InverseSize;

void main()
{
    vec2 texCoord = gl_FragCoord.xy * InverseSize;
    float stencil = texture(StencilTexture, texCoord).r;
    FragColor = vec4(stencil, stencil, stencil, 1.0);
}