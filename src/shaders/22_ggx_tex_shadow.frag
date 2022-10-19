/*

22_ggx_tex_shadow.frag: fragment shader for GGX illumination model, with shadow rendering using shadow map

N.B. 1)  "21_ggx_tex_shadow.vert" must be used as vertex shader

N.B. 2) the shader considers only a directional light (simpler to manage for the creation of the shadow map). For more lights, of different kind, the shader must be modified to consider each case

N.B. 3)  the different effects are implemented using Shaders Subroutines

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2021/2022
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

const float PI = 3.14159265359;

// output shader variable
out vec4 colorFrag;

// light incidence directions (calculated in vertex shader, interpolated by rasterization)
in vec3 lightDir;
// the transformed normal has been calculated per-vertex in the vertex shader
in vec3 vNormal;
// vector from fragment to camera (in view coordinate)
in vec3 vViewPosition;

// interpolated texture coordinates
in vec2 interp_UV;

// for the correct rendering of the shadows, we need to calculate the vertex coordinates also in "light coordinates" (= using light as a camera)
in vec4 posLightSpace;

// texture repetitions
uniform float repeat;

// texture sampler
uniform sampler2D tex;
// texture sampler for the depth map
uniform sampler2D shadowMap;

uniform float alpha; // rugosity - 0 : smooth, 1: rough
uniform float F0; // fresnel reflectance at normal incidence
uniform float Kd; // weight of diffuse reflection

////////////////////////////////////////////////////////////////////

// the "type" of the Subroutine
subroutine float shadow_map();

// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform shadow_map Shadow_Calculation;

////////////////////////////////////////////////////////////////////

//////////////////////////////////////////
// it applies a very basic shadow mapping. The final result is heavily aliased (with a lot of "shadow acne"), and the areas outside the light frustum are rendered as in shadow
subroutine(shadow_map)
float Shadow_Acne() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // given the fragment position in light coordinates, we apply the perspective divide. Usually, perspective divide is applied in an automatic way to the coordinates saved in the gl_Position variable. In this case, the vertex position in light coordinates has been saved in a separate variable, so we need to do it manually
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    // after the perspective divide the values are in the range [-1,1]: we must convert them in [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    // we sample the shadow map, considering the closer depth value from the point of view of the light
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // we get the depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // Version 1: if the depth of the current fragment is greater than the depth in the shadow map, then the fragment is in shadow
    // -> A LOT OF ALIASING/SHADOW ACNE
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}
//////////////////////////////////////////

//////////////////////////////////////////
// it applies an adaptive bias to the depth test, in order to eliminate the "shadow acne"
subroutine(shadow_map)
float Shadow_Bias() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // given the fragment position in light coordinates, we apply the perspective divide. Usually, perspective divide is applied in an automatic way to the coordinates saved in the gl_Position variable. In this case, the vertex position in light coordinates has been saved in a separate variable, so we need to do it manually
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    // after the perspective divide the values are in the range [-1,1]: we must convert them in [0,1]
    projCoords = projCoords * 0.5 + 0.5;
    // we sample the shadow map, considering the closer depth value from the point of view of the light
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // we get the depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // Version 2: we calculate an adaptive bias to apply to the currentDepth value, to avoid the shadow acne effect.
    // the bias value is in the range [0.005,0.05]: the final value is calculated considering the angle between the normal and the direction of light
    vec3 normal = normalize(vNormal);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // if the depth (with bias) of the current fragment is greater than the depth in the shadow map, then the fragment is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

//////////////////////////////////////////
// it applies Percentage-Closer Filtering to smooth the shadow edged. Moreover, the rendering of the areas behind the far plane of the light frustum is corrected
subroutine(shadow_map)
float Shadow_PCF_Final() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // given the fragment position in light coordinates, we apply the perspective divide. Usually, perspective divide is applied in an automatic way to the coordinates saved in the gl_Position variable. In this case, the vertex position in light coordinates has been saved in a separate variable, so we need to do it manually
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    // after the perspective divide the values are in the range [-1,1]: we must convert them in [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    // we get the depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // we calculate an adaptive bias to apply to the currentDepth value, to avoid the shadow acne effect.
    // the bias value is in the range [0.005,0.05]: the final value is calculated considering the angle between the normal and the direction of light
    vec3 normal = normalize(vNormal);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // Version 3: we apply Percentage Close Filtering (PCF) to smooth shadow edges
    float shadow = 0.0;
    // we determine the texel dimension
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    // we sample the depth map considering the 3x3 neighbourhood of the current fragment, and we apply the same test of Version 2 to each sample
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            // we sample the depth map
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            // if the depth (with bias) of the current fragment is greater than the depth in the shadow map, then the fragment is in shadow. We add the result to the shadow variable
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    // we average the shadow result on the kernel size of the PCF
    shadow /= 9.0;

    // To avoid that the areas behind the far plane of the light frustum are considered in shadow, we set their shadow value to 0 (= in light)
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

//////////////////////////////////////////
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

///////////// MAIN ////////////////////////////////////////////////
void main()
{
    // we repeat the UVs and we sample the texture
    vec2 repeated_Uv = mod(interp_UV*repeat, 1.0);
    vec4 surfaceColor = texture(tex, repeated_Uv);

    // normalization of the per-fragment normal
    vec3 N = normalize(vNormal);
    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(lightDir.xyz);

    // cosine angle between direction of light and normal
    float NdotL = max(dot(N, L), 0.0);

    // diffusive (Lambert) reflection component
    vec3 lambert = (Kd*surfaceColor.rgb)/PI;

    // we initialize the specular component
    vec3 specular = vec3(0.0);

    // initialization of shadow value
    float shadow = 0.0;

    // if the cosine of the angle between direction of light and normal is positive, then I can calculate the specular component
    if(NdotL > 0.0)
    {
        // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
        vec3 V = normalize( vViewPosition );

        // half vector
        vec3 H = normalize(L + V);

        // we implement the components seen in the slides for a PBR BRDF
        // we calculate the cosines and parameters to be used in the different components
        float NdotH = max(dot(N, H), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float VdotH = max(dot(V, H), 0.0);
        float alpha_Squared = alpha * alpha;
        float NdotH_Squared = NdotH * NdotH;

        // Geometric factor G2
        float G2 = G1(NdotV, alpha)*G1(NdotL, alpha);

        // Rugosity D
        // GGX Distribution
        float D = alpha_Squared;
        float denom = (NdotH_Squared*(alpha_Squared-1.0)+1.0);
        D /= PI*denom*denom;

        // Fresnel reflectance F (approx Schlick)
        vec3 F = vec3(pow(1.0 - VdotH, 5.0));
        F *= (1.0 - F0);
        F += F0;

        // we put everything together for the specular component
        specular = (F * G2 * D) / (4.0 * NdotV * NdotL);

        // we calculate the shadow value for the fragment
        shadow = Shadow_Calculation();
    }

    // the rendering equation is:
    //integral of: BRDF * Li * (cosine angle between N and L)
    // BRDF in our case is: the sum of Lambert and GGX
    // Li is considered as equal to 1: light is white, and we have not applied attenuation. With colored lights, and with attenuation, the code must be modified and the Li factor must be multiplied to finalColor
    //We weight using the shadow value
    // N.B. ) shadow value = 1 -> fragment is in shadow
    //        shadow value = 0 -> fragment is in light
    // Therefore, we use (1-shadow) as weight to apply to the illumination model
    vec3 finalColor = (1.0 - shadow)*(lambert + specular)*NdotL;


    colorFrag = vec4(finalColor, 1.0);
}
