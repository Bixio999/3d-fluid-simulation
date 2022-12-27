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

// we load the structure for the dynamic objects
#include "../obstacle_object.h"

/////////////////////////////////////////////
// we define the structures used in the gui

// structure for the supported fluid types
enum TargetFluid {
    GAS,
    LIQUID
};

// structure for the supported post processing effects in liquid simulation
enum LiquidEffect {
    NONE,
    BLUR,
    DENOISE
};

// structure to define a force
struct Force
{
    glm::vec3 position;
    glm::vec3 direction;
    GLfloat radius;
    GLfloat strength;
};

// structure to define a fluid quantity (gas or liquid)
struct FluidQuantity
{
    glm::vec3 position;
    GLfloat radius;
};

// default parameters for the simulation grid
const GLuint GRID_WIDTH = 100, GRID_HEIGHT = 100, GRID_DEPTH = 100;

/////////////////////////////////////////////
// we define the parameters controlled by the gui and used in main application

// parameters for simulation time step
extern GLfloat timeStep; // time step used for simulation
extern GLfloat simulationFramerate; // framerate used for simulation (how many times the simulation is updated per second)

// we define the target for fluid simulation
extern TargetFluid targetFluid;

// we define the post-process effect for liquid
extern LiquidEffect liquidEffect;

// Level Set parameters
extern GLfloat levelSetDampingFactor; // damping factor for level set

extern GLfloat levelSetEquilibriumHeight; // equilibrium height for level set
extern GLfloat levelSetInitialHeight; // initial height for level set

// Liquid parameters
extern GLfloat gravityAcceleration; // gravity acceleration for liquid
extern GLfloat gravityLevelSetThreshold; // level set threshold for gravity application in liquid

// Jacobi pressure solver iterations
extern GLuint pressureIterations;

// Buoyancy parameters
extern GLfloat ambientTemperature; // ambient temperature for buoyancy
extern GLfloat dampingBuoyancy; // force damping for buoyancy
extern GLfloat ambientWeight; // weight of gas for buoyancy

// Dissipation factors
extern GLfloat velocityDissipation; // velocity dissipation factor
extern GLfloat densityDissipation; // density dissipation factor
extern GLfloat temperatureDissipation; // temperature dissipation factor

// Fluid Volume parameters
extern glm::vec3 fluidTranslation; // translation of fluid volume
extern GLfloat fluidScale; // scale of fluid volume

// Blur filter parameters
extern GLfloat blurRadius; // radius of blur filter

// DeNoise filter parameters
extern GLfloat deNoiseSigma; 
extern GLfloat deNoiseThreshold;
extern GLfloat deNoiseKSigma;

// rotation speed on Y axis
extern GLfloat spin_speed;

// Custom fluid interaction
extern vector<Force*> externalForces;
extern vector<FluidQuantity*> fluidQuantities;

// data structure for user-defined obstacle objects interaction
extern vector<ObstacleObject*> obstacleObjects;

/////////////////////////////////////////////
// we define the functions used to manage the gui

void DrawUI();

void InitUI(GLFWwindow* window);

void RenderUI();

void CollapseUI();

void ExpandUI();

void ResetParameters();

void ResetForcesAndDyes(TargetFluid target);

/////////////////////////////////////////////
// we define the functions used to manage the obstacle objects

// create an obstacle object with a high poly model for rendering and a low poly model for simulation
void CreateObstacleObject(const string& highPolyPath, const string& lowPolyPath, const char* name, glm::vec3 position = glm::vec3(0.0), glm::vec3 scale = glm::vec3(1.0));

// create an obstacle object with the same model for rendering and simulation
void CreateObstacleObject(const string& highPolyPath, const char* name, glm::vec3 position = glm::vec3(0.0), glm::vec3 scale = glm::vec3(1.0));