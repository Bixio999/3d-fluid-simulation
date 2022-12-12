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

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
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

void main()
{
    vec4 rd_back = texture(RayDataBack, gl_FragCoord.xy * InverseSize);
    vec4 rd_front = texture(RayDataFront, gl_FragCoord.xy * InverseSize);

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

    vec3 fluidColor = vec3(0.8,0.8, 0.8);
    vec4 finalColor = vec4(0,0,0,0);

    // FragColor = vec4(dir, 1);

    vec3 sampleInterval = 0.5 / grid_size;
    float t = dot(sampleInterval, abs(dir)) / (length(dir) * length(dir));
    t /= 2.0;
    // float t = 0.5 / grid_size.x;

    float sampledDensity = 0;
    for(float i = 0.5 * t * rand(gl_FragCoord.xy); i <= 1.0; i += t)
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