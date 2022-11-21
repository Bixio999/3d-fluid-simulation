#include "fluid-sim.h"

// Std. Includes
#include <string>

//////////////////////////////////////

float GRID_WIDTH;
float GRID_HEIGHT;
float GRID_DEPTH;
glm::vec3 InverseSize;

GLuint quadVAO = 0;
GLuint borderVAO = 0;

//////////////////////////////////////

void CheckFramebufferStatus()
{
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error creating framebuffer: ";

        switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
        {
            case GL_FRAMEBUFFER_UNDEFINED:
                std::cout << "GL_FRAMEBUFFER_UNDEFINED" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << std::endl;
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cout << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS" << std::endl;
                break;
            default:
                std::cout << "Unknown error" << std::endl;
                break;
        }
    }
}

//////////////////////////////////////

// create a simulation grid slab
Slab CreateSlab(GLuint width, GLuint height, GLuint depth, GLushort dimensions)
{
    GLuint fbo;

    glGenFramebuffers(1, &fbo);

    GLuint texture;

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_3D, texture);

    switch(dimensions)
    {
        case 1: glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, width, height, depth, 0, GL_RED, GL_HALF_FLOAT, NULL); break;
        case 2: glTexImage3D(GL_TEXTURE_3D, 0, GL_RG16F, width, height, depth, 0, GL_RG, GL_HALF_FLOAT, NULL); break;
        case 3: glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, width, height, depth, 0, GL_RGB, GL_HALF_FLOAT, NULL); break;
        case 4: glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, width, height, depth, 0, GL_RGBA, GL_HALF_FLOAT, NULL); break;
        default: std::cout << "Invalid number of dimensions for the texture" << std::endl; return {0, 0};
    }

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error creating framebuffer" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_3D, 0);

    return {fbo, texture};
}

// create slab with a 2d texture
Slab Create2DSlab(GLuint width, GLuint height, GLushort dimensions)
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    switch(dimensions)
    {
        case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL); break;
        case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, NULL); break;
        case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL); break;
        case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL); break;
        default: std::cout << "Invalid number of dimensions for the texture" << std::endl; return {0, 0};
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    GLfloat borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return {fbo, texture};
}

Scene CreateScene(GLuint width, GLuint height)
{
    GLuint fbo;

    glGenFramebuffers(1, &fbo);

    GLuint colorTex, depthTex;

    glGenTextures(1, &colorTex);
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    GLfloat borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return {fbo, colorTex, depthTex};
}

// swap the simulation grid slabs
void SwapSlabs(Slab *slab1, Slab *slab2)
{
    GLuint temp = slab1->fbo;
    slab1->fbo = slab2->fbo;
    slab2->fbo = temp;

    temp = slab1->tex;
    slab1->tex = slab2->tex;
    slab2->tex = temp;
}

// define the simulation grid size
void SetGridSize(GLuint width, GLuint height, GLuint depth)
{
    GRID_WIDTH = width;
    GRID_HEIGHT = height;
    GRID_DEPTH = depth;

    InverseSize = glm::vec3(1.0f / width, 1.0f / height, 1.0f / depth);
}

