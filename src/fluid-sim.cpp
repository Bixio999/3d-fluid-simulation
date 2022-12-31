#include "fluid-sim.h"

// Std. Includes
#include <string>

//////////////////////////////////////
// we define the simulation parameters

// we define the simulation grid size
float GRID_WIDTH;
float GRID_HEIGHT;
float GRID_DEPTH;

// utility vector used to store the inverse of the grid size
glm::vec3 InverseSize;

// we define the utility VAOs as global variables
GLuint quadVAO = 0;
GLuint borderVAO = 0;

//////////////////////////////////////
// utility functions

// check for framebuffer completeness
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
// we define the utility functions for the simulation 

// create a simulation grid slab with the given dimensions and number of components
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

    // we set the texture filters to linear interpolation
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // we set the texture wrapping to clamp to edge
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

// create slab with a 2d texture defined with the given dimensions and number of components
// the 2d slabs are used to usually store screen size textures
Slab Create2DSlab(GLuint width, GLuint height, GLushort dimensions, bool filter)
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

    // we set the texture filters to linear interpolation or nearest neighbor
    if (filter)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    // we set the texture wrapping to clamp to edge
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // we set the border color to black
    GLfloat borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return {fbo, texture};
}

// destroy the given slab
void DestroySlab(Slab* slab)
{
    glDeleteFramebuffers(1, &slab->fbo);
    glDeleteTextures(1, &slab->tex);
}

// clear the given slabs
void ClearSlabs(int nSlabs, ...)
{
    va_list slabs;
    va_start(slabs, nSlabs);

    // for each given slab in arguments, we bind the framebuffer and clear the color buffer
    for (int i = 0; i < nSlabs; i++)
    {
        Slab* slab = va_arg(slabs, Slab*);
        glBindFramebuffer(GL_FRAMEBUFFER, slab->fbo);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// create a scene with a color and depth texture
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

// swap the simulation grid slabs. This is used due to the ping-pong method 
// (use the previous result as the input for the next iteration, so two slabs are needed due to 
// openGL's framebuffer binding)
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

// create the border vao for rendering a line strip placed in the frame borders
void CreateBorderVAO()
{
    GLuint vbo;

    glGenVertexArrays(1, &borderVAO);
    glBindVertexArray(borderVAO);

    // we set the border vertices a bit inside the frame to avoid artifacts
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
void InitSimulationVAOs()
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

// execute advection with semi-Lagrangian method
void Advect(Shader* advectionShader, Slab *velocity, ObstacleSlab *obstacle, Slab *source, Slab *dest, float dissipation, float timeStep)
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
}

// execute advection with MacCormack method: allows to reduce the error compared to semi-Lagrangian method for the same grid resolution
void AdvectMacCormack(Shader* advectionShader, Shader* macCormackShader, Slab *velocity, Slab *phi1_hat, Slab *phi2_hat, ObstacleSlab *obstacle, Slab* source, Slab* dest, float dissipation, float timeStep)
{
    // use the semi-Lagrangian advection as base for the MacCormack method

    // first advection pass - compute phi1_hat (predictor step)
    Advect(advectionShader, velocity, obstacle, source, phi1_hat, dissipation, timeStep);

    // second advection pass - compute phi2_hat (corrector step)
    Advect(advectionShader, velocity, obstacle, phi1_hat, phi2_hat, 1 / dissipation, -timeStep);

    // third advection pass - compute new velocities by using predictor and corrector to minimize the error

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
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, obstacle->tex);
    glUniform1i(glGetUniformLocation(macCormackShader->Program, "ObstacleTexture"), 4);

    glUniform1f(glGetUniformLocation(macCormackShader->Program, "timeStep"), timeStep);
    glUniform3fv(glGetUniformLocation(macCormackShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));
    glUniform1f(glGetUniformLocation(macCormackShader->Program, "dissipation"), dissipation);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(source, dest);
}

// compute and apply the buoyancy force to the velocity field of the gas
// simulate the effect of temperature and density on the velocity field
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

// apply external forces to the velocity field
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

// emit fluid into the density field at a given position
void AddDensity(Shader* dyeShader, Slab *density, Slab *dest, glm::vec3 position, float radius, float color, GLboolean isLiquidSimulation)
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
    glUniform1i(glGetUniformLocation(dyeShader->Program, "isLiquidSimulation"), isLiquidSimulation);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(density, dest);
}

