#include <glad/glad.h>

// extern GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// we load the extern GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// classes developed during lab lectures to manage shaders, to load models, and for FPS camera
#include <utils/shader.h>
#include <utils/model.h>

enum TargetFluid {
    GAS,
    LIQUID
};

enum LiquidEffect {
    NONE,
    BLUR,
    DENOISE
};

struct Force
{
    glm::vec3 position;
    glm::vec3 direction;
    GLfloat radius;
    GLfloat strength;
};

struct FluidQuantity
{
    glm::vec3 position;
    GLfloat radius;
};

const GLuint GRID_WIDTH = 100, GRID_HEIGHT = 100, GRID_DEPTH = 100;

//////////////////////////

// parameters for simulation time step
extern GLfloat timeStep;
extern GLfloat simulationFramerate;
// we define the target for fluid simulation
// TargetFluid targetFluid = LIQUID;
extern TargetFluid targetFluid;

// we define the post-process effect for liquid
extern LiquidEffect liquidEffect;

// Level Set parameters
extern GLfloat levelSetDampingFactor;

extern GLfloat levelSetEquilibriumHeight;
extern GLfloat levelSetInitialHeight;

// Liquid parameters
extern GLfloat gravityAcceleration;
extern GLfloat gravityLevelSetThreshold;

// Jacobi pressure solver iterations
extern GLuint pressureIterations;

// Buoyancy parameters
extern GLfloat ambientTemperature;
extern GLfloat ambientBuoyancy;
extern GLfloat ambientWeight;

// Dissipation factors
extern GLfloat velocityDissipation;
extern GLfloat densityDissipation;
extern GLfloat temperatureDissipation;

// Fluid Volume parameters
extern glm::vec3 fluidTranslation;
extern GLfloat fluidScale;

// Blur filter parameters
extern GLfloat blurRadius;

// DeNoise filter parameters
extern GLfloat deNoiseSigma;
extern GLfloat deNoiseThreshold;
extern GLfloat deNoiseSlider;
extern GLfloat deNoiseKSigma;

// rotation angle on Y axis
extern GLfloat orientationY;
// rotation speed on Y axis
extern GLfloat spin_speed;

// Custom fluid interaction
extern vector<Force*> externalForces;
extern vector<FluidQuantity*> fluidQuantities;

/////////////////////////

void DrawUI();

void InitUI(GLFWwindow* window);

void RenderUI();

void CollapseUI();

void ExpandUI();

void ResetParameters();

void ResetForcesAndDyes(TargetFluid target);