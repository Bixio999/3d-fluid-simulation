#version 410 core

out vec4 FragColor;

uniform vec2 InverseScreenSize;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 grid_size;

uniform float nearPlane;
uniform vec3 eyePos;
uniform vec3 cameraFront;
uniform vec3 cameraUp;
uniform vec3 cameraRight;

uniform sampler2D RayDataBack;
uniform sampler2D RayDataFront;

uniform sampler2D BackgroundTexture;
uniform sampler3D LevelSetTexture;

in vec4 mvpPos;
in vec3 ogPos;
in vec3 texPos;

in vec3 lightDir;
in vec3 vViewPosition;

uniform float rugosity; // rugosity - 0 : smooth, 1: rough
uniform float F0; // fresnel reflectance at normal incidence
uniform float Kd; // weight of diffuse reflection
const float PI = 3.14159265359;


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

vec3 ComputeGradient(vec3 pos)
{
    vec3 InverseGridSize = 1.0 / grid_size;

    float left = texture(LevelSetTexture, pos + vec3(-1, 0, 0) * InverseGridSize).r;
    float right = texture(LevelSetTexture, pos + vec3(1, 0, 0) * InverseGridSize).r;
    float bottom = texture(LevelSetTexture, pos + vec3(0, -1, 0) * InverseGridSize).r;
    float top = texture(LevelSetTexture, pos + vec3(0, 1, 0) * InverseGridSize).r;
    float back = texture(LevelSetTexture, pos + vec3(0, 0, -1) * InverseGridSize).r;
    float front = texture(LevelSetTexture, pos + vec3(0, 0, 1) * InverseGridSize).r;

    vec3 gradient = 0.5 * vec3(right - left, top - bottom, front - back);

    return gradient;
}

// Schlick-GGX method for geometry obstruction (used by GGX model)
float G1(float angle, float alpha)
{
    // in case of Image Based Lighting, the k factor is different:
    // usually it is set as k=(alpha*alpha)/2
    float r = (alpha + 1.0);
    float k = (r*r) / 8.0;

    float num   = angle;
    float denom = angle * (1.0 - k) + k;

    return num / denom;
}

vec3 ComputeLighting(vec3 color, vec3 normal)
{
    vec3 L = normalize(lightDir);

    float NdotL = max(dot(normal, L), 0.0);
    vec3 lambert = (Kd * color) / PI;
    vec3 specular = vec3(0.0);

    if (NdotL > 0.0)
    {
        vec3 V = normalize(vViewPosition);
        vec3 H = normalize(L + V);

        float NdotH = max(dot(normal, H), 0.0);
        float NdotV = max(dot(normal, V), 0.0);
        float VdotH = max(dot(V, H), 0.0);
        float alpha_Squared = rugosity * rugosity;
        float NdotH_Squared = NdotH * NdotH;

        float G2 = G1(NdotV, rugosity) * G1(NdotL, rugosity);

        float D = alpha_Squared;
        float denom = NdotH_Squared * (alpha_Squared - 1.0) + 1.0;
        D = D / (PI * denom * denom);

        vec3 F = vec3(pow(1.0 - VdotH, 5.0));
        F *= (1.0 - F0);
        F += F0;

        // we put everything together for the specular component
        specular = (F * G2 * D) / (4.0 * NdotV * NdotL);
    }

    return (lambert + specular) * NdotL;
}

void main()
{
    vec4 rd_back = texture(RayDataBack, gl_FragCoord.xy * InverseScreenSize);
    vec4 rd_front = texture(RayDataFront, gl_FragCoord.xy * InverseScreenSize);

    // object in front of volume
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

    vec3 fluidColor = vec3(52.0,120.0, 198.0); //52	120	198	
    fluidColor /= 255.0;
    vec4 finalColor = vec4(0,0,0,0);

    float alpha = 0.0;
    float stepSize = 0.05;

    // FragColor = vec4(dir, 1);

    float t = 0.5 / grid_size.x;
    float i = t;
    vec3 p = start + dir * i;;

    float curr = texture(LevelSetTexture, p).x;
    float prev = curr; 
    i += t;
    bool surfaceFound = false;
    vec3 surface;

    for(; i <= 1.0 && length(p - start) <= length(dir); i += t)
    {
        p = start + dir * i;

        curr = texture(LevelSetTexture, p).x;

        if (curr < 0.0)
            alpha += stepSize;

        if (curr * prev < 0.0 && !surfaceFound)
        {
            surfaceFound = true;
            surface = vec3(i, curr, prev);
            // surface = p;
            // break;
        }
    }

    alpha = clamp(alpha, 0.0, 0.8);

    if (curr < 0.0 || surfaceFound)
    {
        vec3 surfaceNormal = ComputeGradient(surface);
        surfaceNormal = normalize(surfaceNormal);

        finalColor = vec4(surface, 1.0);

        // vec2 refractionPos = gl_FragCoord.xy - vec2(dot(surfaceNormal, cameraRight), dot(surfaceNormal, cameraUp));
        // vec3 refractionColor = texture(BackgroundTexture, refractionPos * InverseScreenSize).xyz;

        // vec3 color = fluidColor * alpha + refractionColor * (1.0 - alpha);

        // finalColor = vec4(ComputeLighting(color, surfaceNormal), 1.0);
    }

    FragColor = finalColor;

}