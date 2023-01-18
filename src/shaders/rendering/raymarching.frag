/*
    OpenGL 4.1 Core - Fluid Raymarching - Fragment Shader

    This shader is part of the program that renders the fluid through
    raymarching. It is called for each fragment of the fluid volume.

    The Raymarching algorithm is implemented in the subroutine
    "raymarch_func". The subroutine is called with the start and 
    direction of the ray. The subroutine returns the color of the
    fragment.

    Depending on the target fluid, the subroutine is called with
    different implementations. The implementations are defined in
    the "raymarching" subroutine, which is a uniform variable.
    The two implementations are "RaymarchingGas" and 
    "RaymarchingLiquid":
    - RaymarchingGas: Raymarches the gas volume by using the
      "DensityTexture" as density field. The color og the gas 
      depends on the density accumulated along the ray. When the
      accumulated density is greater than 1, the fragment is 
      considered to be entirely inside the gas volume and the 
      raymarching is stopped.
    - RaymarchingLiquid: Raymarches the liquid volume by using the
      "DensityTexture" as level set field. The color of the liquid
      depends on the quantity of liquid accumulated along the ray,
      as its alpha value. Also, the color of the liquid depends on
      the presence of the liquid surface along the ray: when the 
      ray intersects the liquid surface, the color of the fragment
      is shaded according to the GGX lighting model, using the
      approximated gradient of the level set field as normal vector.
      For rays that intersects the liquid surface, the refraction 
      color is computed by sampling the "BackgroundTexture" in a
      position corresponding to the refraction direction. 

    To handle the cases of camera inside the fluid volume, the 
    raymarching algorithm is divided in two parts: the first part
    calculates the start and end points of the ray, and the second
    part raymarches the volume from the start point along the 
    direction of the ray. In the first part, if the camera is
    inside the fluid volume, the front raydata texture sample is 
    equal to zero, meaning that the result of the subtraction 
    between back and front raydata is a negative value (the one
    stored in the back raydata texture). In this case, the start
    point of the ray is computed from the intersection of the ray
    from the camera position to the fragment, and the near plane
    of the camera. Otherwise, if both back and front raydata 
    textures were successfully sampled, the start point of the
    ray is stored in the front raydata texture, while the end
    point of the ray is stored in the back raydata texture; so
    the ray direction is computed directly from the two raydata
    subtraction.

    For the cases of obstacles in front of the fluid volume, the 
    front raydata texture is set to a negative value and the 
    raymarching algorithm is stopped and the fragment discarded.

    To improve the performance of the raymarching algorithm, the
    marching step is computed with a function that depends on the
    distance between the start and end points of the ray. This 
    allow to reduce the number of steps needed to raymarch the
    volume, and therefore to improve the performance of the
    algorithm. However, this fast raymarching function causes the
    so-called "banding" effect, which is a visible artifact in the
    rendered image. To reduce the effect of the banding, the
    start point of the ray is moved along the ray direction by a
    random small value, which is computed using the "rand"
    function. This function is a simple random number generator
    that uses the fragment position as seed. The random value is
    added to the start point of the ray, and the raymarching
    algorithm is performed on the new start point. This solves the
    banding effect, but introduces a new problem: the rendered
    image now has a "dithering" effect, which is caused by the
    random value added to the start point of the ray. To try
    to reduce the effect of the dithering, some optional 
    post-processing effects are applied to the rendered image. 
    The currently implemented post-processing effects are
    Blur and Denoise, and offer a good compromise between
    performance and image quality. The Blur effect seems to 
    be the most effective in reducing the dithering effect,
    without affecting the performance too much. The Denoise
    effect is more effective in some areas of the image, but
    the overhead of the denoising algorithm is too high to be
    used in real-time applications due to high latency in the
    rendering pipeline.

    In the end, to improve the quality of the liquid rendering,
    the sampling of the "DensityTexture" is performed using a
    tricubic interpolation, which is more accurate than the
    default trilinear interpolation.

    The Raymarching program is composed of the following shaders:
    - Vertex Shader: raydata.vert - Computes the texture space coords
      for the vertex, and pass the lighting parameters to the 
      fragment shader.
    - Fragment Shader: this shader

*/

