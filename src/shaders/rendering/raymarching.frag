#version 410 core

out vec4 FragColor;

uniform vec2 InverseSize;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 grid_size;

uniform float nearPlane;
uniform vec3 eyePos;
uniform vec3 cameraFront;

uniform sampler2D RayDataBack;
uniform sampler2D RayDataFront;

uniform sampler3D DensityTexture;

in vec4 mvpPos;
in vec3 ogPos;
in vec3 texPos;

vec3 NearPlaneIntersection(vec3 eyePos, vec3 end)
{
    vec3 p0 = eyePos + cameraFront * nearPlane;

    vec3 dir = normalize(end - eyePos);

    float b = dot(dir, cameraFront);
    // if (b == 0) return vec3(0, 0, 0);

    float a = dot(p0 - eyePos, cameraFront);
    // if (a == 0) return vec3(0, 0, 0);

    float t = a / b;

    vec3 p = eyePos + dir * t;

    return p;
    // return eyePos + dir * t;
}

vec3 TextureVoxelClamp(vec3 pos)
{
    pos *= (grid_size - 1.0);
    pos += 0.5;
    pos /= grid_size;
    return pos;
}

void main()
{
    vec4 rd_back = texture(RayDataBack, gl_FragCoord.xy * InverseSize);
    vec4 rd_front = texture(RayDataFront, gl_FragCoord.xy * InverseSize);

    if (rd_front.x < 0.0)
        discard;

    vec4 raydata = rd_back - rd_front;

    vec3 start, dir;
    float fragDepth = gl_FragCoord.z;
    if (raydata.w < 0.0)
    {
        vec3 mOgPos = (model * vec4(ogPos, 1.0)).xyz;
        start = NearPlaneIntersection(eyePos, mOgPos);

        vec4 mvpNearPlane = projection * view * vec4(start, 1.0);
        mvpNearPlane /= mvpNearPlane.w;
        fragDepth = mvpNearPlane.z;
        // gl_FragDepth = mvpNearPlane.z * 0.5 + 0.5;

        start = (vec4(start, 1.0) * transpose(inverse(model))).xyz;

        start = (start + 1.0) / 2.0;
        start.z = 1.0 - start.z;
        start = clamp(start, 0.0, 1.0);
        start = TextureVoxelClamp(start);

        dir = raydata.xyz - start;

        // FragColor = vec4(start, 1.0); 
    }
    else
    {
        if (!gl_FrontFacing)
            discard;

        start = TextureVoxelClamp(texPos);

        dir = raydata.xyz;

        // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        // FragColor = vec4(start, 1.0);   
    }

    gl_FragDepth = fragDepth;

    // FragColor = vec4(dir, 1.0);

    vec3 fluidColor = vec3(0.8,0.8, 0.8);
    vec4 finalColor = vec4(0,0,0,0);

    // FragColor = vec4(dir, 1);

    float t = 0.5 / grid_size.x;

    float sampledDensity = 0;
    for(float i = 0; i <= 1.0; i += t)
    {
        vec3 p = start + dir * i;

        sampledDensity = texture(DensityTexture, p).x;
        finalColor.xyz += fluidColor * sampledDensity * (1.0 - finalColor.w);
        finalColor.w += sampledDensity * (1.0 - finalColor.w);

        if (finalColor.w > 0.99)
            break;
    }

    if (finalColor.w < 0.01)
        discard;
        // FragColor = vec4(0,0,0,0);
    else
        FragColor = finalColor;

}