// create the quad vao for rendering
void CreateQuadVAO()
{
    GLuint quad_VBO;

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    short positions[] = { 
        -1, -1,
        1, -1,
        -1, 1,
        1, 1
    };
    glGenBuffers(1, &quad_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, 2 * sizeof(short), 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CreateBorderVAO()
{
    GLuint vbo;

    glGenVertexArrays(1, &borderVAO);
    glBindVertexArray(borderVAO);

    #define V 0.9999f

    float positions[] = {
        -V, -V,
        V, -V,
        V, V,
        -V, V,
        -V, -V
    };
    #undef V

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// initialize the simulation data structures
void InitSimulation()
{
    // create the quad vao (triangle strip)
    CreateQuadVAO();

    // create the border vao (lines strip)
    CreateBorderVAO();
}

// setup vars to start simulation phase
void BeginSimulation()
{
    glBindVertexArray(quadVAO);
    glViewport(0,0, GRID_WIDTH, GRID_HEIGHT);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// reset state to end simulation phase
void EndSimulation()
{
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
}

///////////////////////////////////////////

// execute advection 
void Advect(Shader* advectionShader, Slab *velocity, Obstacle *obstacle, Slab *source, Slab *dest, float dissipation, float timeStep)
{
    advectionShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, velocity->tex);
    glUniform1i(glGetUniformLocation(advectionShader->Program, "VelocityTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, source->tex);
    glUniform1i(glGetUniformLocation(advectionShader->Program, "SourceTexture"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, obstacle->tex);
    glUniform1i(glGetUniformLocation(advectionShader->Program, "ObstacleTexture"), 2);

    glUniform1f(glGetUniformLocation(advectionShader->Program, "timeStep"), timeStep);
    glUniform3fv(glGetUniformLocation(advectionShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));
    glUniform1f(glGetUniformLocation(advectionShader->Program, "dissipation"), dissipation);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, 0);

    // SwapSlabs(source, dest);
}

void AdvectMacCormack(Shader* advectionShader, Shader* macCormackShader, Slab *velocity, Slab *phi1_hat, Slab *phi2_hat, Obstacle *obstacle, Slab* source, Slab* dest, float dissipation, float timeStep)
{
    // first advection pass - compute phi1_hat
    Advect(advectionShader, velocity, obstacle, source, phi1_hat, dissipation, timeStep);

    // second advection pass - compute phi2_hat
    Advect(advectionShader, velocity, obstacle, phi1_hat, phi2_hat, 1 / dissipation, -timeStep);

    // third advection pass - compute new velocities

    macCormackShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, velocity->tex);
    glUniform1i(glGetUniformLocation(macCormackShader->Program, "VelocityTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, phi1_hat->tex);
    glUniform1i(glGetUniformLocation(macCormackShader->Program, "Phi1HatTexture"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, phi2_hat->tex);
    glUniform1i(glGetUniformLocation(macCormackShader->Program, "Phi2HatTexture"), 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, source->tex);
    glUniform1i(glGetUniformLocation(macCormackShader->Program, "SourceTexture"), 3);

    glUniform1f(glGetUniformLocation(macCormackShader->Program, "timeStep"), timeStep);
    glUniform3fv(glGetUniformLocation(macCormackShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));
    glUniform1f(glGetUniformLocation(macCormackShader->Program, "dissipation"), dissipation);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(source, dest);
}

// execute buoyancy
void Buoyancy(Shader* buoyancyShader, Slab *velocity, Slab *temperature, Slab *density, Slab *dest, float ambientTemperature, float timeStep, float sigma, float kappa)
{
    buoyancyShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, velocity->tex);
    glUniform1i(glGetUniformLocation(buoyancyShader->Program, "VelocityTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, temperature->tex);
    glUniform1i(glGetUniformLocation(buoyancyShader->Program, "TemperatureTexture"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, density->tex);
    glUniform1i(glGetUniformLocation(buoyancyShader->Program, "DensityTexture"), 2);

    glUniform1f(glGetUniformLocation(buoyancyShader->Program, "timeStep"), timeStep);
    glUniform1f(glGetUniformLocation(buoyancyShader->Program, "ambientTemperature"), ambientTemperature);
    glUniform1f(glGetUniformLocation(buoyancyShader->Program, "smokeBuoyancy"), sigma);
    glUniform1f(glGetUniformLocation(buoyancyShader->Program, "smokeWeight"), kappa);
    glUniform3fv(glGetUniformLocation(buoyancyShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(velocity, dest);
}

// apply external forces
void ApplyExternalForces(Shader *externalForcesShader, Slab *velocity, Slab *dest, float timeStep, glm::vec3 force, glm::vec3 position, float radius)
{
    externalForcesShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, velocity->tex);
    glUniform1i(glGetUniformLocation(externalForcesShader->Program, "VelocityTexture"), 0);
    glUniform1f(glGetUniformLocation(externalForcesShader->Program, "timeStep"), timeStep);
    glUniform3fv(glGetUniformLocation(externalForcesShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));
    glUniform3fv(glGetUniformLocation(externalForcesShader->Program, "force"), 1, glm::value_ptr(force));
    glUniform3fv(glGetUniformLocation(externalForcesShader->Program, "center"), 1, glm::value_ptr(position));
    glUniform1f(glGetUniformLocation(externalForcesShader->Program, "radius"), radius);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(velocity, dest);
}

// add density
void AddDensity(Shader *dyeShader, Slab *density, Slab *dest, glm::vec3 position, float radius, float color)
{
    dyeShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, density->tex);
    glUniform1i(glGetUniformLocation(dyeShader->Program, "DensityTexture"), 0);
    glUniform3fv(glGetUniformLocation(dyeShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));
    glUniform3fv(glGetUniformLocation(dyeShader->Program, "center"), 1, glm::value_ptr(position));
    glUniform1f(glGetUniformLocation(dyeShader->Program, "radius"), radius);
    glUniform1f(glGetUniformLocation(dyeShader->Program, "dyeIntensity"), color);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(density, dest);
}

// add temperature
void AddTemperature(Shader *dyeShader, Slab *temperature, Slab *dest, glm::vec3 position, float radius, float appliedTemperature)
{
    dyeShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, temperature->tex);
    glUniform1i(glGetUniformLocation(dyeShader->Program, "TemperatureTexture"), 0);
    glUniform3fv(glGetUniformLocation(dyeShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));
    glUniform3fv(glGetUniformLocation(dyeShader->Program, "center"), 1, glm::value_ptr(position));
    glUniform1f(glGetUniformLocation(dyeShader->Program, "radius"), radius);
    glUniform1f(glGetUniformLocation(dyeShader->Program, "temperature"), appliedTemperature);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(temperature, dest);
}

// execute divergence
void Divergence(Shader* divergenceShader, Slab *velocity, Slab *divergence, Obstacle *obstacle, Slab *dest)
{
    divergenceShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, velocity->tex);
    glUniform1i(glGetUniformLocation(divergenceShader->Program, "VelocityTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, obstacle->tex);
    glUniform1i(glGetUniformLocation(divergenceShader->Program, "ObstacleTexture"), 1);

    glUniform3fv(glGetUniformLocation(divergenceShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(divergence, dest);
}

// execute jacobi
void Jacobi(Shader* jacobiShader, Slab *pressure, Slab *divergence, Obstacle *obstacle, Slab *dest, GLuint iterations)
{
    jacobiShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, pressure->fbo);
    glClear(GL_COLOR_BUFFER_BIT);

    for (GLuint i = 0; i < iterations; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, pressure->tex);
        glUniform1i(glGetUniformLocation(jacobiShader->Program, "Pressure"), 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, divergence->tex);
        glUniform1i(glGetUniformLocation(jacobiShader->Program, "Divergence"), 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_3D, obstacle->tex);
        glUniform1i(glGetUniformLocation(jacobiShader->Program, "Obstacle"), 2);

        glUniform3fv(glGetUniformLocation(jacobiShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

        SwapSlabs(pressure, dest);
    }

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, 0);
}


// apply pressure
void ApplyPressure(Shader* pressureShader, Slab *velocity, Slab *pressure, Obstacle *obstacle, Slab *dest)
{
    pressureShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, velocity->tex);
    glUniform1i(glGetUniformLocation(pressureShader->Program, "VelocityTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, pressure->tex);
    glUniform1i(glGetUniformLocation(pressureShader->Program, "PressureTexture"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, obstacle->tex);
    glUniform1i(glGetUniformLocation(pressureShader->Program, "ObstacleTexture"), 2);

    glUniform3fv(glGetUniformLocation(pressureShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(velocity, dest);
}

//////////////////// RAYDATA TEXTURE CREATION /////////////////////////

void RayData(Shader &backShader, Shader &frontShader, Model &cubeModel, Slab &back, Slab &front, Scene &scene, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection, glm::vec2 inverseScreenSize)
{
    glEnable(GL_CULL_FACE);

    backShader.Use();

    glBindFramebuffer(GL_FRAMEBUFFER, back.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene.depthTex);
    glUniform1i(glGetUniformLocation(backShader.Program, "SceneDepthTexture"), 0);

    glUniformMatrix4fv(glGetUniformLocation(backShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(backShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(backShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(backShader.Program, "grid_size"), 1, glm::value_ptr(glm::vec3(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH)));
    glUniform2fv(glGetUniformLocation(backShader.Program, "InverseSize"), 1, glm::value_ptr(inverseScreenSize));

    glCullFace(GL_FRONT);

    cubeModel.Draw();

    frontShader.Use();

    glBindFramebuffer(GL_FRAMEBUFFER, front.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, back.tex);
    glUniform1i(glGetUniformLocation(frontShader.Program, "RayDataTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, scene.depthTex);
    glUniform1i(glGetUniformLocation(frontShader.Program, "SceneDepthTexture"), 1);

    glUniformMatrix4fv(glGetUniformLocation(frontShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(frontShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(frontShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(frontShader.Program, "grid_size"), 1, glm::value_ptr(glm::vec3(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH)));
    glUniform2fv(glGetUniformLocation(frontShader.Program, "InverseSize"), 1, glm::value_ptr(inverseScreenSize));

    glCullFace(GL_BACK);

    cubeModel.Draw();

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_CULL_FACE);
}

/////////////////////// VOLUME RENDERING WITH RAYMARCHING TECHNIQUE //////////////////////////

// render front faces of the cube
void RenderFluid(Shader &renderShader, Model &cubeModel, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection, Slab &rayDataFront, Slab &rayDataBack, Slab &density, Scene &dest, glm::vec2 inverseScreenSize, GLfloat nearPlane, glm::vec3 eyePosition, glm::vec3 cameraFront)
{
    renderShader.Use();
    glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rayDataFront.tex);
    glUniform1i(glGetUniformLocation(renderShader.Program, "RayDataFront"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, density.tex);
    glUniform1i(glGetUniformLocation(renderShader.Program, "DensityTexture"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rayDataBack.tex);
    glUniform1i(glGetUniformLocation(renderShader.Program, "RayDataBack"), 2);

    glUniformMatrix4fv(glGetUniformLocation(renderShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(renderShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(renderShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(renderShader.Program, "grid_size"), 1, glm::value_ptr(glm::vec3(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH)));
    glUniform2fv(glGetUniformLocation(renderShader.Program, "InverseSize"), 1, glm::value_ptr(inverseScreenSize));
    glUniform1f(glGetUniformLocation(renderShader.Program, "nearPlane"), nearPlane);
    glUniform3fv(glGetUniformLocation(renderShader.Program, "eyePos"), 1, glm::value_ptr(eyePosition));
    glUniform3fv(glGetUniformLocation(renderShader.Program, "cameraFront"), 1, glm::value_ptr(cameraFront));

    cubeModel.Draw();

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // cubeModel.Draw();

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlendRendering(Shader &blendingShader, Scene &scene, Scene &fluid, Slab &raydataBack, glm::vec2 inverseScreenSize)
{
    blendingShader.Use();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fluid.colorTex);
    glUniform1i(glGetUniformLocation(blendingShader.Program, "FluidTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fluid.depthTex);
    glUniform1i(glGetUniformLocation(blendingShader.Program, "FluidDepth"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, raydataBack.tex);
    glUniform1i(glGetUniformLocation(blendingShader.Program, "RayDataDepth"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, scene.colorTex);
    glUniform1i(glGetUniformLocation(blendingShader.Program, "SceneTexture"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, scene.depthTex);
    glUniform1i(glGetUniformLocation(blendingShader.Program, "SceneDepth"), 4);

    glUniform2fv(glGetUniformLocation(blendingShader.Program, "InverseSize"), 1, glm::value_ptr(inverseScreenSize));

    glBindVertexArray(quadVAO);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, 0);
}

///////////////////////// OBSTACLE FUNCTIONS /////////////////////////////

void BorderObstacle(Shader &borderObstacleShader, Shader &borderObstacleShaderLayered, Obstacle &dest)
{
    glViewport(0,0, GRID_WIDTH, GRID_HEIGHT);

    glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    borderObstacleShaderLayered.Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform3fv(glGetUniformLocation(borderObstacleShaderLayered.Program, "color"), 1, glm::value_ptr(color));

    glBindVertexArray(borderVAO);
    glDrawArraysInstanced(GL_LINE_STRIP, 0, 5, GRID_DEPTH);

    borderObstacleShader.Use();
    glBindVertexArray(quadVAO);

    glUniform4fv(glGetUniformLocation(borderObstacleShader.Program, "color"), 1, glm::value_ptr(color));

    glBindFramebuffer(GL_FRAMEBUFFER, dest.firstLayerFBO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.lastLayerFBO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // for(int i = 0; i < GRID_DEPTH; i++)
    // {
    //     glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dest.tex, 0, i);
    //     glDrawArrays(GL_LINE_STRIP, 0, 5);
    // }

    // glBindVertexArray(quadVAO);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Obstacle CreateObstacleBuffer(GLuint width, GLuint height, GLuint depth)
{
    GLuint fbo;

    glGenFramebuffers(1, &fbo);


    GLuint texture;

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_3D, texture);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, width, height, depth, 0, GL_RED, GL_HALF_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);


    GLuint depthStencil = 0;

    glGenTextures(1, &depthStencil);

    glBindTexture(GL_TEXTURE_2D_ARRAY, depthStencil);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH24_STENCIL8, width, height, depth, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthStencil, 0);

    std::cout << "\tChecking for general obstacle buffer FBO completeness: " << std::endl;
    CheckFramebufferStatus();

    GLuint firstLayerFBO, lastLayerFBO;

    glGenFramebuffers(1, &firstLayerFBO);
    glGenFramebuffers(1, &lastLayerFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, firstLayerFBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, 0);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthStencil, 0, 0);

    std::cout << "\tChecking for first layer FBO completeness: " << std::endl;
    CheckFramebufferStatus();

    glBindFramebuffer(GL_FRAMEBUFFER, lastLayerFBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, depth - 1);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthStencil, 0, depth - 1);

    std::cout << "\tChecking for last layer FBO completeness: " << std::endl;
    CheckFramebufferStatus();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    return {fbo, texture, depthStencil, firstLayerFBO, lastLayerFBO};
}

Slab CreateStencilBuffer(GLuint width, GLuint height)
{
    GLuint fbo;

    glGenFramebuffers(1, &fbo);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8, width, height, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
    // glDrawBuffer(GL_NONE);
    // glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error creating framebuffer: ";

        switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
        {
            case GL_FRAMEBUFFER_UNDEFINED:
                std::cout << "GL_FRAMEBUFFER_UNDEFINED" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << std::endl;
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cout << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" << std::endl;
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                std::cout << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS" << std::endl;
                break;
            default:
                std::cout << "Unknown error" << std::endl;
                break;
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return {fbo, tex};
}

void DynamicObstacle(Shader &stencilObstacleShader, Obstacle &dest, Model &obstacleModel, glm::mat4 &model, glm::vec3 translation, GLfloat scale)
{
    glViewport(0,0, GRID_WIDTH, GRID_HEIGHT);

    glm::mat4 projection, view;

    GLfloat far_plane = 100, near_plane = 1;
    GLfloat frustumSize = scale;

    glm::vec3 viewEye = glm::vec3(translation);
    viewEye.y = viewEye.y * scale + 1.0f;
    // std::cout << "viewEye: " << viewEye.x << ", " << viewEye.y << ", " << viewEye.z << std::endl;
    // std::cout << "translation: " << translation.x << ", " << translation.y << ", " << translation.z << std::endl;

    glm::vec3 viewCenter = translation;
    viewCenter.x += glm::epsilon<float>(); // avoid gimbal lock
    glm::vec3 viewUp = glm::vec3(0.0f, 1.0f, 0.0f);
    view = glm::lookAt(viewEye, viewCenter, viewUp);

    // std::cout << view[0][0] << " " << view[0][1] << " " << view[0][2] << " " << view[0][3] << std::endl;
    // std::cout << view[1][0] << " " << view[1][1] << " " << view[1][2] << " " << view[1][3] << std::endl;
    // std::cout << view[2][0] << " " << view[2][1] << " " << view[2][2] << " " << view[2][3] << std::endl;
    // std::cout << view[3][0] << " " << view[3][1] << " " << view[3][2] << " " << view[3][3] << std::endl;

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    // glStencilOp(GL_KEEP, GL_INCR_WRAP, GL_DECR_WRAP);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    stencilObstacleShader.Use();

    projection = glm::ortho(-frustumSize, frustumSize, -frustumSize, frustumSize, near_plane, far_plane);
    glUniformMatrix4fv(glGetUniformLocation(stencilObstacleShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(stencilObstacleShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(stencilObstacleShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glUniform1f(glGetUniformLocation(stencilObstacleShader.Program, "scaling_factor"), scale);
    glUniform1f(glGetUniformLocation(stencilObstacleShader.Program, "grid_depth"), GRID_DEPTH);

    glUniform3fv(glGetUniformLocation(stencilObstacleShader.Program, "color"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));

    obstacleModel.DrawInstanced(GRID_DEPTH);

    // glDisable(GL_DEPTH_TEST);
    glStencilFunc(GL_GREATER, 0, 0xFF);
    glStencilMask(0x00);

    obstacleModel.DrawInstanced(GRID_DEPTH);

    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDisable(GL_STENCIL_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /*
     * PROBLEMA: DEVO RENDERIZZARE L'OGGETTO PER OGNI LAYER MA HO BISOGNO DI MANTERE IL BUFFER DI STENCIL;
        * COLOR E DEPTH/STENCIL PRESENTI IN DEST FRAMEBUFFER

        * SOLUZIONE ??? : CREARE UN FRAMEBUFFER PER OGNI LAYER E RENDERIZZARE OGNI LAYER IN UN FRAMEBUFFER DIVERSO

        * ALTERNATIVA: CREARE UN ARRAY DI PROJECTION E USARE LAYERED RENDERING RICHIAMANDO LA MATRICE CORRETTA CON gl_Layer
            * POSSIBILITÀ DI USARE 



        * NOTA: RENDERDOC NON VISUALIZZA LO STENCIL BUFFER - INAFFIDABILE PER DEBUGGARE
     */


    // for(int i = 0; i < GRID_DEPTH; i++)
    // {
    //     near_plane = 2 * scale * (1.0f - (float) (i + 1) / GRID_DEPTH) + 1.0f;
    //     // std::cout << "near_plane: " << near_plane << " ; slice: " << i << std::endl;
    //     projection = glm::ortho(-frustumSize, frustumSize, -frustumSize, frustumSize, near_plane, far_plane);

    //     glEnable(GL_DEPTH_TEST);
    //     // glEnable(GL_STENCIL_TEST);
    //     glStencilFunc(GL_ALWAYS, 1, 0xFF);
    //     glStencilMask(0xFF);
    //     glStencilOp(GL_KEEP, GL_INCR_WRAP, GL_DECR_WRAP);
    //     // glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    //     // glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_DECR_WRAP);
    //     // glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_INCR_WRAP);

    //     // glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dest.tex, 0, i);
    //     // glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);
    //     glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dest.tex, 0, i);
    //     glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, stencil_buffer.tex, 0, i);
    //     // glClearStencil(0xFF);
    //     glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    //     fillShader.Use();

    //     glUniformMatrix4fv(glGetUniformLocation(fillShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    //     glUniformMatrix4fv(glGetUniformLocation(fillShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    //     glUniformMatrix4fv(glGetUniformLocation(fillShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    //     obstacleModel.Draw();

    //     glDisable(GL_DEPTH_TEST);
    //     glStencilFunc(GL_GREATER, 0, 0xFF);
    //     glStencilMask(0x00);
    //     // glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    //     // glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);

    //     obstacleModel.Draw();

        // glDisable(GL_DEPTH_TEST);
        // glDisable(GL_STENCIL_TEST);

        // glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dest.tex, 0, i);
        // glBindVertexArray(quadVAO);
        // stencilToObstacleShader.Use();

        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, stencil_buffer.tex);
        // glUniform1i(glGetUniformLocation(stencilToObstacleShader.Program, "StencilTexture"), 0);
        // glUniform2fv(glGetUniformLocation(stencilToObstacleShader.Program, "InverseSize"), 1, glm::value_ptr(glm::vec2(1 / GRID_WIDTH, 1 / GRID_HEIGHT)));

        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // }
    //     glDisable(GL_STENCIL_TEST);
    //     glDisable(GL_DEPTH_TEST);


    // // glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