#version 410 core

out vec4 FragColor; // output color

uniform vec2 InverseScreenSize; // inverse of the screen size

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 grid_size; // Number of voxels in each dimension

uniform float nearPlane; // near plane of the camera
uniform vec3 eyePos; // camera position
uniform vec3 cameraFront; // camera front vector
uniform vec3 cameraUp; // camera up vector 
uniform vec3 cameraRight; // camera right vector

// Raydata textures
uniform sampler2D RayDataBack; 
uniform sampler2D RayDataFront;

uniform sampler3D DensityTexture; // fluid texture
uniform sampler3D ObstacleTexture;
uniform sampler2D BackgroundTexture; 

in vec3 ogPos; // cube local space position
in vec3 texPos; // texture space position

// Lighting parameters
in vec3 lightDir; // light direction in view space
in vec3 vViewPosition; // fragment-camera vector in view space

uniform float rugosity; // rugosity - 0 : smooth, 1: rough
uniform float F0; // fresnel reflectance at normal incidence
uniform float Kd; // weight of diffuse reflection

const float PI = 3.14159265359;

///////////////////

// Raymarching subroutine declarations
subroutine vec4 raymarching(vec3 start, vec3 dir);
subroutine uniform raymarching raymarch_func;

///////////////////

// Computes the intersection of a ray with the near plane of the camera
vec3 NearPlaneIntersection(vec3 eyePos, vec3 end)
{
    // Calculate a point on the near plane of the camera
    vec3 p0 = eyePos + cameraFront * nearPlane;

    // Calculate the direction of the ray
    vec3 dir = normalize(end - eyePos);

    // Calculate the intersection of the ray with the near plane
    float b = dot(dir, cameraFront);
    float a = dot(p0 - eyePos, cameraFront);

    float t = a / b;

    vec3 p = eyePos + dir * t;
    return p;
}

// Returns a random value in the range [0,1] using the fragment 
// position as seed
float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

// Apply the texture space alignment to the given position
// in order to obtain the coordinates in the discrete space
vec3 TextureVoxelAlign(vec3 pos)
{
    pos *= (grid_size - 1.0);
    pos += 0.5;
    pos /= grid_size;
    return pos;
}

