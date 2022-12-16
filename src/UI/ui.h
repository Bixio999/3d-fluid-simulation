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

// parameters for simulation time step
extern GLfloat timeStep = 0.25f; // 0.25f
extern GLfloat simulationFramerate = 1.0f / 60.0f;

// we define the target for fluid simulation
// TargetFluid targetFluid = LIQUID;
extern TargetFluid targetFluid = GAS;

// Level Set parameters
extern GLfloat levelSetDampingFactor = 0.2f; // 0.2f

extern GLfloat levelSetEquilibriumHeight = 0.4f;
extern GLfloat levelSetInitialHeight = 0.4f;

// Liquid parameters
extern GLfloat gravityAcceleration = 3.0f;
extern GLfloat gravityLevelSetThreshold = 1.0f;

// Jacobi pressure solver iterations
extern GLuint pressureIterations = 40; // 40

// Buoyancy parameters
extern GLfloat ambientTemperature = 0.0f;
extern GLfloat ambientBuoyancy = 0.9f;
extern GLfloat ambientWeight = 0.15f;

// Dissipation factors
extern GLfloat velocityDissipation = 0.99f; // 0.8f
extern GLfloat densityDissipation = 0.99f; // 0.9f
extern GLfloat temperatureDissipation = 0.9f; // 0.9f

// Fluid Volume parameters
extern glm::vec3 fluidTranslation = glm::vec3(0.0f, 2.0f, 1.0f);
extern GLfloat fluidScale = 2.0f;

// Blur filter parameters
extern GLfloat blurRadius = 1.0f;

// DeNoise filter parameters
extern GLfloat deNoiseSigma = 7.0f;
extern GLfloat deNoiseThreshold = 0.23f;
extern GLfloat deNoiseSlider = 0.0f;
extern GLfloat deNoiseKSigma = 3.0f;

// rotation angle on Y axis
extern GLfloat orientationY = 0.0f;
// rotation speed on Y axis
extern GLfloat spin_speed = 60.0f;

/////////////////////////

void DrawUI();

void InitUI(GLFWwindow* window);

void RenderUI();