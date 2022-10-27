#include "fluid-sim.h"

// Std. Includes
#include <string>

//////////////////////////////////////

float GRID_WIDTH;
float GRID_HEIGHT;
float GRID_DEPTH;
glm::vec3 InverseSize;



GLuint quadVAO = 0;

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

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
         1,  1,

        -1, -1, 
         1,  1, 
        -1,  1 
    };
    glGenBuffers(1, &quad_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, 2 * sizeof(short), 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// initialize the simulation shaders and data structures
void InitSimulation()
{
    // create the quad vao
    CreateQuadVAO();
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
void Advect(Shader* advectionShader, Slab *velocity, Slab *source, Slab *dest, float dissipation, float timeStep)
{
    advectionShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, velocity->tex);
    glUniform1i(glGetUniformLocation(advectionShader->Program, "VelocityTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, source->tex);
    glUniform1i(glGetUniformLocation(advectionShader->Program, "SourceTexture"), 1);

    glUniform1f(glGetUniformLocation(advectionShader->Program, "timeStep"), timeStep);
    glUniform3fv(glGetUniformLocation(advectionShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));
    glUniform1f(glGetUniformLocation(advectionShader->Program, "dissipation"), dissipation);

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);

    // SwapSlabs(source, dest);
}

void AdvectMacCormack(Shader* advectionShader, Shader* macCormackShader, Slab *velocity, Slab *phi1_hat, Slab *phi2_hat, Slab* source, Slab* dest, float dissipation, float timeStep)
{
    // first advection pass - compute phi1_hat
    Advect(advectionShader, velocity, source, phi1_hat, dissipation, timeStep);

    // second advection pass - compute phi2_hat
    Advect(advectionShader, velocity, phi1_hat, phi2_hat, 1 / dissipation, -timeStep);

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

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GRID_DEPTH);

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

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GRID_DEPTH);

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

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GRID_DEPTH);

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

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GRID_DEPTH);

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

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(temperature, dest);
}

// execute divergence
void Divergence(Shader *divergenceShader, Slab *velocity, Slab *divergence, Slab *dest)
{
    divergenceShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, velocity->tex);
    glUniform1i(glGetUniformLocation(divergenceShader->Program, "VelocityTexture"), 0);
    glUniform3fv(glGetUniformLocation(divergenceShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(divergence, dest);
}

// execute jacobi
void Jacobi(Shader *jacobiShader, Slab *pressure, Slab *divergence, Slab *dest, GLuint iterations)
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
        glUniform3fv(glGetUniformLocation(jacobiShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GRID_DEPTH);

        SwapSlabs(pressure, dest);
    }

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);
}


// apply pressure
void ApplyPressure(Shader *pressureShader, Slab *velocity, Slab *pressure, Slab *dest)
{
    pressureShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, dest->fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, velocity->tex);
    glUniform1i(glGetUniformLocation(pressureShader->Program, "VelocityTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, pressure->tex);
    glUniform1i(glGetUniformLocation(pressureShader->Program, "PressureTexture"), 1);
    glUniform3fv(glGetUniformLocation(pressureShader->Program, "InverseSize"), 1, glm::value_ptr(InverseSize));

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GRID_DEPTH);

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);

    SwapSlabs(velocity, dest);
}

/////////////////////////////////////////////

// draw raydata texture for fluid rendering
void RayData(Shader* raydataShader, Model &cubeModel, Slab *raydata, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection)
{
    glEnable(GL_CULL_FACE);

    raydataShader->Use();

    glBindFramebuffer(GL_FRAMEBUFFER, raydata->fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(glGetUniformLocation(raydataShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(raydataShader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(raydataShader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(raydataShader->Program, "grid_size"), 1, glm::value_ptr(glm::vec3(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH)));

    glCullFace(GL_FRONT);

    cubeModel.Draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_CULL_FACE);
}

// render front faces of the cube
void RenderFluid(Shader* renderShader, Model &cubeModel, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection, Slab *raydata, Slab *density, glm::vec2 inverseScreenSize)
{
    glEnable(GL_CULL_FACE);

    renderShader->Use();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, raydata->tex);
    glUniform1i(glGetUniformLocation(renderShader->Program, "RayDataTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, density->tex);
    glUniform1i(glGetUniformLocation(renderShader->Program, "DensityTexture"), 1);

    glUniformMatrix4fv(glGetUniformLocation(renderShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(renderShader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(renderShader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(renderShader->Program, "grid_size"), 1, glm::value_ptr(glm::vec3(GRID_WIDTH, GRID_HEIGHT, GRID_DEPTH)));
    glUniform2fv(glGetUniformLocation(renderShader->Program, "InverseSize"), 1, glm::value_ptr(inverseScreenSize));

    glCullFace(GL_BACK);

    cubeModel.Draw();

    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_3D, 0);

    glDisable(GL_CULL_FACE);
}