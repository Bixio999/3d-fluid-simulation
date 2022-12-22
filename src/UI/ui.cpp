#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "ui.h"

//////////////////////////

// parameters for simulation time step
GLfloat timeStep;
GLfloat simulationFramerate;

// we define the target for fluid simulation
// TargetFluid targetFluid = LIQUID;
TargetFluid targetFluid;

// we define the post-process effect for liquid
LiquidEffect liquidEffect;

// Level Set parameters
GLfloat levelSetDampingFactor;

GLfloat levelSetEquilibriumHeight;
GLfloat levelSetInitialHeight;

// Liquid parameters
GLfloat gravityAcceleration;
GLfloat gravityLevelSetThreshold;

// Jacobi pressure solver iterations
GLuint pressureIterations;

// Buoyancy parameters
GLfloat ambientTemperature;
GLfloat ambientBuoyancy;
GLfloat ambientWeight;

// Dissipation factors
GLfloat velocityDissipation;
GLfloat densityDissipation;
GLfloat temperatureDissipation;

// Fluid Volume parameters
glm::vec3 fluidTranslation;
GLfloat fluidScale;

// Blur filter parameters
GLfloat blurRadius;

// DeNoise filter parameters
GLfloat deNoiseSigma;
GLfloat deNoiseThreshold;
GLfloat deNoiseSlider;
GLfloat deNoiseKSigma;

// rotation angle on Y axis
GLfloat orientationY;
// rotation speed on Y axis
GLfloat spin_speed;

// Custom fluid interaction
vector<Force*> externalForces = vector<Force*>();
vector<FluidQuantity*> fluidQuantities = vector<FluidQuantity*>();

//////////////////////////

void InitFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ResetParameters()
{
    // parameters for simulation time step
    timeStep = 0.25f; // 0.25f
    simulationFramerate = 1.0f / 60.0f;

    // we define the target for fluid simulation
    targetFluid = LIQUID;
    // targetFluid = GAS;

    // we define the post-process effect for liquid
    liquidEffect = NONE;

    // Level Set parameters
    levelSetDampingFactor = 0.2f; // 0.2f

    levelSetEquilibriumHeight = 0.4f;
    levelSetInitialHeight = 0.4f;

    // Liquid parameters
    gravityAcceleration = 3.0f;
    gravityLevelSetThreshold = 1.0f;

    // Jacobi pressure solver iterations
    pressureIterations = 40; // 40

    // Buoyancy parameters
    ambientTemperature = 0.0f;
    ambientBuoyancy = 0.9f;
    ambientWeight = 0.15f;

    // Dissipation factors
    velocityDissipation = 0.99f; // 0.8f
    densityDissipation = targetFluid == GAS ? 0.99f : 1.0f; // 0.9f
    temperatureDissipation = 0.9f; // 0.9f

    // Fluid Volume parameters
    fluidTranslation = glm::vec3(0.0f, 2.0f, 1.0f);
    fluidScale = 2.0f;

    // Blur filter parameters
    blurRadius = 1.0f;

    // DeNoise filter parameters
    deNoiseSigma = 7.0f;
    deNoiseThreshold = 0.23f;
    deNoiseSlider = 0.0f;
    deNoiseKSigma = 3.0f;

    // // rotation angle on Y axis
    // orientationY = 0.0f;
    // rotation speed on Y axis
    spin_speed = 60.0f;
}

