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

struct Slab
{
    GLuint fbo;
    GLuint tex;
};

// create a simulation grid slab
Slab CreateSlab(GLuint width, GLuint height, GLuint depth, GLushort dimensions);

// create slab with a 2d texture
Slab Create2DSlab(GLuint width, GLuint height, GLushort dimensions);

// swap the simulation grid slabs
void SwapSlabs(Slab *slabA, Slab *slabB);

// define the simulation grid size
void SetGridSize(GLuint width, GLuint height, GLuint depth);

// initialize shaders and data structures
void InitSimulation();

// setup vars to start simulation phase
void BeginSimulation();

// reset state to end simulation phase
void EndSimulation();

///////////////////////////////////////////

// execute advection 
void Advect(Shader* advectionShader, Slab *velocity, Slab *source, Slab *dest, float dissipation, float timeStep);

// execute buoyancy
void Buoyancy(Shader* buoyancyShader, Slab *velocity, Slab *temperature, Slab *density, Slab *dest, float ambientTemperature, float timeStep, float sigma, float kappa);

// execute divergence
void Divergence(Shader* divergenceShader, Slab *velocity, Slab *divergence, Slab *dest);

// execute jacobi
void Jacobi(Shader* jacobiShader, Slab *pressure, Slab *divergence, Slab *dest, GLuint iterations);

// apply external forces
void ApplyExternalForces(Shader* externalForcesShader, Slab *velocity, Slab *dest, float timeStep, glm::vec3 force, glm::vec3 position, float radius);

// add density
void AddDensity(Shader* dyeShader, Slab *density, Slab *dest, glm::vec3 position, float radius, float color);

// add temperature
void AddTemperature(Shader *dyeShader, Slab *temperature, Slab *dest, glm::vec3 position, float radius, float appliedTemperature);

// apply pressure
void ApplyPressure(Shader* pressureShader, Slab *velocity, Slab *pressure, Slab *dest);

/////////////////////////////////////////////

// render back faces of the cube
void RayData(Shader* raydataShader, Model &cubeModel, Slab *raydata, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection);

// render front faces of the cube
void RenderFluid(Shader* renderShader, Model &cubeModel, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection, Slab *raydata, Slab *density, glm::vec2 inverseScreenSize);