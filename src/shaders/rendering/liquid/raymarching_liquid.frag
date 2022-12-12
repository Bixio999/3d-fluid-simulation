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
uniform sampler3D ObstacleTexture;

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

float interpolate_tricubic_fast(sampler3D tex, vec3 coord)
{
	// shift the coordinate from [0,1] to [-0.5, nrOfVoxels-0.5]
	// vec3 nrOfVoxels = vec3(textureSize3D(tex, 0));
    vec3 nrOfVoxels = grid_size;
	vec3 coord_grid = coord * nrOfVoxels;
	vec3 index = floor(coord_grid);
	vec3 fraction = coord_grid - index;
	vec3 one_frac = 1.0 - fraction;

    // return one_frac;

	vec3 w0 = 1.0/6.0 * one_frac*one_frac*one_frac;
	vec3 w1 = 2.0/3.0 * fraction*fraction*(2.0-fraction);
	vec3 w2 = 2.0/3.0 * one_frac*one_frac*(2.0-one_frac);
	vec3 w3 = 1.0/6.0 * fraction*fraction*fraction;

	vec3 g0 = w0 + w1;
	vec3 g1 = w2 + w3;
	vec3 mult = 1.0 / nrOfVoxels;
	vec3 h0 = mult * ((w1 / g0) + index);  //h0 = w1/g0 - 1, move from [-0.5, nrOfVoxels-0.5] to [0,1]
	vec3 h1 = mult * ((w3 / g1) + index);  //h1 = w3/g1 + 1, move from [-0.5, nrOfVoxels-0.5] to [0,1]

    // return h0 - h1;

	// fetch the eight linear interpolations
	// weighting and fetching is interleaved for performance and stability reasons
	float tex000 = texture(tex, h0).r;
	float tex100 = texture(tex, vec3(h1.x, h0.y, h0.z)).r;
	tex000 = mix(tex100, tex000, g0.x);  //weigh along the x-direction
	float tex010 = texture(tex, vec3(h0.x, h1.y, h0.z)).r;
	float tex110 = texture(tex, vec3(h1.x, h1.y, h0.z)).r;
	tex010 = mix(tex110, tex010, g0.x);  //weigh along the x-direction
	tex000 = mix(tex010, tex000, g0.y);  //weigh along the y-direction
	float tex001 = texture(tex, vec3(h0.x, h0.y, h1.z)).r;
	float tex101 = texture(tex, vec3(h1.x, h0.y, h1.z)).r;
	tex001 = mix(tex101, tex001, g0.x);  //weigh along the x-direction
	float tex011 = texture(tex, vec3(h0.x, h1.y, h1.z)).r;
	float tex111 = texture(tex, h1).r;
	tex011 = mix(tex111, tex011, g0.x);  //weigh along the x-direction
	tex001 = mix(tex011, tex001, g0.y);  //weigh along the y-direction

	return mix(tex001, tex000, g0.z);  //weigh along the z-direction
}

vec3 ComputeGradient(vec3 pos)
{
    vec3 InverseGridSize = 1.0 / grid_size;

    float center = texture(LevelSetTexture, pos).r;

    float left = texture(LevelSetTexture, pos + vec3(-1, 0, 0) * InverseGridSize).r;
    float right = texture(LevelSetTexture, pos + vec3(1, 0, 0) * InverseGridSize).r;
    float bottom = texture(LevelSetTexture, pos + vec3(0, -1, 0) * InverseGridSize).r;
    float top = texture(LevelSetTexture, pos + vec3(0, 1, 0) * InverseGridSize).r;
    float back = texture(LevelSetTexture, pos + vec3(0, 0, -1) * InverseGridSize).r;
    float front = texture(LevelSetTexture, pos + vec3(0, 0, 1) * InverseGridSize).r;

    float obsLeft = texture(ObstacleTexture, pos + vec3(-1, 0, 0) * InverseGridSize).r;
    float obsRight = texture(ObstacleTexture, pos + vec3(1, 0, 0) * InverseGridSize).r;
    float obsBottom = texture(ObstacleTexture, pos + vec3(0, -1, 0) * InverseGridSize).r;
    float obsTop = texture(ObstacleTexture, pos + vec3(0, 1, 0) * InverseGridSize).r;
    float obsBack = texture(ObstacleTexture, pos + vec3(0, 0, -1) * InverseGridSize).r;
    float obsFront = texture(ObstacleTexture, pos + vec3(0, 0, 1) * InverseGridSize).r;

    if (obsLeft > 0.0) left = center;
    if (obsRight > 0.0) right = center;
    if (obsBottom > 0.0) bottom = center;
    if (obsTop > 0.0) top = center;
    if (obsBack > 0.0) back = center;
    if (obsFront > 0.0) front = center;

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
        if (NdotV * NdotL > 0.0)
            specular = (F * G2 * D) / (4.0 * NdotV * NdotL);
    }

    return (lambert + specular) * NdotL;
    // return lambert * NdotL;
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
    float stepSize = 0.1;

    // FragColor = vec4(dir, 1);

    if (length(dir) <= 0.0)
        discard;

    vec3 sampleInterval = 0.33 / grid_size;
    float t = dot(sampleInterval, abs(dir)) / (length(dir) * length(dir));
    // t /= 2.0;
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

        // curr = texture(LevelSetTexture, p).x;
        curr = interpolate_tricubic_fast(LevelSetTexture, p);

        if (curr < 0.0)
            alpha += stepSize;

        if (curr * prev < 0.0 && !surfaceFound)
        {
            surfaceFound = true;
            surface = p;
            // surface = vec3(i, curr, prev);
        }

        if (surfaceFound && alpha >= 0.8)
            break;
    }

    alpha = clamp(alpha, 0.0, 0.8);
    float lightingFactor = 0.6;

    if (alpha > 0.0)
    {
        vec3 surfaceNormal;
        if (surfaceFound)
            surfaceNormal = ComputeGradient(surface);
        else
        {
            surfaceNormal = - ComputeGradient(p);
            lightingFactor += lightingFactor * 0.8;
        }

        surfaceNormal.z *= -1.0;
        surfaceNormal = (view * vec4(surfaceNormal, 0.0)).xyz;

        vec2 refractionPos = gl_FragCoord.xy - vec2(dot(surfaceNormal, cameraRight), dot(surfaceNormal, cameraUp));

        surfaceNormal = normalize(surfaceNormal);

        // finalColor = vec4(surfaceNormal, 1.0);

        vec3 refractionColor = texture(BackgroundTexture, refractionPos * InverseScreenSize).xyz;
        // finalColor = vec4(refractionColor, 1.0);

        // fluidColor = ComputeLighting(fluidColor, surfaceNormal);

        fluidColor = fluidColor * lightingFactor + ComputeLighting(fluidColor, surfaceNormal) * (1 - lightingFactor);

        // finalColor = vec4(fluidColor, 1.0);

        vec3 color = fluidColor * alpha + refractionColor * (1.0 - alpha);

        finalColor = vec4(color, 1.0);
        // finalColor = vec4(interpolate_tricubic_fast(LevelSetTexture, surface), 0, 0, 1.0);
    }

    FragColor = finalColor;

}