// increase the temperature of the fluid at a given position
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

// compute the divergence of the velocity field
void Divergence(Shader* divergenceShader, Slab *velocity, Slab *divergence, ObstacleSlab *obstacle, Slab *obstacleVelocity, Slab *dest)
{
    divergenceShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, velocity->tex);
    glUniform1i(glGetUniformLocation(divergenceShader->Program, "VelocityTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, obstacle->tex);
    glUniform1i(glGetUniformLocation(divergenceShader->Program, "ObstacleTexture"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, obstacleVelocity->tex);
    glUniform1i(glGetUniformLocation(divergenceShader->Program, "ObstacleVelocityTexture"), 2);

    glUniform3fv(glGetUniformLocation(divergenceShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(divergence, dest);
}

// execute jacobi iterations to solve the pressure equation
void Jacobi(Shader* jacobiShader, Slab *pressure, Slab *divergence, ObstacleSlab *obstacle, Slab *dest, GLuint iterations)
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


// apply pressure projection to the velocity field
void ApplyPressure(Shader* pressureShader, Slab *velocity, Slab *pressure, ObstacleSlab *obstacle, Slab *obstacleVelocity, Slab *dest)
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
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, obstacleVelocity->tex);
    glUniform1i(glGetUniformLocation(pressureShader->Program, "ObstacleVelocityTexture"), 3);

    glUniform3fv(glGetUniformLocation(pressureShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(velocity, dest);
}

//////////////////// RAYDATA TEXTURE CREATION /////////////////////////

// draw the textures for front and back raydata buffers.
// this is done to gather information to be used in the raymarcher.
// the front buffer is used to gather information about the initial ray position in the volume
// and flags position where the volume is occluded by an obstacle.
// the back buffer is used to gather information about the final ray position and volume backface depth for scene blending.
void RayData(Shader &backShader, Shader &frontShader, Model &cubeModel, Slab &back, Slab &front, Scene &scene, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection, glm::vec2 inverseScreenSize)
{
    // draw back raydata

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

    // draw front raydata

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

/////////////////////// RENDERING //////////////////////////

// render gas fluid using raymarching technique. this is done by sampling the density texture with
// the data gathered in the two raydata textures.
// draw the result in a scene object.
void RenderGas(Shader &renderShader, Model &cubeModel, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection, Slab &rayDataFront, Slab &rayDataBack, Slab &density, Scene &dest, glm::vec2 inverseScreenSize, GLfloat nearPlane, glm::vec3 eyePosition, glm::vec3 cameraFront)
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

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// render liquid fluid using raymarching technique. this is done by sampling the density texture with
// the data gathered in the two raydata textures. Liquid surface is also shaded with ggx lighting model.
// for each surface point, the normal is approximated by the gradient of the level set function. the 
// lighting model is then applied to the surface point. Refraction is also applied to all surface points.
// draw the result in a scene object.
void RenderLiquid(Shader &renderShader, Slab &levelSet, ObstacleSlab &obstacle, Slab &rayDataFront, Slab &rayDataBack, Scene &backgroudScene, Scene &dest, Model &cubeModel, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection, glm::vec2 inverseScreenSize, GLfloat nearPlane, glm::vec3 eyePosition, glm::vec3 cameraFront, glm::vec3 cameraUp, glm::vec3 cameraRight, glm::vec3 lightDirection, GLfloat Kd, GLfloat rugosity, GLfloat F0)
{
    renderShader.Use();
    glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, levelSet.tex);
    glUniform1i(glGetUniformLocation(renderShader.Program, "LevelSetTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, backgroudScene.colorTex);
    glUniform1i(glGetUniformLocation(renderShader.Program, "BackgroundTexture"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rayDataFront.tex);
    glUniform1i(glGetUniformLocation(renderShader.Program, "RayDataFront"), 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, rayDataBack.tex);
    glUniform1i(glGetUniformLocation(renderShader.Program, "RayDataBack"), 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, obstacle.tex);
    glUniform1i(glGetUniformLocation(renderShader.Program, "ObstacleTexture"), 4);

    glUniformMatrix4fv(glGetUniformLocation(renderShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(renderShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(renderShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(renderShader.Program, "grid_size"), 1, glm::value_ptr(glm::vec3(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH)));
    glUniform2fv(glGetUniformLocation(renderShader.Program, "InverseScreenSize"), 1, glm::value_ptr(inverseScreenSize));
    glUniform1f(glGetUniformLocation(renderShader.Program, "nearPlane"), nearPlane);
    glUniform3fv(glGetUniformLocation(renderShader.Program, "eyePos"), 1, glm::value_ptr(eyePosition));
    glUniform3fv(glGetUniformLocation(renderShader.Program, "cameraFront"), 1, glm::value_ptr(cameraFront));
    glUniform3fv(glGetUniformLocation(renderShader.Program, "cameraUp"), 1, glm::value_ptr(cameraUp));
    glUniform3fv(glGetUniformLocation(renderShader.Program, "cameraRight"), 1, glm::value_ptr(cameraRight));

    glUniform1f(glGetUniformLocation(renderShader.Program, "Kd"), Kd);
    glUniform1f(glGetUniformLocation(renderShader.Program, "rugosity"), rugosity);
    glUniform1f(glGetUniformLocation(renderShader.Program, "F0"), F0);
    glUniform3fv(glGetUniformLocation(renderShader.Program, "lightVector"), 1, glm::value_ptr(lightDirection));

    cubeModel.Draw();

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_3D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// blend the fluid scene with the background scene. this is done by sampling the fluid scene color and depth and
// compare the depth with the background scene depth. if the fluid scene depth is closer, then the fluid scene color
// is used. otherwise, the background scene color is used. because the fluid can have depth values equal to cube volume
// front face depth or near plane depth in case of culling (camera is inside cube volume), the depth comparison is done 
// by comparing the background scene depth with the first values and the cube volume back face depth (stored in 
// raydataBack texture) to handle the cases of objects inside the fluid volume.
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

//////////////////////////////////////
// Liquid rendering post processing

// blur the fluid scene color to solve the dithering problem caused by raymarching jittering used to
// avoid banding artifacts. the blur effect is applied through a separable gaussian blur filter (so 
// two passes are required)
void Blur(Shader &blurShader, Slab &source, Slab &dest, GLfloat radius, glm::vec2 inverseScreenSize)
{
    blurShader.Use();

    glUniform1f(glGetUniformLocation(blurShader.Program, "radius"), radius);
    glUniform2fv(glGetUniformLocation(blurShader.Program, "InverseScreenSize"), 1, glm::value_ptr(inverseScreenSize));

    glBindVertexArray(quadVAO);

    for(int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, source.tex);
        glUniform1i(glGetUniformLocation(blurShader.Program, "SourceTexture"), 0);

        glUniform1i(glGetUniformLocation(blurShader.Program, "axis"), i);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        SwapSlabs(&source, &dest);  
    }

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// apply a denoising filter to the fluid scene color. the filter is based on a circular gaussian filter
// (see https://github.com/BrutPitt/glslSmartDeNoise)
void DeNoise(Shader &deNoiseShader, Slab &source, Slab &dest, GLfloat sigma, GLfloat threshold, GLfloat kSigma, glm::vec2 inverseScreenSize)
{
    deNoiseShader.Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source.tex);
    glUniform1i(glGetUniformLocation(deNoiseShader.Program, "imageData"), 0);

    glUniform1f(glGetUniformLocation(deNoiseShader.Program, "uSigma"), sigma);
    glUniform1f(glGetUniformLocation(deNoiseShader.Program, "uThreshold"), threshold);
    glUniform1f(glGetUniformLocation(deNoiseShader.Program, "uKSigma"), kSigma);
    glUniform2fv(glGetUniformLocation(deNoiseShader.Program, "InverseScreenSize"), 1, glm::value_ptr(inverseScreenSize));

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    SwapSlabs(&source, &dest);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

///////////////////////// OBSTACLE FUNCTIONS /////////////////////////////

// draw the cube volume obstacle borders in the obstacle texture. the borders are drawn as a set of
// line strips, one for each depth slice of the obstacle volume, and two full screen quads are used
// to draw the first and last layers. 
void BorderObstacle(Shader &borderObstacleShader, Shader &borderObstacleShaderLayered, ObstacleSlab &dest)
{
    glViewport(0,0, GRID_WIDTH, GRID_HEIGHT);

    // draw the borders in all the depth slices with line strips

    glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

    borderObstacleShaderLayered.Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);

    glUniform4fv(glGetUniformLocation(borderObstacleShaderLayered.Program, "color"), 1, glm::value_ptr(color));

    glBindVertexArray(borderVAO);
    glDrawArraysInstanced(GL_LINE_STRIP, 0, 5, GRID_DEPTH);

    // draw the borders in the first and last layers with full screen quads

    borderObstacleShader.Use();

    glBindVertexArray(quadVAO);

    glUniform4fv(glGetUniformLocation(borderObstacleShader.Program, "color"), 1, glm::value_ptr(color));

    glBindFramebuffer(GL_FRAMEBUFFER, dest.firstLayerFBO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.lastLayerFBO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// intantiate an obstacle slab with the given dimensions. an obstacle slab is composed by
// a 3D texture for color buffer and a 2D texture array for depth-stencil buffer. 
// for the last one, the stencil buffer is the only one used, but OpenGL 4.1 doesn't
// allow to create a 2D texture array with only the stencil buffer, neither to create
// a single 2D texture with the stencil buffer
ObstacleSlab CreateObstacleBuffer(GLuint width, GLuint height, GLuint depth)
{
    GLuint fbo;

    glGenFramebuffers(1, &fbo);

    GLuint texture;

    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_3D, texture);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, width, height, depth, 0, GL_RED, GL_HALF_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

    // create the first and last layer FBOs to draw the borders of the obstacle volume
    // with full screen quads

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

// clear the obstacle buffers (both position and velocity)
void ClearObstacleBuffers(ObstacleSlab &obstaclePosition, Slab &obstacleVelocity)
{
    glBindFramebuffer(GL_FRAMEBUFFER, obstaclePosition.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, obstacleVelocity.fbo);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// draw in the obstacle buffer a voxelized version of the obstacle in order to 
// determine the grid cells that are inside the obstacle. The stencil buffer is used
// to apply the voxelization through the stencil test, with a similar algorithm to the
// stencil shadow volume one: the stencil buffer is incremented for each back-facing
// fragment, and decremented for each front-facing fragment. after rendering both 
// back and front faces of the obstacle, the stencil buffer will contain a non-zero value
// for each grid cell that is inside the obstacle. in order to capture the geometry of the
// obstacle through each slice of the 3D texture, the shader computes a different projection
// matrix for each slice, by adjusting the near plane to the slice's depth, and the far plane
// a default value (100.0f) away enough to capture the whole obstacle. 
//
//as optimization, the poly model of the obstacle is used (when instanced) to reduce the number 
// of data to compute and send to the GPU without losing the geometry of the obstacle.
void DynamicObstaclePosition(Shader &stencilObstacleShader, ObstacleSlab &dest, ObstacleObject* obstacle, glm::mat4 view, glm::mat4 projection, GLfloat scale)
{
    glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);

    glEnable(GL_BLEND);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_DECR); // decrement stencil value for front-facing fragments
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_INCR); // increment stencil value for back-facing fragments

    glStencilFunc(GL_ALWAYS, 0, 0xFF); // always pass stencil test
    glStencilMask(0xFF); // enable writing to stencil buffer

    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDepthMask(GL_FALSE);

    stencilObstacleShader.Use();

    glUniformMatrix4fv(glGetUniformLocation(stencilObstacleShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(stencilObstacleShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(stencilObstacleShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(obstacle->modelMatrix));

    glUniform1f(glGetUniformLocation(stencilObstacleShader.Program, "scaling_factor"), scale);
    glUniform1f(glGetUniformLocation(stencilObstacleShader.Program, "grid_depth"), GRID_DEPTH);

    glUniform4fv(glGetUniformLocation(stencilObstacleShader.Program, "color"), 1, glm::value_ptr(glm::vec4(0.0f)));

    glCullFace(GL_FRONT);
    obstacle->lowPolyModel->DrawInstanced(GRID_DEPTH);

    glCullFace(GL_BACK);
    obstacle->lowPolyModel->DrawInstanced(GRID_DEPTH);

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF); // pass stencil test only if stencil value is not 0
    glStencilMask(0x00); // disable writing to stencil buffer

    glDisable(GL_CULL_FACE); // draw all faces
    glUniform4fv(glGetUniformLocation(stencilObstacleShader.Program, "color"), 1, glm::value_ptr(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
    obstacle->lowPolyModel->DrawInstanced(GRID_DEPTH);

    // reset state
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0xFF);
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// compute the velocity of the obstacle at each grid cell, by calculating the velocity of each
// vertex of the obstacle with the position of the obstacle at the previous time step, and
// the current time step. because the obstacle is voxelized, we only need to compute the velocity
// of the vertices that are on the surface of the obstacle and splitted for each slice: in
// geometry stage, calculate the intersection of each triangle with the current slice of the 
// 3D texture, and output only the triangles that lays on the current slice, or output a quad 
// if the intersection produces a segment. this quad is obtained by extending the intersection 
// segment along the projection of the triangle normal on the current slice plane (the velocity 
// of their vertices is computed by interpolation), by a factor equal to the diagonal of a grid 
// cell
void DynamicObstacleVelocity(Shader &obstacleVelocityShader, Slab &obstacle_velocity, Slab &dest, ObstacleObject* obstacle, glm::mat4 view, glm::mat4 projection, glm::vec3 firstLayerPoint, glm::vec3 layersDir, GLfloat deltaTime, GLfloat texelDiagonalSize)
{
    glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);
    glClear(GL_COLOR_BUFFER_BIT);

    obstacleVelocityShader.Use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, obstacle_velocity.tex);
    glUniform1i(glGetUniformLocation(obstacleVelocityShader.Program, "ObstacleVelocity"), 0);

    glUniformMatrix4fv(glGetUniformLocation(obstacleVelocityShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(obstacle->modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(obstacleVelocityShader.Program, "prevModel"), 1, GL_FALSE, glm::value_ptr(obstacle->prevModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(obstacleVelocityShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(obstacleVelocityShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glUniform1f(glGetUniformLocation(obstacleVelocityShader.Program, "grid_depth"), GRID_DEPTH);

    glUniform1f(glGetUniformLocation(obstacleVelocityShader.Program, "deltaTime"), deltaTime);
    glUniform1f(glGetUniformLocation(obstacleVelocityShader.Program, "texelDiagonal"), texelDiagonalSize);

    glUniform3fv(glGetUniformLocation(obstacleVelocityShader.Program, "firstLayerPoint"), 1, glm::value_ptr(firstLayerPoint));
    glUniform3fv(glGetUniformLocation(obstacleVelocityShader.Program, "layersDir"), 1, glm::value_ptr(layersDir));

    obstacle->lowPolyModel->DrawInstanced(GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    SwapSlabs(&obstacle_velocity, &dest);
}

// update the obstacle buffers by computing the voxelized obstacle position and velocity 
void DynamicObstacle(Shader &stencilObstacleShader, Shader &obstacleVelocityShader, ObstacleSlab &obstacle_position, Slab &obstacle_velocity, Slab &temp_slab, ObstacleObject* obstacle, glm::vec3 translation, GLfloat scale, GLfloat deltaTime)
{
    glViewport(0,0, GRID_WIDTH, GRID_HEIGHT);

    glm::mat4 projection, view;

    GLfloat far_plane = 100, near_plane = 1; // near and far plane of the orthographic projection
    
    // size of the orthographic projection equal to half the edge 
    // of the cube (easily computed from the scale thanks to unitary cube)
    GLfloat frustumSize = scale; 

    // compute the projection as orthographic projection of the cube
    projection = glm::ortho(-frustumSize, frustumSize, -frustumSize, frustumSize, near_plane, far_plane);

    // compute the view matrix as a lookAt matrix, with the camera in front of the cube
    glm::vec3 viewEye = glm::vec3(translation);
    viewEye.z += (scale + 1.0f);

    glm::vec3 viewCenter = translation;
    viewCenter.x += glm::epsilon<float>(); // avoid gimbal lock
    glm::vec3 viewUp = glm::vec3(0.0f, 1.0f, 0.0f);
    view = glm::lookAt(viewEye, viewCenter, viewUp);

    // compute the obstacle position 
    DynamicObstaclePosition(stencilObstacleShader, obstacle_position, obstacle, view, projection, scale);

    // calculate the front direction of the view frustum
    glm::vec3 projectionDir = glm::normalize(viewCenter - viewEye);

    // calculate a point on the first and last layer of the 3D texture along the view
    // front direction to define their slice planes (point on plane + normal)
    glm::vec3 firstLayerPoint = viewEye + projectionDir * near_plane;
    glm::vec3 lastLayerPoint = viewEye + projectionDir * (2 * scale + 1);

    // compute the obstacle velocity
    DynamicObstacleVelocity(obstacleVelocityShader, obstacle_velocity, temp_slab, obstacle, view, projection, firstLayerPoint, lastLayerPoint - firstLayerPoint, deltaTime, 1.41421f * (2*scale / GRID_WIDTH));
}

///////////////////////// LIQUID SIMULATION FUNCTIONS /////////////////////////////

// initialize the liquid simulation by setting the level set to the initial height
void InitLiquidSimulation(Shader &initLiquidSimShader, Slab &levelSet, GLfloat initialHeight)
{
    glViewport(0,0, GRID_WIDTH, GRID_HEIGHT);

    glBindFramebuffer(GL_FRAMEBUFFER, levelSet.fbo);
    glClear(GL_COLOR_BUFFER_BIT);

    initLiquidSimShader.Use();

    glUniform1f(glGetUniformLocation(initLiquidSimShader.Program, "initialHeight"), glm::clamp(initialHeight, 0.0f, 1.0f));
    glUniform1f(glGetUniformLocation(initLiquidSimShader.Program, "grid_height"), GRID_HEIGHT);

    glBindVertexArray(quadVAO);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// apply the level set damping by adding (a factor of) the level set value in the liquid /
// equilibrium state. This is used to damp the level set value and avoid oscillations 
// caused by the level set advection
void ApplyLevelSetDamping(Shader &dampingLevelSetShader, Slab &levelSet, ObstacleSlab obstacle, Slab &dest, GLfloat dampingFactor, GLfloat equilibriumHeight)
{
    glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);
    glClear(GL_COLOR_BUFFER_BIT);

    dampingLevelSetShader.Use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, levelSet.tex);
    glUniform1i(glGetUniformLocation(dampingLevelSetShader.Program, "LevelSetTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, obstacle.tex);
    glUniform1i(glGetUniformLocation(dampingLevelSetShader.Program, "ObstacleTexture"), 1);

    glUniform1f(glGetUniformLocation(dampingLevelSetShader.Program, "dampingFactor"), glm::clamp(dampingFactor, 0.0f, 1.0f));
    glUniform1f(glGetUniformLocation(dampingLevelSetShader.Program, "equilibriumHeight"), glm::clamp(equilibriumHeight, 0.0f, 1.0f));
    glUniform3fv(glGetUniformLocation(dampingLevelSetShader.Program, "InverseSize"), 1, glm::value_ptr(InverseSize));
    glUniform1f(glGetUniformLocation(dampingLevelSetShader.Program, "grid_height"), GRID_HEIGHT);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    SwapSlabs(&levelSet, &dest);
}

// apply the gravity to the velocity field for those cells that are inside the liquid
void ApplyGravity(Shader &gravityShader, Slab &velocity, Slab &levelSet, Slab &dest, GLfloat gravityAcceleration, GLfloat timeStep, GLfloat threshold)
{
    glBindFramebuffer(GL_FRAMEBUFFER, dest.fbo);
    glClear(GL_COLOR_BUFFER_BIT);

    gravityShader.Use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, velocity.tex);
    glUniform1i(glGetUniformLocation(gravityShader.Program, "VelocityTexture"), 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, levelSet.tex);
    glUniform1i(glGetUniformLocation(gravityShader.Program, "LevelSetTexture"), 1);

    glUniform3fv(glGetUniformLocation(gravityShader.Program, "InverseSize"), 1, glm::value_ptr(InverseSize));
    glUniform1f(glGetUniformLocation(gravityShader.Program, "gravityAcceleration"), gravityAcceleration);
    glUniform1f(glGetUniformLocation(gravityShader.Program, "timeStep"), timeStep);
    glUniform1f(glGetUniformLocation(gravityShader.Program, "levelSetThreshold"), threshold);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    SwapSlabs(&velocity, &dest);
}

