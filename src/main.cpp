/*
This file is the main code of the 3D Fluid Simulation project. This project is based on the paper 
"Real-Time Simulation and Rendering of 3D Fluids" by Keenan Crane, Ignacio Llamas and Sarah Tariq, 
published in GPU Gems 3 (2008). The paper is available at:
 https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-30-real-time-simulation-and-rendering-3d-fluids

author: Vincenzo Lombardo - the structure of the code in this file is based on the code 
developed by Davide Gadia for the course of Real-Time Graphics Programming, Universita' degli Studi di Milano

------------------------------------------------------------

Real-Time Graphics Programming - a.a. 2021/2022
Master degree in Computer Science
Universita' degli Studi di Milano

------------------------------------------------------------

In this file we define the main function of the application. The main function is responsible for the initialization of the application,
the main loop, and the cleanup of the application. The main loop is responsible for the rendering of the scene, the update of the simulation,
and the management of the user input. The main function is also responsible for the initialization, drawing and rendering calls of the GUI, 
through the functions defined in the file src/UI/ui.h.
*/

/*
OpenGL coordinate system (right-handed)
positive X axis points right
positive Y axis points up
positive Z axis points "outside" the screen


                              Y
                              |
                              |
                              |________X
                             /
                            /
                           /
                          Z
*/


// Std. Includes
#include <string>

// Loader for OpenGL extensions
// http://glad.dav1d.de/
// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

// classes developed during lab lectures to manage shaders, to load models, and for FPS camera
#include <utils/shader.h>
#include <utils/model.h>
#include <utils/camera.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// we include the library for images loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// we include the fluid simulation functions
#include "fluid-sim.h"

// we include the UI functions
#include "UI/ui.h"

///////////////////////// FUNCTIONS DECLARATION /////////////////////////

// callback functions for keyboard and mouse events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements();

// the name of the subroutines are searched in the shaders, and placed in the shaders vector (to allow shaders swapping)
void SetupShader(int shader_program);

// print on console the name of current shader subroutine
void PrintCurrentShader(int subroutine);

// in this application, we have isolated the scene models rendering using a function, which will be called in each rendering step
void RenderObjects(Shader &shader, Model &planeModel, GLint render_pass, GLuint depthMap);

// load image from disk and create an OpenGL texture
GLint LoadTexture(const char* path);

// Shaders and data structure initialization functions
void CreateRenderShader(TargetFluid target);
void CreateFluidShaders(TargetFluid target);

///////////////////////// GLOBAL VARIABLES /////////////////////////

// dimensions of application's window
GLuint screenWidth = 1200, screenHeight = 900;

// the rendering steps used in the application for objects shadow map rendering
enum render_passes{ SHADOWMAP, RENDER};
// index of the current shader subroutine (= 0 in the beginning)
GLuint current_subroutine = 2;
// a vector for all the shader subroutines names used and swapped in the application
vector<std::string> shaders;

// we initialize an array of booleans for each keyboard key
bool keys[1024];

// we need to store the previous mouse position to calculate the offset with the current frame
GLfloat lastX, lastY;
// when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
bool firstMouse = true;
// Gamemode parameters
GLboolean mouseUnlock = GL_FALSE;

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// parameters for simulation time step
GLfloat lastSimulationUpdate = 0.0f;

// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// View matrix: the camera moves, so we just set to indentity now
glm::mat4 view = glm::mat4(1.0f);

// Model and Normal transformation matrices for the fixed objects in the scene: we set to identity
glm::mat4 cubeModelMatrix = glm::mat4(1.0f); // model matrix for the fluid volume
glm::mat4 planeModelMatrix = glm::mat4(1.0f); // model matrix for the plane
glm::mat3 planeNormalMatrix = glm::mat3(1.0f);  // normal matrix for the plane

// we create a camera. We pass the initial position as a paramenter to the constructor. The last boolean tells if we want a camera "anchored" to the ground
Camera camera(glm::vec3(0.0f, 0.0f, 7.0f), GL_FALSE);

// we consider a directional light. We pass the direction of incoming light as an uniform to the shaders
glm::vec3 lightDir0 = glm::vec3(1.0f, 1.0f, 1.0f);

// weight for the diffusive component
GLfloat Kd = 3.0f;
// roughness index for GGX shader
GLfloat alpha = 0.2f;
// Fresnel reflectance at 0 degree (Schlik's approximation)
GLfloat F0 = 0.9f;

// vector for the textures IDs
vector<GLint> textureID;

// UV repetitions
GLfloat repeat = 1.0;

// rotation angle on Y axis
GLfloat orientationY;

// Target fluid exclusive shaders
Shader *buoyancyShader, *temperatureShader, *initLiquidShader, *dampingLevelSetShader, *gravityShader;
Shader *renderShader;

