#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <imgui/ImGuiFileDialog/ImGuiFileDialog.h>

#include "ui.h"

//////////////////////////
// parameters definition

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
GLfloat dampingBuoyancy;
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
GLfloat deNoiseKSigma;

// rotation speed on Y axis
GLfloat spin_speed;

// Custom fluid interaction
vector<Force*> externalForces = vector<Force*>();
vector<FluidEmitter*> fluidQuantities = vector<FluidEmitter*>();

// data structure for obstacle objects interaction
vector<ObstacleObject*> obstacleObjects = vector<ObstacleObject*>();

//////////////////////////

// Reset parameters to default values
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
    gravityAcceleration = 9.0f;
    gravityLevelSetThreshold = 1.0f;

    // Jacobi pressure solver iterations
    pressureIterations = 40; // 40

    // Buoyancy parameters
    ambientTemperature = 0.0f;
    dampingBuoyancy = 0.9f;
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
    deNoiseKSigma = 3.0f;

    // rotation speed on Y axis
    spin_speed = 60.0f;
}

// Reset the forces and fluid quantities
void ResetForcesAndEmitters(TargetFluid target)
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
        fluidQuantities.push_back(new FluidEmitter {glm::vec3(GRID_WIDTH / 2.0f, GRID_HEIGHT * 0.4f, GRID_DEPTH * 0.7f), 
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
        fluidQuantities.push_back(new FluidEmitter {glm::vec3(GRID_WIDTH / 2.0f, GRID_HEIGHT * 0.8f, GRID_DEPTH / 2.0f), 
                                        3.0f});
    }
}

//////////////////////////
// we define the GUI for the simulation parameters

