#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// classes developed during lab lectures to manage shaders, to load models, and for FPS camera
#include <utils/shader.h>
#include <utils/model.h>

// we load the structure for the dynamic objects
#include "obstacle_object.h"

/////////////////////////////////////////////
// we define the structures for the simulation

// structure for textures used in simulation
struct Slab
{
    GLuint fbo;
    GLuint tex;
};

// structure for scene texture used to compose the final image
struct Scene
{
    GLuint fbo;
    GLuint colorTex;
    GLuint depthTex;
};

// structure for obstacle texture
struct ObstacleSlab
{
    GLuint fbo;
    GLuint tex;
    GLuint depthStencil;

    // additional fbos used to draw in the first and last layer of the obstacle texture
    GLuint firstLayerFBO;
    GLuint lastLayerFBO;
};

/////////////////////////////////////////////
// we define the utility functions for the simulation 

// create a simulation grid slab
Slab CreateSlab(GLuint width, GLuint height, GLuint depth, GLushort dimensions);

// create slab with a 2d texture
Slab Create2DSlab(GLuint width, GLuint height, GLushort dimensions, bool filter);

// destroy a simulation grid slab
void DestroySlab(Slab &slab);

// clear the given simulation grid slabss
void ClearSlabs(int nSlabs, ...);

// create a scene
Scene CreateScene(GLuint width, GLuint height);

// swap the simulation grid slabs 
void SwapSlabs(Slab &slabA, Slab &slabB);

// define the simulation grid size
void SetGridSize(GLuint width, GLuint height, GLuint depth);

// initialize data structures
void InitSimulationVAOs();

// setup vars and state to begin simulation phase
void BeginSimulation();

// reset state to end simulation phase
void EndSimulation();

///////////////////////////////////////////
// we define the simulation functions

// execute advection with semi-lagrangian scheme
void Advect(Shader &advectionShader, Slab &velocity, ObstacleSlab &obstacle, Slab &source, Slab &dest, float dissipation, float timeStep);

// execute advection with mac-cormack scheme
void AdvectMacCormack(Shader &advectionShader, Shader &macCormackShader, Slab &velocity, Slab &phi1_hat, Slab &phi2_hat, ObstacleSlab &obstacle, Slab &source, Slab &dest, float dissipation, float timeStep);

// execute buoyancy
void Buoyancy(Shader &buoyancyShader, Slab &velocity, Slab &temperature, Slab &density, Slab &dest, float ambientTemperature, float timeStep, float sigma, float kappa);

// execute divergence
void Divergence(Shader &divergenceShader, Slab &velocity, Slab &divergence, ObstacleSlab &obstacle, Slab &obstacleVelocity, Slab &dest);

// execute jacobi
void Jacobi(Shader &jacobiShader, Slab &pressure, Slab &divergence, ObstacleSlab &obstacle, Slab &dest, GLuint iterations);

// apply external forces
void ApplyExternalForces(Shader &externalForcesShader, Slab &velocity, Slab &dest, float timeStep, glm::vec3 force, glm::vec3 position, float radius);

// add density
void AddDensity(Shader &dyeShader, Slab &density, Slab &dest, glm::vec3 position, float radius, float color, GLboolean isLiquidSimulation);

// apply pressure
void ApplyPressure(Shader &pressureShader, Slab &velocity, Slab &pressure, ObstacleSlab &obstacle, Slab &obstacleVelocity, Slab &dest);

/////////////////////////////////////////////
// we define the gas-exlusive simulation functions

// add temperature
void AddTemperature(Shader &dyeShader, Slab &temperature, Slab &dest, glm::vec3 position, float radius, float appliedTemperature);

/////////////////////////////////////////////
// we define the liquid-exclusive simulation functions

// initialize the simulation
void InitLiquidSimulation(Shader &initLiquidSimShader, Slab &levelSet, GLfloat initialHeight = 0.5f);

// update the level set
void ApplyLevelSetDamping(Shader &dampingLevelSetShader, Slab &levelSet, ObstacleSlab obstacle, Slab &dest, GLfloat dampingFactor, GLfloat equilibriumHeight = 0.5f);

// update the velocity with gravity
void ApplyGravity(Shader &gravityShader, Slab &velocity, Slab &levelSet, Slab &dest, GLfloat gravityAcceleration, GLfloat timeStep, GLfloat threshold = 0.0f);

/////////////////////////////////////////////
// we define the fluid rendering functions

// generate the raydata texture for the raymarching 
void RayData(Shader &backShader, Shader &frontShader, Model &cubeModel, Slab &back, Slab &front, Scene &scene, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection, glm::vec2 inverseScreenSize);

// render the fluid using the raycasting technique
void RenderFluid(Shader &renderShader, Slab &density_slab, ObstacleSlab &obstacle, Slab &rayDataFront, Slab &rayDataBack, Scene &backgroudScene, Scene &dest, Model &cubeModel, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection, glm::vec2 inverseScreenSize, GLfloat nearPlane, glm::vec3 eyePosition, glm::vec3 cameraFront, glm::vec3 cameraUp, glm::vec3 cameraRight, glm::vec3 lightDirection, GLfloat Kd, GLfloat rugosity, GLfloat F0, GLboolean isLiquidSimulation);

// compose the final frame
void BlendRendering(Shader &blendingShader, Scene &scene, Scene &fluid, Slab &raydataBack, glm::vec2 inverseScreenSize);

/////////////////////////////////////////////
// post processing functions for the liquid rendering

// apply the blur effect to the given slab
void Blur(Shader &blurShader, Slab &source, Slab &dest, GLfloat radius, glm::vec2 inverseScreenSize);

// apply the denoise effect to the given slab
void DeNoise(Shader &deNoiseShader, Slab &source, Slab &dest, GLfloat sigma, GLfloat threshold, GLfloat kSigma, glm::vec2 inverseScreenSize);

/////////////////////////////////////////////
// we define the obstacle functions

// create the volume obstacle grid
ObstacleSlab CreateObstacleBuffer(GLuint width, GLuint height, GLuint depth);

// clear the obstacle position and velocity grid
void ClearObstacleBuffers(ObstacleSlab &obstaclePosition, Slab &obstacleVelocity);

// draw the borders of the obstacle grid
void BorderObstacle(Shader &borderObstacleShader, Shader &borderObstacleShaderLayered, ObstacleSlab &dest);

// draw a dynamic obstacle in the obstacle grid
void DynamicObstacle(Shader &stencilObstacleShader, Shader &obstacleVelocityShader, ObstacleSlab &obstacle_position, Slab &obstacle_velocity, Slab &temp_slab, ObstacleObject* obstacle, glm::vec3 translation, GLfloat scale, GLfloat deltaTime);