// Tricubic filtering function
float interpolate_tricubic_fast(sampler3D tex, vec3 coord)
{
    vec3 nrOfVoxels = grid_size;
	vec3 coord_grid = coord * nrOfVoxels;
	vec3 index = floor(coord_grid);
	vec3 fraction = coord_grid - index;
	vec3 one_frac = 1.0 - fraction;

	vec3 w0 = 1.0/6.0 * one_frac*one_frac*one_frac;
	vec3 w1 = 2.0/3.0 * fraction*fraction*(2.0-fraction);
	vec3 w2 = 2.0/3.0 * one_frac*one_frac*(2.0-one_frac);
	vec3 w3 = 1.0/6.0 * fraction*fraction*fraction;

	vec3 g0 = w0 + w1;
	vec3 g1 = w2 + w3;
	vec3 mult = 1.0 / nrOfVoxels;
	vec3 h0 = mult * ((w1 / g0) + index);  //h0 = w1/g0 - 1, move from [0.5, nrOfVoxels-0.5] to [0,1]
	vec3 h1 = mult * ((w3 / g1) + index);  //h1 = w3/g1 + 1, move from [0.5, nrOfVoxels-0.5] to [0,1]

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

///////////////////

// Raymarching for gas 
subroutine(raymarching)
vec4 RaymarchingGas(vec3 start, vec3 dir)
{
    // Default color
    vec3 fluidColor = vec3(0.8,0.8, 0.8);

    // Final color
    vec4 finalColor = vec4(0,0,0,0);

    // Compute the marching step
    vec3 sampleInterval = 0.5 / grid_size;
    float t = dot(sampleInterval, abs(dir)) / (length(dir) * length(dir));
    t /= 2.0; // Half the step size thanks to easier computation

    // Raymarching; exit if the color is opaque; randomize the start position
    // along the ray to avoid banding artifacts
    float sampledDensity = 0;
    for(float i = 0.5 * t * rand(gl_FragCoord.xy); i <= 1.0; i += t)
    {
        // Compute the current position
        vec3 p = start + dir * i;

        // Sample the density
        sampledDensity = texture(DensityTexture, p).x;

        // Compute the color
        finalColor.xyz += fluidColor * sampledDensity * (1.0 - finalColor.w);

        // Update the opacity
        finalColor.w += sampledDensity * (1.0 - finalColor.w);

        // Exit if the color is opaque
        if (finalColor.w > 0.99)
            break;
    }

    // Return the final color
    if (finalColor.w < 0.01)
        return vec4(0.0);
    return vec4(finalColor);
}

///////////////////

// Compute the gradient of the level set field for the given position
// using central differences
vec3 ComputeGradient(vec3 pos)
{
    // Compute the inverse grid size
    vec3 InverseGridSize = 1.0 / grid_size;

    // Sample in all directions
    float center = texture(DensityTexture, pos).r;

    float left = texture(DensityTexture, pos + vec3(-1, 0, 0) * InverseGridSize).r;
    float right = texture(DensityTexture, pos + vec3(1, 0, 0) * InverseGridSize).r;
    float bottom = texture(DensityTexture, pos + vec3(0, -1, 0) * InverseGridSize).r;
    float top = texture(DensityTexture, pos + vec3(0, 1, 0) * InverseGridSize).r;
    float back = texture(DensityTexture, pos + vec3(0, 0, -1) * InverseGridSize).r;
    float front = texture(DensityTexture, pos + vec3(0, 0, 1) * InverseGridSize).r;

    // Sample the obstacle texture
    float obsLeft = texture(ObstacleTexture, pos + vec3(-1, 0, 0) * InverseGridSize).r;
    float obsRight = texture(ObstacleTexture, pos + vec3(1, 0, 0) * InverseGridSize).r;
    float obsBottom = texture(ObstacleTexture, pos + vec3(0, -1, 0) * InverseGridSize).r;
    float obsTop = texture(ObstacleTexture, pos + vec3(0, 1, 0) * InverseGridSize).r;
    float obsBack = texture(ObstacleTexture, pos + vec3(0, 0, -1) * InverseGridSize).r;
    float obsFront = texture(ObstacleTexture, pos + vec3(0, 0, 1) * InverseGridSize).r;

    // If the neighbor is an obstacle, use the center value
    if (obsLeft > 0.0) left = center;
    if (obsRight > 0.0) right = center;
    if (obsBottom > 0.0) bottom = center;
    if (obsTop > 0.0) top = center;
    if (obsBack > 0.0) back = center;
    if (obsFront > 0.0) front = center;

    // Compute the gradient
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

// GGX model for lighting
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
        if (NdotV * NdotL > 0.0) // avoid division by zero
            specular = (F * G2 * D) / (4.0 * NdotV * NdotL);
    }

    return (lambert + specular) * NdotL;
}

