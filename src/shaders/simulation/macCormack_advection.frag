
#version 410 core

out vec4 FragColor;

uniform sampler3D VelocityTexture;
uniform sampler3D SourceTexture;
uniform sampler3D Phi1HatTexture;
uniform sampler3D Phi2HatTexture;

uniform float timeStep;
uniform vec3 InverseSize;
uniform float dissipation;

in float layer;

void main()
{
    vec3 fragCoord = vec3(gl_FragCoord.xy, layer);

    vec3 u = texture(VelocityTexture, InverseSize * fragCoord).xyz;

    vec3 coord = (fragCoord - timeStep * u);

    coord = floor(coord + vec3(0.5));

    vec3 neighborhood[8];

    neighborhood[0] = texture(SourceTexture, InverseSize * (coord + vec3(-0.5, -0.5, -0.5))).xyz;
    neighborhood[1] = texture(SourceTexture, InverseSize * (coord + vec3(-0.5, -0.5,  0.5))).xyz;
    neighborhood[2] = texture(SourceTexture, InverseSize * (coord + vec3(-0.5,  0.5, -0.5))).xyz;
    neighborhood[3] = texture(SourceTexture, InverseSize * (coord + vec3(-0.5,  0.5,  0.5))).xyz;
    neighborhood[4] = texture(SourceTexture, InverseSize * (coord + vec3( 0.5, -0.5, -0.5))).xyz;
    neighborhood[5] = texture(SourceTexture, InverseSize * (coord + vec3( 0.5, -0.5,  0.5))).xyz;
    neighborhood[6] = texture(SourceTexture, InverseSize * (coord + vec3( 0.5,  0.5, -0.5))).xyz;
    neighborhood[7] = texture(SourceTexture, InverseSize * (coord + vec3( 0.5,  0.5,  0.5))).xyz;

    vec3 minVal = neighborhood[0];
    vec3 maxVal = neighborhood[0];

    for (int i = 1; i < 8; i++)
    {
        minVal = min(minVal, neighborhood[i]);
        maxVal = max(maxVal, neighborhood[i]);
    }

    vec3 phi1_hat = texture(Phi1HatTexture, InverseSize * fragCoord).xyz;
    vec3 phi2_hat = texture(Phi2HatTexture, InverseSize * fragCoord).xyz;
    vec3 val = texture(SourceTexture, InverseSize * fragCoord).xyz;

    // FragColor = vec4(val - phi2_hat, 1.0);

    vec3 newVal = phi1_hat + 0.5 * (val - phi2_hat);

    newVal = clamp(newVal, minVal, maxVal);

    FragColor = vec4(newVal, 1.0);

    if (length(FragColor) < 0.0001)
        FragColor = vec4(0.0);
}