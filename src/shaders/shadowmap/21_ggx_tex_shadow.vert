/*
21_ggx_tex_shadow.vert: vertex shader for GGX illumination model, with shadow rendering using shadow map

N.B.) the shader considers only a directional light (simpler to manage for the creation of the shadow map). For more lights, of different kind, the shader must be modified to consider each case

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2021/2022
Master degree in Computer Science
Universita' degli Studi di Milano
*/

#version 410 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;
// UV coordinates
layout (location = 2) in vec2 UV;
// the numbers used for the location in the layout qualifier are the positions of the vertex attribute
// as defined in the Mesh class

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;

// normals transformation matrix (= transpose of the inverse of the model-view matrix)
uniform mat3 normalMatrix;

// transformation (projection and view) matrix for the light
uniform mat4 lightSpaceMatrix;

// direction of incoming light is passed as an uniform
uniform vec3 lightVector;

// direction of incoming light in view coordinates
out vec3 lightDir;
// normals in view coordinates
out vec3 vNormal;

// in the fragment shader, we need to calculate also the reflection vector for each fragment
// to do this, we need to calculate in the vertex shader the view direction (in view coordinates) for each vertex, and to have it interpolated for each fragment by the rasterization stage
out vec3 vViewPosition;

// the output variable for UV coordinates
out vec2 interp_UV;

// for the correct rendering of the shadows, we need to calculate the vertex coordinates also in "light coordinates" (= using light as a camera)
out vec4 posLightSpace;


void main(){

  // vertex position in world coordinates
  vec4 mPosition = modelMatrix * vec4( position, 1.0 );
  // vertex position in camera coordinates
  vec4 mvPosition = viewMatrix * mPosition;

  // view direction, negated to have vector from the vertex to the camera
  vViewPosition = -mvPosition.xyz;

  // transformations are applied to the normal
  vNormal = normalize( normalMatrix * normal );

  // light incidence directions in view coordinate
  lightDir = vec3(viewMatrix  * vec4(lightVector, 0.0));

  // we apply the projection transformation
  gl_Position = projectionMatrix * mvPosition;

  // I assign the values to a variable with "out" qualifier so to use the per-fragment interpolated values in the Fragment shader
  interp_UV = UV;

  // vertex position in "light coordinates"
  posLightSpace = lightSpaceMatrix * mPosition;

}