// Raymarching for the liquid
subroutine(raymarching)
vec4 RaymarchingLiquid(vec3 start, vec3 dir)
{
    // Set the fluid color
    vec3 fluidColor = vec3(52.0,120.0, 198.0); //52	120	198	
    fluidColor /= 255.0;

    // Final color
    vec4 finalColor = vec4(0,0,0,0);

    float alpha = 0.0;
    // Increment for alpha blending
    float stepSize = 0.1;

    // Compute the marching step
    vec3 sampleInterval = 0.5 / grid_size;
    float t = dot(sampleInterval, abs(dir)) / (length(dir) * length(dir));

    // Randomize the starting point
    float i = t + t * rand(gl_FragCoord.xy);
    vec3 p = start + dir * i;

    // Sample the texture to get initial density
    float curr = texture(DensityTexture, p).x;
    float prev = curr; 
    i += t;

    // Keep track of the surface
    bool surfaceFound = false;
    vec3 surface;

    // Raymarching loop; stop when alpha is 0.8 and surface is found
    for(; i <= 1.0 && length(p - start) <= length(dir); i += t)
    {
        // Get the current position
        p = start + dir * i;

        // Sample the level set
        curr = interpolate_tricubic_fast(DensityTexture, p);

        // Increase the alpha if we are inside the fluid
        if (curr < 0.0)
            alpha += stepSize;

        // If the sign changes, we found the surface
        if (curr * prev < 0.0 && !surfaceFound)
        {
            surfaceFound = true;
            surface = p;
        }

        // Stop if we alreadny found the surface and alpha is >= 0.8
        if (surfaceFound && alpha >= 0.8)
            break;
    }

    // Clamp the alpha value to 0.8
    alpha = min(alpha, 0.8);

    // If we found liquid, compute the lighting
    if (alpha > 0.0)
    {
        // Merging factor for the lighting
        float lightingFactor = 0.6;

        // Compute the surface normal
        vec3 surfaceNormal;

        if (surfaceFound) // If we found the surface, use the gradient at the surface
            surfaceNormal = ComputeGradient(surface);
        else
        {
            // Otherwise, use the gradient at the last sampled point,
            // which is a point on a fluid volume face
            surfaceNormal = - ComputeGradient(p);

            // Increase the lighting factor to enhance the color 
            // for those pixels that are under the fluid surface
            lightingFactor += lightingFactor * 0.8;
        }

        // Adjust the normal to the view space
        surfaceNormal.z *= -1.0;
        surfaceNormal = (view * vec4(surfaceNormal, 0.0)).xyz;

        // Compute the refraction vector
        vec2 refractionPos = gl_FragCoord.xy - vec2(dot(surfaceNormal, cameraRight), dot(surfaceNormal, cameraUp));

        // Normalize the vector 
        surfaceNormal = normalize(surfaceNormal);

        // Sample the background texture
        vec3 refractionColor = texture(BackgroundTexture, refractionPos * InverseScreenSize).xyz;

        // Compute the lighting
        fluidColor = fluidColor * lightingFactor + ComputeLighting(fluidColor, surfaceNormal) * (1 - lightingFactor);

        // Blend the refraction and fluid colors
        alpha = max(alpha, 0.1);
        vec3 color = fluidColor * alpha + refractionColor * (1.0 - alpha);

        // Set the final color
        finalColor = vec4(color, 1.0);
    }
    return finalColor;
}

///////////////////

// Main function
void main()
{
    // Get the ray data
    vec4 rd_back = texture(RayDataBack, gl_FragCoord.xy * InverseScreenSize);
    vec4 rd_front = texture(RayDataFront, gl_FragCoord.xy * InverseScreenSize);

    // check for object in front of volume
    if (rd_front.x < 0.0)
        discard;

    // Compute the ray direction
    vec4 raydata = rd_back - rd_front;

    vec3 start, dir;
    float fragDepth = gl_FragCoord.z;

    // Check if the camera is inside the volume
    if (raydata.w < 0.0)
    {
        // Calculate the intersection point with the near plane
        vec3 mOgPos = (model * vec4(ogPos, 1.0)).xyz;
        start = NearPlaneIntersection(eyePos, mOgPos);

        // Convert the position to the clip space to get the depth value
        // of the near plane, which is used to set the fragment depth
        vec4 mvpNearPlane = projection * view * vec4(start, 1.0);
        mvpNearPlane /= mvpNearPlane.w;
        fragDepth = mvpNearPlane.z; 

        // Convert the position to the cube local space
        start = (vec4(start, 1.0) * transpose(inverse(model))).xyz;

        // Convert the position to the texture space
        start = (start + 1.0) / 2.0;
        start.z = 1.0 - start.z;
        start = clamp(start, 0.0, 1.0);

        // Apply the coordinate discretization
        start = TextureVoxelAlign(start);

        // Compute the ray direction
        dir = raydata.xyz - start;
    }
    else // The camera is outside the volume
    {
        // Discard the fragment if it is not a front-facing fragment
        if (!gl_FrontFacing)
            discard;

        // Compute the start position of the ray
        start = TextureVoxelAlign(texPos);

        // Compute the ray direction
        dir = raydata.xyz;
    }

    // Set the fragment depth
    gl_FragDepth = fragDepth;

    // Discard the fragment if the ray direction is an invalid value
    if (length(dir) <= 0.0)
        discard;

    // Compute the raymarching
    FragColor = raymarch_func(start, dir);
}