// draw the GUI for general fluid simulation parameters
void ShowSimulationProperties()
{
    // header
    if (!ImGui::CollapsingHeader("Simulation Properties", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    // simulation tree 
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Simulation"))
    {
        // simulation time step
        ImGui::SliderFloat("Time Step", &timeStep, 0.0f, 1.0f);
        timeStep = std::max(timeStep, 0.0f); // we do not allow negative time steps

        // simulation framerate
        static int simFramerate = 60; // we define the number of frames per second
        ImGui::SliderInt("Framerate", &simFramerate, 0, 1000);
        simulationFramerate = 1.0f / simFramerate; // we compute the simulation framerate in seconds

        ImGui::TreePop();
    }

    // velocity solver tree
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Velocity Solver"))
    {
        // velocity dissipation
        ImGui::SliderFloat("Dissipation", &velocityDissipation, 0.0f, 1.0f);

        // pressure solver iterations
        ImGui::SliderInt("Pressure Iterations", (int*)&pressureIterations, 0, 100);

        ImGui::TreePop();
    }
}

// draw the GUI for the liquid simulation parameters
void ShowLiquidParameters()
{
    // header
    if (!ImGui::CollapsingHeader("Liquid Parameters", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    // level set tree
    if (ImGui::TreeNode("Level Set"))
    {
        // level set damping factor
        ImGui::SliderFloat("Damping Factor", &levelSetDampingFactor, 0.0f, 1.0f);

        // level set equilibrium height
        ImGui::SliderFloat("Equilibrium Height", &levelSetEquilibriumHeight, 0.0f, 1.0f);

        ImGui::TreePop();
    }

    // gravity tree
    if (ImGui::TreeNode("Gravity"))
    {
        // gravity acceleration
        ImGui::SliderFloat("Acceleration Factor", &gravityAcceleration, 0.0f, 15.0f);

        // gravity level set threshold
        ImGui::SliderFloat("Level Set Threshold", &gravityLevelSetThreshold, 0.0f, 10.0f);

        ImGui::TreePop();
    }

    // post processing tree
    if (ImGui::TreeNode("Post-process effect"))
    {
        // available post-process effects
        const char* items[] = {"None", "Blur", "DeNoise"};
        ImGui::Combo("Post-process effect", (int*) &liquidEffect, items, IM_ARRAYSIZE(items));

        // draw the parameters for the selected post-process effect
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

// draw the GUI for the gas simulation parameters
void ShowGasParameters()
{
    // header
    if (!ImGui::CollapsingHeader("Gas Parameters", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    // buoyancy tree
    if (ImGui::TreeNode("Buoyancy"))
    {
        // damping factor
        ImGui::SliderFloat("Buoyancy damping factor", &dampingBuoyancy, 0.0f, 1.0f);

        // gas weight
        ImGui::SliderFloat("Weight Factor", &ambientWeight, 0.0f, 1.0f);

        // ambient temperature
        ImGui::SliderFloat("Ambient Temperature", &ambientTemperature, 0.0f, 1.0f);

        ImGui::TreePop();
    }

    // dissipations tree
    if (ImGui::TreeNode("Dissipation"))
    {
        ImGui::SliderFloat("Temperature Dissipation", &temperatureDissipation, 0.0f, 1.0f);
        ImGui::SliderFloat("Density Dissipation", &densityDissipation, 0.0f, 1.0f);

        ImGui::TreePop();
    }
}

// draw the GUI for the creation and manipulation of external forces
void ShowStaticForceParameters()
{
    // header
    if (!ImGui::CollapsingHeader("Static Force Parameters"))
        return;

    // draw the force parameters for each force currently defined
    for (int i = 0; i < externalForces.size(); i++)
    {
        // we create a tree node for each force
        std::string s = "Force " + std::to_string(i);
        if (ImGui::TreeNode(s.c_str()))
        {
            // force position
            ImGui::SliderFloat3("Position", glm::value_ptr(externalForces[i]->position), 0.0f, GRID_WIDTH);

            // force direction
            ImGui::SliderFloat3("Direction", glm::value_ptr(externalForces[i]->direction), -1.0, 1.0f);

            // force strength and radius
            ImGui::SliderFloat("Strength", &(externalForces[i]->strength), 0.0f, 20.0f);
            ImGui::SliderFloat("Radius", &(externalForces[i]->radius), 0.0f, 20.0f);

            // delete button
            if (ImGui::Button("Delete"))
            {
                Force* f = externalForces[i];
                externalForces.erase(externalForces.begin() + i);
                delete f;
            }

            ImGui::TreePop();
        }
    }

    // create a new force
    if (ImGui::Button("Add Force"))
    {
        externalForces.push_back(new Force{ glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 0.0f });
    }
}

// draw the GUI for the creation and manipulation of static fluid emitters
void ShowStaticFluidEmitterParameter()
{
    // header
    if (!ImGui::CollapsingHeader("Static Fluid Emitter Parameters"))
        return;

    // draw the fluid emitter parameters for each emitter currently defined
    for (int i = 0; i < fluidQuantities.size(); i++)
    {
        // we create a tree node for each emitter
        std::string s = "Emitter " + std::to_string(i);
        if (ImGui::TreeNode(s.c_str()))
        {
            // emitter position
            ImGui::SliderFloat3("Position", glm::value_ptr(fluidQuantities[i]->position), 0.0f, GRID_WIDTH);

            // emitter radius
            ImGui::SliderFloat("Radius", &(fluidQuantities[i]->radius), 0.0f, 10.0f);

            if (targetFluid == GAS)
                ImGui::SliderFloat("Temperature", &(fluidQuantities[i]->temperature), -5.0f, 10.0f);

            // delete button
            if (ImGui::Button("Delete"))
            {
                FluidEmitter* q = fluidQuantities[i];
                fluidQuantities.erase(fluidQuantities.begin() + i);
                delete q;
            }

            ImGui::TreePop();
        }
    }

    // create a new emitter
    if (ImGui::Button("Add Fluid"))
    {
        fluidQuantities.push_back(new FluidEmitter());
    }
}

// draw the GUI popup window for the creation of an obstacle objects
void ShowObstacleObjectCreationWindow()
{
    // center the window
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    // create the window
    if (ImGui::BeginPopupModal("Add new obstacle", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // display the creation description
        ImGui::Text("Select an object to add as an obstacle by selecting its\nhigh poly mesh for scene rendering and low poly mesh for\nsimulation obstacle rendering. ");
        ImGui::Separator();

        // select the object to add from the predefined objects list
        const char* items[] = { "New import", "Cube", "Sphere", "Bunny", "Baby Yoda" };
        static int item_current = 0;
        ImGui::Combo("Object", &item_current, items, IM_ARRAYSIZE(items));

        ImGui::Separator();

        // if the user selects a new object to import, display the input fields for the object name and the paths to the high and low poly meshes
        static char object_name[50], high_poly_mesh_path[256], low_poly_mesh_path[256];
        if (item_current == 0)
        {
            // input field for the object name
            ImGui::InputText("Obstacle object name", object_name, IM_ARRAYSIZE(object_name), ImGuiInputTextFlags_CharsNoBlank);

            // window resize constraints for the file browser
            ImVec2 maxSize = ImGui::GetMainViewport()->Size;  // The full display area
            ImVec2 minSize = ImVec2(maxSize.x * 0.3f, maxSize.y * 0.3f);  // A third of the display area

            ImGui::Spacing();

            //////////////////////////////////////////

            // input field for the path to the high poly mesh
            ImGui::InputText("##highpolyPath", high_poly_mesh_path, IM_ARRAYSIZE(high_poly_mesh_path), ImGuiInputTextFlags_CharsNoBlank);
            ImGui::SameLine();
            if (ImGui::Button("Browse##HighPoly"))
            {
                // open the file browser
                ImGuiFileDialog::Instance()->OpenDialog("ChooseHighPolyMesh", "Choose High Poly Mesh", ".obj", ".");
            }

            // display the file browser
            if (ImGuiFileDialog::Instance()->Display("ChooseHighPolyMesh", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
            {
                // if the user selects a file, copy the path to the high poly mesh path input field
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    strcpy(high_poly_mesh_path, filePathName.substr(0, 256).c_str());
                }
                ImGuiFileDialog::Instance()->Close();
            }
            ImGui::SameLine(); ImGui::Text("High poly mesh path");

            ImGui::Spacing();

            //////////////////////////////////////////

            // input field for the path to the low poly mesh
            ImGui::InputText("##lowpolyPath", low_poly_mesh_path, IM_ARRAYSIZE(low_poly_mesh_path), ImGuiInputTextFlags_CharsNoBlank);
            ImGui::SameLine();
            if (ImGui::Button("Browse##LowPoly"))
            {
                // open the file browser
                ImGuiFileDialog::Instance()->OpenDialog("ChooseLowPolyMesh", "Choose Low Poly Mesh", ".obj", ".");
            }
            ImGui::SameLine(); ImGui::Text("Low poly mesh path");

            // display the file browser
            if (ImGuiFileDialog::Instance()->Display("ChooseLowPolyMesh", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
            {
                // if the user selects a file, copy the path to the low poly mesh path input field
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    strcpy(low_poly_mesh_path, filePathName.substr(0, 256).c_str());
                }
                ImGuiFileDialog::Instance()->Close();
            }
            ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
        }

        // prevent object creation if the user tries to add an object without filling in all the fields
        if (ImGui::Button("Add", ImVec2(120, 0))) 
        { 
            if (item_current == 0)
            {
                if (strlen(object_name) == 0 || strlen(high_poly_mesh_path) == 0 || strlen(low_poly_mesh_path) == 0)
                {
                    ImGui::OpenPopup("Error! Please fill in all fields.");
                }
                // if the user fills in all the fields, create the object
                else
                {
                    std::cout << "Creating new obstacle object: " << object_name << std::endl;
                    CreateObstacleObject(high_poly_mesh_path, low_poly_mesh_path, object_name);

                    // reset the input fields
                    memset(object_name, 0, sizeof(object_name));
                    memset(high_poly_mesh_path, 0, sizeof(high_poly_mesh_path));
                    memset(low_poly_mesh_path, 0, sizeof(low_poly_mesh_path));

                    ImGui::CloseCurrentPopup();
                }
            }
            else // if the user selects a predefined object, create it
            {
                switch (item_current)
                {
                    case 1: // Cube
                        CreateObstacleObject("models/cube.obj", "Cube");
                        break;
                    case 2: // Sphere
                        CreateObstacleObject("models/sphere.obj", "Sphere");
                        break;
                    case 3: // Bunny
                        CreateObstacleObject("models/bunny.obj", "Bunny");
                        break;
                    case 4: // Baby Yoda
                        CreateObstacleObject("models/babyyoda.obj", "models/low-poly_babyyoda.obj", "Baby Yoda");
                        break;
                    
                    default:
                        break;
                }
                ImGui::CloseCurrentPopup();
            }   
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

// draw the GUI for the creation and manipulation of obstacle objects
void ShowObstacleObjectsControls()
{
    // header
    if (!ImGui::CollapsingHeader("Obstacle Objects"))
        return;

    // draw the obstacle object parameters for each object currently defined
    for (int i = 0; i < obstacleObjects.size(); i++)
    {
        // we create a tree node for each object
        if (ImGui::TreeNode(obstacleObjects[i]->name.c_str()))
        {
            // enable or disable the object
            ImGui::Checkbox("Enabled", &(obstacleObjects[i]->isActive));

            // object position and scale
            ImGui::SliderFloat3("Position", glm::value_ptr(obstacleObjects[i]->position), -10.0f, 10.0f);
            ImGui::SliderFloat3("Scale", glm::value_ptr(obstacleObjects[i]->scale), 0.0f, 10.0f);

            // delete button
            if (ImGui::Button("Delete"))
            {
                ObstacleObject* o = obstacleObjects[i];
                obstacleObjects.erase(obstacleObjects.begin() + i); 

                delete o;
            }

            ImGui::TreePop();
        }
    }

    // create a new object
    if (ImGui::Button("Add Obstacle"))
    {
        // draw the popup window
        ImGui::OpenPopup("Add new obstacle");
    }

    // draw the popup window for the creation of a new obstacle object
    ShowObstacleObjectCreationWindow();
}

// draw the application GUI
void CustomUI()
{
    // define the viewport
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_FirstUseEver);

    // define the window flags
    int windowFlags =   ImGuiWindowFlags_NoMove  
                      | ImGuiWindowFlags_AlwaysAutoResize
                      | ImGuiWindowFlags_AlwaysVerticalScrollbar
                      ;

    // draw the main window
    ImGui::SetNextWindowCollapsed(false, ImGuiCond_FirstUseEver); // open the window at the start of the application
    if (!ImGui::Begin("3D Fluid Simulation", NULL, windowFlags)) 
    {
        ImGui::End();
        return;
    }

    ////////////////////////////////
    // draw the simulation target controls and reset options

    ImGui::Text("Target fluid:");
    ImGui::RadioButton("Gas",(int*) &targetFluid, 0); ImGui::SameLine();
    ImGui::RadioButton("Liquid",(int*) &targetFluid, 1); 

    ImGui::Separator(); ImGui::Spacing();

    if (ImGui::Button("Reset"))
    {
        ResetParameters();
    } ImGui::SameLine();

    if (ImGui::Button("Reset forces and emitters"))
        ResetForcesAndEmitters(targetFluid);

    ////////////////////////////////
    // draw the simulation parameters

    ShowSimulationProperties();

    ////////////////////////////////
    // draw the fluid parameters

    if (targetFluid == GAS)
        ShowGasParameters();
    else
        ShowLiquidParameters();

    ////////////////////////////////
    // draw the fluid and force emitter parameters

    ShowStaticForceParameters();

    ShowStaticFluidEmitterParameter();

    ////////////////////////////////
    // draw the obstacle object controls

    ShowObstacleObjectsControls();

    ////////////////////////////////

    ImGui::End();
}

// initialize the Dear ImGui frame
void InitFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

// draw the GUI
void DrawUI()
{
    InitFrame();

    CustomUI();
    // ImGui::ShowDemoWindow();
}

// render the GUI
void RenderUI()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// initialize the Dear ImGui library
void InitUI(GLFWwindow* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

// api to collapse the GUI
void CollapseUI()
{
    ImGui::SetWindowCollapsed("3D Fluid Simulation", true);
}

// api to expand the GUI
void ExpandUI()
{
    ImGui::SetWindowCollapsed("3D Fluid Simulation", false);
}

////////////////////////////
// Obstacle Object creation

// create a new obstacle object and add it to the list of obstacle objects
void CreateObstacleObject(const string& highPolyPath, const string& lowPolyPath, const char* name, glm::vec3 position, glm::vec3 scale)
{
    // create the high poly model for scene rendering
    Model* highPoly = new Model(highPolyPath);
    Model* lowPoly;

    // check if the low poly model is the same as the high poly model
    if (highPolyPath == lowPolyPath)
        lowPoly = highPoly;
    else
        lowPoly = new Model(lowPolyPath);

    // setup the name of the object
    string n;
    if (name)
        n = name;
    else
        n = "Obstacle " + std::to_string(obstacleObjects.size() + 1);

    // create the obstacle object
    ObstacleObject *obj = new ObstacleObject { glm::mat4(1.0), glm::mat4(1.0), highPoly, lowPoly, position, scale, n, true };

    // add the object to the list of obstacle objects
    obstacleObjects.push_back(obj);
}

// create a new obstacle object with the same model for scene and simulation rendering, and add it to the list of obstacle objects
void CreateObstacleObject(const string& highPolyPath, const char* name, glm::vec3 position, glm::vec3 scale)
{
    CreateObstacleObject(highPolyPath, highPolyPath, name, position, scale);
}