/////////////////// MAIN function ///////////////////////
int main()
{
    // Initialization of OpenGL context using GLFW
    glfwInit();
    // We set OpenGL specifications required for this application
    // In this case: 4.1 Core
    // If not supported by your graphics HW, the context will not be created and the application will close
    // N.B.) creating GLAD code to load extensions, try to take into account the specifications and any extensions you want to use,
    // in relation also to the values indicated in these GLFW commands
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // we set if the window is resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // we create the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "3D Fluid Simulation", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // we put in relation the window and the callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // we disable the mouse cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // we define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    //the "clear" color for the frame buffer
    glClearColor(0,0,0,0);

    // we set the user parameters to their default values
    ResetParameters();

    // we store the current and last target fluid to handle switching between them
    // and avoid value update during the simulation step  
    TargetFluid currTarget = targetFluid;
    TargetFluid prevTarget = currTarget;

    // output to console initial the target fluid
    if (currTarget == GAS)
        std::cout << "Target fluid: GAS" << std::endl;
    else
    {
        std::cout << "Target fluid: LIQUID" << std::endl;
        densityDissipation = 1.0f; // we set the density dissipation to 1.0f for the liquid
    
    }

    // we set the initial values for forces and emitters to default
    ResetForcesAndEmitters(currTarget);

    // we create the Shader Program for the creation of the shadow map
    Shader shadow_shader("src/shaders/19_shadowmap.vert", "src/shaders/20_shadowmap.frag");
    // we create the Shader Program used for objects (which presents different subroutines we can switch)
    Shader illumination_shader = Shader("src/shaders/21_ggx_tex_shadow.vert", "src/shaders/22_ggx_tex_shadow.frag");

    // we create the Shader Programs for fluid simulation
    Shader advectionShader = Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom"  ,"src/shaders/simulation/advection.frag");
    Shader macCormackShader = Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom"  ,"src/shaders/simulation/macCormack_advection.frag");
    Shader divergenceShader = Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom","src/shaders/simulation/divergence.frag");
    Shader jacobiShader = Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom","src/shaders/simulation/jacobi_pressure.frag");
    Shader externalForcesShader = Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom","src/shaders/simulation/apply_force.frag");
    Shader pressureShader = Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom","src/shaders/simulation/pressure_projection.frag");
    Shader dyeShader = Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom","src/shaders/simulation/add_dye.frag");
    Shader fillShader = Shader("src/shaders/rendering/load_proj_vertices.vert", "src/shaders/rendering/fill.frag");

    // we create the simulation Shader Programs for the requested target fluid
    CreateFluidShaders(currTarget);

    // we create the Shader Programs for solid-fluid interaction
    Shader borderObstacleShaderLayered = Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/obstacles/border.geom","src/shaders/obstacles/border.frag");
    Shader borderObstacleShader = Shader("src/shaders/simulation/load_vertices.vert","src/shaders/obstacles/border.frag");

    Shader stencilObstacleShader = Shader("src/shaders/obstacles/position/obstacle_position.vert", "src/shaders/simulation/set_layer.geom" , "src/shaders/rendering/fill.frag");
    Shader obstacleVelocityShader = Shader("src/shaders/obstacles/velocity/obstacle_velocity.vert", "src/shaders/obstacles/velocity/obstacle_velocity.geom", "src/shaders/obstacles/velocity/obstacle_velocity.frag");

    // we create the Shader Programs for fluid rendering
    Shader raydataBackShader = Shader("src/shaders/rendering/raydata/raydata.vert", "src/shaders/rendering/raydata/raydata_back.frag");
    Shader raydataFrontShader = Shader("src/shaders/rendering/raydata/raydata.vert", "src/shaders/rendering/raydata/raydata_front.frag");

    Shader blendingShader = Shader("src/shaders/rendering/blending/blending.vert", "src/shaders/rendering/blending/blending.frag");
    Shader blurShader = Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/rendering/blur.frag");
    Shader deNoiseShader = Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/rendering/glslSmartDeNoise/frag.glsl");

    // we create the rendering Shader Programs for the requested target fluid
    CreateRenderShader(currTarget);

    // we parse the Shader Program to search for the number and names of the subroutines.
    // the names are placed in the shaders vector
    SetupShader(illumination_shader.Program);
    // we print on console the name of the first subroutine used
    PrintCurrentShader(current_subroutine);

    // we load the images and store them in a vector
    textureID.push_back(LoadTexture("textures/UV_Grid_Sm.png")); // objects texture
    textureID.push_back(LoadTexture("textures/marble-chess.jpg")); // floor texture

    // we load the model(s) (code of Model class is in include/utils/model.h)
    Model planeModel("models/plane.obj"); // floor
    Model cubeModel("models/cube.obj"); // fluid volume

    /////////////////// CREATION OF OBSTACLES /////////////////////////////////////////////////////////////

    CreateObstacleObject("models/bunny_lp.obj", "bunny", glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.3f));

    CreateObstacleObject("models/sphere.obj", "sphere", glm::vec3(-5.0f, 1.0f, 1.0f), glm::vec3(1.0f));
    
    CreateObstacleObject("models/babyyoda.obj", "models/low-poly_babyyoda.obj", "baby yoda", glm::vec3(4.0f, 1.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.3f));

    /////////////////// CREATION OF BUFFERS FOR THE SIMULATION GRID /////////////////////////////////////////

    // we setup the simulation grid
    SetGridSize(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH);

    // we create the simulation buffers

    Slab velocity_slab = CreateSlab(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH, 3);
    std::cout << "Created velocity grid = {" << velocity_slab.fbo << " , " << velocity_slab.tex << "}" << std::endl;
    Slab pressure_slab = CreateSlab(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH, 1);
    std::cout << "Created pressure grid = {" << pressure_slab.fbo << " , " << pressure_slab.tex << "}" << std::endl;
    Slab divergence_slab = CreateSlab(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH, 1);
    std::cout << "Created divergence grid = {" << divergence_slab.fbo << " , " << divergence_slab.tex << "}" << std::endl;

    // advection buffers (macCormack)
    Slab phi1_hat_slab = CreateSlab(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH, 3);
    std::cout << "Created phi1_hat grid = {" << phi1_hat_slab.fbo << " , " << phi1_hat_slab.tex << "}" << std::endl;
    Slab phi2_hat_slab = CreateSlab(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH, 3);
    std::cout << "Created phi2_hat grid = {" << phi2_hat_slab.fbo << " , " << phi2_hat_slab.tex << "}" << std::endl;

    // we create a buffer representing the density for gas simulation or level set for liquid simulation
    Slab density_slab = CreateSlab(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH, 1);
    std::cout << "Created density grid = {" << density_slab.fbo << " , " << density_slab.tex << "}" << std::endl;

    // we create the buffers for the target fluid
    Slab temperature_slab;
    if (currTarget == GAS)
    {
        // gas simulation exclusive buffer
        temperature_slab = CreateSlab(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH, 1);
        std::cout << "Created temperature grid = {" << temperature_slab.fbo << " , " << temperature_slab.tex << "}" << std::endl;
    }

    /////////////////// CREATION OF TEMPORARY BUFFERS /////////////////////////////////////////

    Slab temp_velocity_slab = CreateSlab(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH, 3);
    std::cout << "Created temp velocity grid = {" << temp_velocity_slab.fbo << " , " << temp_velocity_slab.tex << "}" << std::endl;
    Slab temp_pressure_divergence_slab = CreateSlab(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH, 1);
    std::cout << "Created temp pressure divergence grid = {" << temp_pressure_divergence_slab.fbo << " , " << temp_pressure_divergence_slab.tex << "}" << std::endl;

    Slab temp_screenSize_slab = Create2DSlab(width, height, 4, false);
    std::cout << "Created temp screen size grid = {" << temp_screenSize_slab.fbo << " , " << temp_screenSize_slab.tex << "}" << std::endl;

    /////////////////// CREATION OF BUFFER FOR THE RAYDATA TEXTURE /////////////////////////////////////////

    Slab rayDataBack = Create2DSlab(width, height, 4, false);
    std::cout << "Created raydata back grid = {" << rayDataBack.fbo << " , " << rayDataBack.tex << "}" << std::endl;
    Slab rayDataFront = Create2DSlab(width, height, 4, false);
    std::cout << "Created raydata front grid = {" << rayDataFront.fbo << " , " << rayDataFront.tex << "}" << std::endl;
    
    /////////////////// CREATION OF BUFFERS AND DATA FOR OBSTACLES /////////////////////////////////////////

    ObstacleSlab obstacle_slab = CreateObstacleBuffer(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH);
    std::cout << "Created obstacle grid = {" << obstacle_slab.fbo << " , " << obstacle_slab.tex << " , " << obstacle_slab.depthStencil << " , " << obstacle_slab.firstLayerFBO << " , " << obstacle_slab.lastLayerFBO << "}" << std::endl;

    Slab obstacle_velocity_slab = CreateSlab(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH, 3);
    std::cout << "Created obstacle velocity grid = {" << obstacle_velocity_slab.fbo << " , " << obstacle_velocity_slab.tex << "}" << std::endl;

    /////////////////// CREATION OF BUFFER FOR THE DEPTH MAP - SHADOW MAP ///////////////////////////////////

    // buffer dimension: too large -> performance may slow down if we have many lights; too small -> strong aliasing
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    GLuint depthMapFBO;
    // we create a Frame Buffer Object: the first rendering step will render to this buffer, and not to the real frame buffer
    glGenFramebuffers(1, &depthMapFBO);
    // we create a texture for the depth map
    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    // in the texture, we will save only the depth data of the fragments. Thus, we specify that we need to render only depth in the first rendering step
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // we set to clamp the uv coordinates outside [0,1] to the color of the border
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // outside the area covered by the light frustum, everything is rendered in shadow (because we set GL_CLAMP_TO_BORDER)
    // thus, we set the texture border to white, so to render correctly everything not involved by the shadow map
    //*************
    GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // we bind the depth map FBO
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    // we set that we are not calculating nor saving color data
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /////////////////// CREATION OF SCENE BUFFERS /////////////////////////////////////////

    Scene scene = CreateScene(width, height);
    std::cout << "Created scene framebuffer = {" << scene.fbo << " , " << scene.colorTex << ", " << scene.depthTex << "}" << std::endl;

    Scene fluidScene = CreateScene(width, height);
    std::cout << "Created fluid scene framebuffer = {" << fluidScene.fbo << " , " << fluidScene.colorTex << ", " << fluidScene.depthTex << "}" << std::endl;

    ///////////////////////////////////////////////////////////////////

    // Create vertex objects for fluid simulation
    InitSimulationVAOs();

    // we call the init function in case of liquid simulation as default
    if (currTarget == LIQUID)
        InitLiquidSimulation(*initLiquidShader, density_slab, levelSetInitialHeight);

    // Create context for ImGui
    InitUI(window);

    // Projection matrix of the camera: FOV angle, aspect ratio, near and far planes
    GLfloat windowNearPlane = 0.1f;
    GLfloat windowFarPlane = 10000.0f;

    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, windowNearPlane, windowFarPlane);

    // Rendering loop: this code is executed at each frame
    while(!glfwWindowShouldClose(window))
    {
        // we determine the time passed from the beginning
        // and we calculate time difference between current frame rendering and the previous one
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // we check if the user has switched the target fluid
        if (prevTarget != currTarget)
        {
            std::cout << "Switching to " << (currTarget == GAS ? "gas" : "liquid") << " simulation" << std::endl;

            // Destroy old fluid context
            renderShader->Delete();

            if (prevTarget == GAS)
            {
                DestroySlab(&temperature_slab);

                temperatureShader->Delete();
                buoyancyShader->Delete();
            }
            else
            {
                initLiquidShader->Delete();
                dampingLevelSetShader->Delete();
                gravityShader->Delete();
            }
            
            // Reset all simulation slabs
            ClearSlabs(4, &velocity_slab, &pressure_slab, &divergence_slab, &density_slab);

            // Instantiate the new fluid shaders
            CreateRenderShader(currTarget);
            CreateFluidShaders(currTarget);

            if (currTarget == LIQUID)
            {
                densityDissipation = 1.0f;
                InitLiquidSimulation(*initLiquidShader, density_slab, levelSetInitialHeight);
            }
            else
            {
                densityDissipation = 0.99f;
                temperature_slab = CreateSlab(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH, 1);
            }

            ResetForcesAndEmitters(currTarget);
        }

        // Check is an I/O event is happening
        glfwPollEvents();
        // we apply FPS camera movements
        apply_camera_movements();
        
        // Draw the UI
        DrawUI();

        // we update the simulation based on the defined framerate
        if (currentFrame - lastSimulationUpdate >= simulationFramerate)
        {
            /////////////////// STEP 1 - UPDATE OBSTACLES  //////////////////////////////////////////////////////////////////////////

            // we clear the obstacle buffers
            ClearObstacleBuffers(obstacle_slab, obstacle_velocity_slab);

            // we draw fluid box borders in the obstacle position buffer
            BorderObstacle(borderObstacleShader, borderObstacleShaderLayered, obstacle_slab);

            // we define the model matrix of the fluid volume
            cubeModelMatrix = glm::mat4(1.0f);

            cubeModelMatrix = glm::translate(cubeModelMatrix, fluidTranslation);
            cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(fluidScale));

            // we update the model matrices for the obstacles
            for_each(obstacleObjects.begin(), obstacleObjects.end(), [&](ObstacleObject* obj)
            {
                if (!obj->isActive) return; // we skip the obstacle if it is not active

                // we save the previous model matrix
                obj->prevModelMatrix = obj->modelMatrix;

                // we update the model matrix
                obj->modelMatrix = glm::mat4(1.0f);

                obj->modelMatrix = glm::translate(obj->modelMatrix, obj->position);
                obj->modelMatrix = glm::rotate(obj->modelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
                obj->modelMatrix = glm::scale(obj->modelMatrix, obj->scale);

                // we draw the obstacle in the obstacle buffers
                DynamicObstacle(stencilObstacleShader, obstacleVelocityShader, obstacle_slab, obstacle_velocity_slab, temp_velocity_slab, obj, fluidTranslation, fluidScale, simulationFramerate);
            });

            /////////////////// STEP 2 - UPDATE SIMULATION  //////////////////////////////////////////////////////////////////////////

            // we bind the full-screen quad VAO and set up rendering
            BeginSimulation();

            // advect velocity
            AdvectMacCormack(&advectionShader, &macCormackShader, &velocity_slab, &phi1_hat_slab, &phi2_hat_slab, &obstacle_slab, &velocity_slab, &temp_velocity_slab, velocityDissipation, timeStep);
            
            // advect gas density or liquid level set
            AdvectMacCormack(&advectionShader, &macCormackShader, &velocity_slab, &phi1_hat_slab, &phi2_hat_slab, &obstacle_slab, &density_slab, &temp_pressure_divergence_slab, densityDissipation, timeStep);
           
            if (currTarget == GAS)
            {
                // advect temperature
                AdvectMacCormack(&advectionShader, &macCormackShader, &velocity_slab, &phi1_hat_slab, &phi2_hat_slab, &obstacle_slab, &temperature_slab, &temp_pressure_divergence_slab, temperatureDissipation, timeStep);

                // we apply the buoyancy force
                Buoyancy(buoyancyShader, &velocity_slab, &temperature_slab, &density_slab, &temp_velocity_slab, ambientTemperature, timeStep, dampingBuoyancy, ambientWeight);
            }
            else
            {
                // apply level set damping
                ApplyLevelSetDamping(*dampingLevelSetShader, density_slab, obstacle_slab, temp_pressure_divergence_slab, levelSetDampingFactor, levelSetEquilibriumHeight);
            }

            // we splat density and temperature for each emitter
            if (currTarget == GAS)
            {
                // we increase gas density and temperature by a fixed amount
                float dyeColor = 1.2f;

                for_each(fluidQuantities.begin(), fluidQuantities.end(), [&](FluidEmitter* fluidQuantity)
                {
                    if (fluidQuantity->radius > 0.0f)
                    {
                        AddDensity(&dyeShader, &density_slab, &temp_pressure_divergence_slab, fluidQuantity->position, fluidQuantity->radius, dyeColor, GL_FALSE);
                        AddTemperature(temperatureShader, &temperature_slab, &temp_pressure_divergence_slab, fluidQuantity->position, fluidQuantity->radius, dyeColor);
                    }
                });
            }
            else
            {
                // we splat liquid level set for each emitter 
                for_each(fluidQuantities.begin(), fluidQuantities.end(), [&](FluidEmitter* fluidQuantity)
                {
                    if (fluidQuantity->radius > 0.0f)
                    {
                        // we draw the level set as gaussian splat in the density buffer by adding a negative value equal to the radius
                        // this will create a level set consistent to its definition (negative inside and equal to surface distance, positive outside)
                        AddDensity(&dyeShader, &density_slab, &temp_pressure_divergence_slab, fluidQuantity->position, fluidQuantity->radius, -fluidQuantity->radius, GL_TRUE);
                    }
                });

                // we apply the gravity force to the level set
                ApplyGravity(*gravityShader, velocity_slab, density_slab, temp_velocity_slab, gravityAcceleration, timeStep, gravityLevelSetThreshold);
            }

            // we apply the external forces to the fluid
            for_each(externalForces.begin(), externalForces.end(), [&](Force* externalForce)
            {
                if (externalForce->radius > 0.0f && externalForce->strength > 0.0f)
                {
                    ApplyExternalForces(&externalForcesShader, &velocity_slab, &temp_velocity_slab, timeStep, externalForce->direction * externalForce->strength, externalForce->position, externalForce->radius);
                }
            });

            // we update the divergence texture
            Divergence(&divergenceShader, &velocity_slab, &divergence_slab, &obstacle_slab, &obstacle_velocity_slab, &temp_pressure_divergence_slab);

            // we update the pressure texture
            Jacobi(&jacobiShader, &pressure_slab, &divergence_slab, &obstacle_slab, &temp_pressure_divergence_slab, pressureIterations);

            // we apply the pressure projection
            ApplyPressure(&pressureShader, &velocity_slab, &pressure_slab, &obstacle_slab, &obstacle_velocity_slab, &temp_velocity_slab);

            // reset the state
            EndSimulation();

            // we update the simulation time
            lastSimulationUpdate = currentFrame;
        }

        /////////////////// STEP 3 - SCENE RENDERING: SHADOW MAP CREATION ////////////////////////////////////////////////////////

        // we enable the depth test
        glEnable(GL_DEPTH_TEST);

        // we set view and projection matrix for the rendering using light as a camera
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        GLfloat near_plane = -10.0f, far_plane = 10.0f;
        GLfloat frustumSize = 5.0f;

        // for a directional light, the projection is orthographic. For point lights, we should use a perspective projection
        lightProjection = glm::ortho(-frustumSize, frustumSize, -frustumSize, frustumSize, near_plane, far_plane);

        // the light is directional, so technically it has no position. We need a view matrix, so we consider a position on the the direction vector of the light
        lightView = glm::lookAt(lightDir0, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // transformation matrix for the light
        lightSpaceMatrix = lightProjection * lightView;

        /// We "install" the  Shader Program for the shadow mapping creation
        shadow_shader.Use();

        // we pass the transformation matrix as uniform
        glUniformMatrix4fv(glGetUniformLocation(shadow_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        // we set the viewport for the first rendering step = dimensions of the depth texture
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

        // we activate the FBO for the depth map rendering
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // we render the scene, using the shadow shader
        RenderObjects(shadow_shader, planeModel, SHADOWMAP, depthMap);

        /////////////////// STEP 3.5 - SCENE RENDERING: RENDERING FROM CAMERA ////////////////////////////////////////////////////////

        // we enable the buffer blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // we get the view matrix from the Camera class
        view = camera.GetViewMatrix();

        // we activate back the scene Frame Buffer
        glBindFramebuffer(GL_FRAMEBUFFER, scene.fbo);

        // we "clear" the frame and z buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // we set the rendering mode
        if (wireframe)
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // if animated rotation is activated, than we increment the rotation angle using delta time and the rotation speed parameter
        if (spinning)
            orientationY+=(deltaTime*spin_speed);

        // we set the viewport for the rendering step
        glViewport(0, 0, width, height);

        // We "install" the selected Shader Program as part of the current rendering process. We pass to the shader the light transformation matrix, and the depth map rendered in the first rendering step
        illumination_shader.Use();

         // we search inside the Shader Program the name of the subroutine currently selected, and we get the numerical index
        GLuint index = glGetSubroutineIndex(illumination_shader.Program, GL_FRAGMENT_SHADER, shaders[current_subroutine].c_str());

        // we activate the subroutine using the index (this is where shaders swapping happens)
        glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &index);

        // we pass projection and view matrices to the Shader Program
        glUniformMatrix4fv(glGetUniformLocation(illumination_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(illumination_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(illumination_shader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        // we determine the position in the Shader Program of the uniform variables
        GLint lightDirLocation = glGetUniformLocation(illumination_shader.Program, "lightVector");
        GLint kdLocation = glGetUniformLocation(illumination_shader.Program, "Kd");
        GLint alphaLocation = glGetUniformLocation(illumination_shader.Program, "alpha");
        GLint f0Location = glGetUniformLocation(illumination_shader.Program, "F0");

        // we assign the value to the uniform variables
        glUniform3fv(lightDirLocation, 1, glm::value_ptr(lightDir0));
        glUniform1f(kdLocation, Kd);
        glUniform1f(alphaLocation, alpha);
        glUniform1f(f0Location, F0);

        // we render the scene 
        RenderObjects(illumination_shader, planeModel, RENDER, depthMap);

        // we render fluid volume (back first to be included in the scene)
        fillShader.Use();

        // we enable the face culling
        glEnable(GL_CULL_FACE);

        // we pass the transformation matrices to the Shader Program
        glUniformMatrix4fv(glGetUniformLocation(fillShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(glm::scale(cubeModelMatrix, glm::vec3(1.001f, 1.001f, 1.001f)))); // we scale the cube a bit to avoid z-fighting
        glUniformMatrix4fv(glGetUniformLocation(fillShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(fillShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform4fv(glGetUniformLocation(fillShader.Program, "color"), 1, glm::value_ptr(glm::vec4(1.0f, 1.0f, 1.0f, 0.1f)));

        // we render the back faces of the cube
        glCullFace(GL_FRONT);
        cubeModel.Draw();

        // we reset the face culling and the blending
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        /////////////////// STEP 4 - RAYDATA: GENERATING DATA FOR RAYMARCHING ////////////////////////////////////////////////

        // we calculate the inverse screen size
        glm::vec2 inverseScreenSize = glm::vec2(1.0f / width, 1.0f / height);

        // we create the raydata texture
        RayData(raydataBackShader, raydataFrontShader, cubeModel, rayDataBack, rayDataFront, scene, cubeModelMatrix, view, projection, inverseScreenSize);

        //////////////////////////////// STEP 5 - RAYMARCHING ////////////////////////////////////////////////

        if (currTarget == GAS)
        {
            // we render the gas
            RenderGas(*renderShader, cubeModel, cubeModelMatrix, view, projection, rayDataFront, rayDataBack, density_slab, fluidScene, inverseScreenSize, windowNearPlane, camera.Position, camera.Front);
        }
        else
        {
            // we render the liquid
            RenderLiquid(*renderShader, density_slab, obstacle_slab, rayDataFront, rayDataBack, scene, fluidScene, cubeModel, cubeModelMatrix, view, projection, inverseScreenSize, windowNearPlane, camera.Position, camera.Front, camera.Up, camera.Right, lightDir0, Kd, alpha, F0);
        }

        //////////////////////////////// STEP 6 - BLENDING AND FINAL SCENE COMPOSITING ////////////////////////////////////////////////

        if (currTarget == LIQUID)
        {
            // we apply the post-process effects in the fluid scene to solve the banding effect
            Slab fluidSceneSlab  = {fluidScene.fbo, fluidScene.colorTex};

            switch (liquidEffect)
            {
                case BLUR:
                    Blur(blurShader, fluidSceneSlab, temp_screenSize_slab, blurRadius, inverseScreenSize);
                    break;
                case DENOISE:
                    DeNoise(deNoiseShader, fluidSceneSlab, temp_screenSize_slab, deNoiseSigma, deNoiseThreshold, deNoiseKSigma, inverseScreenSize);
                    fluidScene.colorTex = fluidSceneSlab.tex;
                    fluidScene.fbo = fluidSceneSlab.fbo;
                    break;
                default:
                    break;
            }
        }

        // we combine the fluid rendering with the scene rendering
        BlendRendering(blendingShader, scene, fluidScene, rayDataBack, inverseScreenSize);

        // we render the front faces of the fluid volume
        glEnable(GL_BLEND);
        glEnable(GL_CULL_FACE);

        fillShader.Use();

        glUniformMatrix4fv(glGetUniformLocation(fillShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(glm::scale(cubeModelMatrix, glm::vec3(1.001f, 1.001f, 1.001f))));
        glUniformMatrix4fv(glGetUniformLocation(fillShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(fillShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform4fv(glGetUniformLocation(fillShader.Program, "color"), 1, glm::value_ptr(glm::vec4(1.0f, 1.0f, 1.0f, 0.1f)));

        glCullFace(GL_BACK);
        cubeModel.Draw();

        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);

        //////////////////////////////// STEP 7 - RENDERING THE UI AND FINAL OPERATIONS ////////////////////////////////////////////////

        RenderUI();

        prevTarget = currTarget;
        currTarget = targetFluid;

        // Swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    illumination_shader.Delete();
    shadow_shader.Delete();

    advectionShader.Delete();
    macCormackShader.Delete();
    divergenceShader.Delete();
    jacobiShader.Delete();
    externalForcesShader.Delete();
    pressureShader.Delete();
    dyeShader.Delete();
    fillShader.Delete();

    temperatureShader->Delete();
    delete temperatureShader;
    buoyancyShader->Delete();
    delete buoyancyShader;
    initLiquidShader->Delete();
    delete initLiquidShader;
    dampingLevelSetShader->Delete();
    delete dampingLevelSetShader;
    gravityShader->Delete();
    delete gravityShader;

    borderObstacleShaderLayered.Delete();
    borderObstacleShader.Delete();
    stencilObstacleShader.Delete();
    obstacleVelocityShader.Delete();

    raydataBackShader.Delete();
    raydataFrontShader.Delete();

    blendingShader.Delete();
    blurShader.Delete();
    deNoiseShader.Delete();

    renderShader->Delete();
    delete renderShader;

    // chiudo e cancello il contesto creato
    glfwTerminate();
    return 0;
}

//////////////////////////////////////////

// we render the objects. We pass also the current rendering step, and the depth map generated in the first step, which is used by the shaders of the second step
void RenderObjects(Shader &shader, Model &planeModel, GLint render_pass, GLuint depthMap)
{
    // For the second rendering step -> we pass the shadow map to the shaders
    if (render_pass==RENDER)
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        GLint shadowLocation = glGetUniformLocation(shader.Program, "shadowMap");
        glUniform1i(shadowLocation, 2);
    }
    // we pass the needed uniforms
    GLint textureLocation = glGetUniformLocation(shader.Program, "tex");
    GLint repeatLocation = glGetUniformLocation(shader.Program, "repeat");

    // PLANE
    // we activate the texture of the plane
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureID[1]);
    glUniform1i(textureLocation, 1);
    glUniform1f(repeatLocation, 80.0);

    /*
      we create the transformation matrix

      N.B.) the last defined is the first applied

      We need also the matrix for normals transformation, which is the inverse of the transpose of the 3x3 submatrix (upper left) of the modelview. We do not consider the 4th column because we do not need translations for normals.
      An explanation (where XT means the transpose of X, etc):
        "Two column vectors X and Y are perpendicular if and only if XT.Y=0. If We're going to transform X by a matrix M, we need to transform Y by some matrix N so that (M.X)T.(N.Y)=0. Using the identity (A.B)T=BT.AT, this becomes (XT.MT).(N.Y)=0 => XT.(MT.N).Y=0. If MT.N is the identity matrix then this reduces to XT.Y=0. And MT.N is the identity matrix if and only if N=(MT)-1, i.e. N is the inverse of the transpose of M.
    */
    // we reset to identity at each frame
    planeModelMatrix = glm::mat4(1.0f);
    planeNormalMatrix = glm::mat3(1.0f);
    planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f, -1.0f, 0.0f));
    planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(10.0f, 1.0f, 10.0f));
    planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
    // we render the plane
    planeModel.Draw();

    // we activate the texture of the objects
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID[0]);
    glUniform1i(textureLocation, 0);
    glUniform1f(repeatLocation, repeat);

    // OBJECTS

    glm::mat3 normalMatrix;
    for_each(obstacleObjects.begin(), obstacleObjects.end(), [&](ObstacleObject* obj)
    {
        if (!obj->isActive) return;

        // we create the normal matrix
        normalMatrix = glm::inverseTranspose(glm::mat3(view * obj->modelMatrix));

        // we pass the needed uniforms
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(obj->modelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

        // we render the object
        obj->objectModel->Draw();
    });

    // we deactivate the textures
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);

}

//////////////////////////////////////////

// we load the image from disk and we create an OpenGL texture
GLint LoadTexture(const char* path)
{
    GLuint textureImage;
    int w, h, channels;
    unsigned char* image;
    image = stbi_load(path, &w, &h, &channels, STBI_rgb);

    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;

    glGenTextures(1, &textureImage);
    glBindTexture(GL_TEXTURE_2D, textureImage);
    // 3 channels = RGB ; 4 channel = RGBA
    if (channels==3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    else if (channels==4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    // we set how to consider UVs outside [0,1] range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // we set the filtering for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);

    // we set the binding to 0 once we have finished
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureImage;
}

///////////////////////////////////////////

// The function parses the content of the Shader Program, searches for the Subroutine type names,
// the subroutines implemented for each type, print the names of the subroutines on the terminal, and add the names of
// the subroutines to the shaders vector, which is used for the shaders swapping
void SetupShader(int program)
{
    int maxSub,maxSubU,countActiveSU;
    GLchar name[256];
    int len, numCompS;

    // global parameters about the Subroutines parameters of the system
    glGetIntegerv(GL_MAX_SUBROUTINES, &maxSub);
    glGetIntegerv(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, &maxSubU);
    std::cout << "Max Subroutines:" << maxSub << " - Max Subroutine Uniforms:" << maxSubU << std::endl;

    // get the number of Subroutine uniforms (only for the Fragment shader, due to the nature of the exercise)
    // it is possible to add similar calls also for the Vertex shader
    glGetProgramStageiv(program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &countActiveSU);

    // print info for every Subroutine uniform
    for (int i = 0; i < countActiveSU; i++) {

        // get the name of the Subroutine uniform (in this example, we have only one)
        glGetActiveSubroutineUniformName(program, GL_FRAGMENT_SHADER, i, 256, &len, name);
        // print index and name of the Subroutine uniform
        std::cout << "Subroutine Uniform: " << i << " - name: " << name << std::endl;

        // get the number of subroutines
        glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, i, GL_NUM_COMPATIBLE_SUBROUTINES, &numCompS);

        // get the indices of the active subroutines info and write into the array s
        int *s =  new int[numCompS];
        glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, i, GL_COMPATIBLE_SUBROUTINES, s);
        std::cout << "Compatible Subroutines:" << std::endl;

        // for each index, get the name of the subroutines, print info, and save the name in the shaders vector
        for (int j=0; j < numCompS; ++j) {
            glGetActiveSubroutineName(program, GL_FRAGMENT_SHADER, s[j], 256, &len, name);
            std::cout << "\t" << s[j] << " - " << name << "\n";
            shaders.push_back(name);
        }
        std::cout << std::endl;

        delete[] s;
    }
}

/////////////////////////////////////////

// we print on console the name of the currently used shader subroutine
void PrintCurrentShader(int subroutine)
{
    std::cout << "Current shader subroutine: " << shaders[subroutine]  << std::endl;
}

//////////////////////////////////////////

// If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
void apply_camera_movements()
{
    if (mouseUnlock) return; // if the mouse is not locked, we do not move the camera

    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_SPACE])
        camera.ProcessKeyboard(UP, deltaTime);
    if (keys[GLFW_KEY_LEFT_SHIFT])
        camera.ProcessKeyboard(DOWN, deltaTime);
}

//////////////////////////////////////////

// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    GLuint new_subroutine;

    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // if P is pressed, we start/stop the animated rotation of models
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
        spinning=!spinning;

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;

    // pressing a key number, we change the shader applied to the models
    // if the key is between 1 and 9, we proceed and check if the pressed key corresponds to
    // a valid subroutine
    if((key >= GLFW_KEY_1 && key <= GLFW_KEY_9) && action == GLFW_PRESS && !mouseUnlock)
    {
        // "1" to "9" -> ASCII codes from 49 to 59
        // we subtract 48 (= ASCII CODE of "0") to have integers from 1 to 9
        // we subtract 1 to have indices from 0 to 8
        new_subroutine = (key-'0'-1);
        // if the new index is valid ( = there is a subroutine with that index in the shaders vector),
        // we change the value of the current_subroutine variable
        // NB: we can just check if the new index is in the range between 0 and the size of the shaders vector,
        // avoiding to use the std::find function on the vector
        if (new_subroutine<shaders.size())
        {
            current_subroutine = new_subroutine;
            PrintCurrentShader(current_subroutine);
        }
    }

    // if right arrow is pressed, we increase the speed of the animated rotation of models (only if the mouse is not locked)
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT) && !mouseUnlock)
    {
        spin_speed += 5;
        std::cout << "Spin speed: " << spin_speed << std::endl;
    }

    // if left arrow is pressed, we decrease the speed of the animated rotation of models (only if the mouse is not locked)
    if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT) && !mouseUnlock)
    {
        spin_speed -= 5;
        std::cout << "Spin speed: " << spin_speed << std::endl;
    }

    // if M is pressed, we unlock/lock the mouse and we show/hide the UI
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        if (mouseUnlock)
        {
            mouseUnlock = false;
            firstMouse = true;
            CollapseUI();
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            mouseUnlock = true;
            ExpandUI();
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    // we keep trace of the pressed keys
    // with this method, we can manage 2 keys pressed at the same time:
    // many I/O managers often consider only 1 key pressed at the time (the first pressed, until it is released)
    // using a boolean array, we can then check and manage all the keys pressed at the same time
    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

//////////////////////////////////////////

// callback for mouse events
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
      // we move the camera view following the mouse cursor
      // we calculate the offset of the mouse cursor from the position in the last frame
      // when rendering the first frame, we do not have a "previous state" for the mouse, so we set the previous state equal to the initial values (thus, the offset will be = 0)
      if(firstMouse)
      {
          lastX = xpos;
          lastY = ypos;
          firstMouse = false;
      }

      if (mouseUnlock) return; // if the mouse is not locked, we do not move the camera

      // offset of mouse cursor position
      GLfloat xoffset = xpos - lastX;
      GLfloat yoffset = lastY - ypos;

      // the new position will be the previous one for the next frame
      lastX = xpos;
      lastY = ypos;

      // we pass the offset to the Camera class instance in order to update the rendering
      camera.ProcessMouseMovement(xoffset, yoffset);

}

//////////////////////////////////////////

// instantiates the shader for the fluid rendering based on the current target fluid
void CreateRenderShader(TargetFluid target)
{
    if (target == GAS)
    {
        // we create the Shader Programs for only gas rendering
        renderShader = new Shader("src/shaders/rendering/raydata/raydata.vert", "src/shaders/rendering/raymarching.frag");
    }
    else
    {
        // we create the Shader Programs for only liquid rendering
        renderShader = new Shader("src/shaders/rendering/raydata/raydata.vert", "src/shaders/rendering/liquid/raymarching_liquid.frag");
    }
}

// instantiates the target fluid exclusive shaders for the simulation
void CreateFluidShaders(TargetFluid target)
{
    if (target == GAS)
    {
        // we create the Shader Programs for only gas simulation
        buoyancyShader = new Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom"  ,"src/shaders/simulation/buoyancy.frag");
        temperatureShader = new Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom","src/shaders/simulation/add_temperature.frag");
    }
    else
    {
        // we create the Shader Programs for only liquid simulation
        initLiquidShader = new Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom","src/shaders/simulation/liquid/fill_levelSet.frag");
        dampingLevelSetShader = new Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom","src/shaders/simulation/liquid/damp_levelSet.frag");
        gravityShader = new Shader("src/shaders/simulation/load_vertices.vert", "src/shaders/simulation/set_layer.geom","src/shaders/simulation/liquid/add_gravity.frag");
    }
}