void ResetForcesAndDyes(TargetFluid target)
{
    externalForces.clear();
    fluidQuantities.clear();

    // we define the default static external forces and fluid quantities
    
    if (target == GAS)
    {
        // we add the forces for gas simulation
        externalForces.push_back(new Force {glm::vec3(GRID_WIDTH / 2.0f, GRID_HEIGHT * 0.4f, GRID_DEPTH * 0.7f), 
                                        glm::vec3(0,0,-1), 
                                        20.0f,
                                        2.0f});

        // we add the fluid for gas simulation
        fluidQuantities.push_back(new FluidQuantity {glm::vec3(GRID_WIDTH / 2.0f, GRID_HEIGHT * 0.4f, GRID_DEPTH * 0.7f), 
                                        5.0f});
    }
    else
    {
        // we add the forces for liquid simulation
        glm::vec3 v = glm::vec3(GRID_WIDTH / 2.0f, GRID_HEIGHT * 0.8f, GRID_DEPTH / 2.0f);
        v.x += v.x * 0.1f;
        externalForces.push_back(new Force {v,
                                        glm::vec3(1,0,0), 
                                        5.0f,
                                        2.0f});
        v.x -= v.x * 0.2f;
        externalForces.push_back(new Force {glm::vec3(v),
                                        glm::vec3(-1,0,0), 
                                        5.0f,
                                        2.0f});

        // we add the fluid for liquid simulation
        fluidQuantities.push_back(new FluidQuantity {glm::vec3(GRID_WIDTH / 2.0f, GRID_HEIGHT * 0.8f, GRID_DEPTH / 2.0f), 
                                        3.0f});
    }
}

void ShowSimulationProperties()
{
    if (!ImGui::CollapsingHeader("Simulation Properties", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Simulation"))
    {
        ImGui::SliderFloat("Time Step", &timeStep, 0.0f, 1.0f);
        timeStep = std::max(timeStep, 0.0f);

        static int simFramerate = 60;
        ImGui::SliderInt("Framerate", &simFramerate, 0, 1000);
        simulationFramerate = 1.0f / simFramerate;

        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Velocity Solver"))
    {
        ImGui::SliderFloat("Dissipation", &velocityDissipation, 0.0f, 1.0f);
        ImGui::SliderInt("Pressure Iterations", (int*)&pressureIterations, 0, 100);

        ImGui::TreePop();
    }
}

void ShowLiquidParameters()
{
    if (!ImGui::CollapsingHeader("Liquid Parameters", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    if (ImGui::TreeNode("Level Set"))
    {
        ImGui::SliderFloat("Initial Height", &levelSetInitialHeight, 0.0f, 1.0f);
        ImGui::SliderFloat("Damping Factor", &levelSetDampingFactor, 0.0f, 1.0f);
        ImGui::SliderFloat("Equilibrium Height", &levelSetEquilibriumHeight, 0.0f, 1.0f);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Gravity"))
    {
        ImGui::SliderFloat("Acceleration Factor", &gravityAcceleration, 0.0f, 10.0f);
        ImGui::SliderFloat("Level Set Threshold", &gravityLevelSetThreshold, 0.0f, 10.0f);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Post-process effect"))
    {
        const char* items[] = {"None", "Blur", "DeNoise"};
        // static int item_current = 0;
        ImGui::Combo("Post-process effect", (int*) &liquidEffect, items, IM_ARRAYSIZE(items));

        switch (liquidEffect)
        {
            case BLUR: // Blur
                static int val = (int)blurRadius;
                ImGui::SliderInt("Blur Radius", &val, 1.0f, 10.0f);
                blurRadius = (float)val;
                break;
            case DENOISE: // DeNoise
                ImGui::SliderFloat("DeNoise Sigma", &deNoiseSigma, 0.0f, 10.0f);
                ImGui::SliderFloat("DeNoise Threshold", &deNoiseThreshold, 0.0f, 10.0f);
                ImGui::SliderFloat("DeNoise K", &deNoiseKSigma, 0.0f, 10.0f);
                break;
            default:
                break;
        }

        ImGui::TreePop();
    }
}

void ShowGasParameters()
{
    if (!ImGui::CollapsingHeader("Gas Parameters", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    if (ImGui::TreeNode("Buoyancy"))
    {
        ImGui::SliderFloat("Buoyancy Factor", &ambientBuoyancy, 0.0f, 10.0f);
        ImGui::SliderFloat("Weight Factor", &ambientWeight, 0.0f, 10.0f);
        ImGui::SliderFloat("Ambient Temperature", &ambientTemperature, 0.0f, 10.0f);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Dissipation"))
    {
        ImGui::SliderFloat("Temperature Dissipation", &temperatureDissipation, 0.0f, 1.0f);
        ImGui::SliderFloat("Density Dissipation", &densityDissipation, 0.0f, 1.0f);

        ImGui::TreePop();
    }
}

void ShowStaticForceParameters()
{
    if (!ImGui::CollapsingHeader("Static Force Parameters"))
        return;

    for (int i = 0; i < externalForces.size(); i++)
    {
        std::string s = "Force " + std::to_string(i);
        if (ImGui::TreeNode(s.c_str()))
        {
            // glm::vec3 v = externalForces[i].position;
            // float val[3] = {v.x, v.y, v.z};
            ImGui::SliderFloat3("Position", glm::value_ptr(externalForces[i]->position), 0.0f, GRID_WIDTH);
            // externalForces[i].position = glm::vec3(val[0], val[1], val[2]);

            // v = externalForces[i].direction;
            ImGui::SliderFloat3("Direction", glm::value_ptr(externalForces[i]->direction), -1.0, 1.0f);

            ImGui::SliderFloat("Strength", &(externalForces[i]->strength), 0.0f, 10.0f);
            ImGui::SliderFloat("Radius", &(externalForces[i]->radius), 0.0f, 10.0f);

            if (ImGui::Button("Delete"))
            {
                Force* f = externalForces[i];
                externalForces.erase(externalForces.begin() + i);
                delete f;
            }

            ImGui::TreePop();
        }
    }

    if (ImGui::Button("Add Force"))
    {
        externalForces.push_back(new Force{ glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 0.0f });
    }
}

void ShowStaticFluidDyeParameter()
{
    if (!ImGui::CollapsingHeader("Static Fluid Dye Parameters"))
        return;

    for (int i = 0; i < fluidQuantities.size(); i++)
    {
        std::string s = "Dye " + std::to_string(i);
        if (ImGui::TreeNode(s.c_str()))
        {
            // glm::vec3 v = fluidQuantities[i]->position;
            ImGui::SliderFloat3("Position", glm::value_ptr(fluidQuantities[i]->position), 0.0f, GRID_WIDTH);

            ImGui::SliderFloat("Radius", &(fluidQuantities[i]->radius), 0.0f, 10.0f);

            if (ImGui::Button("Delete"))
            {
                FluidQuantity* q = fluidQuantities[i];
                fluidQuantities.erase(fluidQuantities.begin() + i);
                delete q;
            }

            ImGui::TreePop();
        }
    }

    if (ImGui::Button("Add Fluid"))
    {
        fluidQuantities.push_back(new FluidQuantity { glm::vec3(0.0f), 0.0f});
    }
}

void CustomUI()
{
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver);

    int windowFlags =   ImGuiWindowFlags_NoMove  
                      | ImGuiWindowFlags_AlwaysAutoResize
                      | ImGuiWindowFlags_AlwaysVerticalScrollbar
                      ;

    ImGui::SetNextWindowCollapsed(false, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("3D Fluid Simulation", NULL, windowFlags)) 
    {
        ImGui::End();
        return;
    }

    ////////////////////////////////
    
    ImGui::Text("Target fluid:");
    ImGui::RadioButton("Gas",(int*) &targetFluid, 0); ImGui::SameLine();
    ImGui::RadioButton("Liquid",(int*) &targetFluid, 1); 

    ImGui::Separator(); ImGui::Spacing();

    if (ImGui::Button("Reset"))
    {
        ResetParameters();
    } ImGui::SameLine();

    if (ImGui::Button("Reset forces and dyes"))
        ResetForcesAndDyes(targetFluid);

    ////////////////////////////////

    ShowSimulationProperties();

    ////////////////////////////////

    if (targetFluid == GAS)
        ShowGasParameters();
    else
        ShowLiquidParameters();

    ////////////////////////////////

    ShowStaticForceParameters();

    ShowStaticFluidDyeParameter();

    ////////////////////////////////

    ImGui::End();
}

void DrawUI()
{
    InitFrame();

    CustomUI();
    // ImGui::ShowDemoWindow();
}

void CollapseUI()
{
    ImGui::SetWindowCollapsed("3D Fluid Simulation", true);
}

void ExpandUI()
{
    ImGui::SetWindowCollapsed("3D Fluid Simulation", false);
}

void RenderUI()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void InitUI(GLFWwindow* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}
