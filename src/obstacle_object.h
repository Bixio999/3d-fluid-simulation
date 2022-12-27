// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// we load the model class
#include <utils/model.h>

#pragma once

// we define the structure for the dynaic objects
struct ObstacleObject
{
    glm::mat4 modelMatrix; // current model matrix
    glm::mat4 prevModelMatrix; // previous model matrix (used for object motion in simulation)
    Model* objectModel; // model of the object used for rendering in scene
    Model* lowPolyModel; // model of the object used in the simulation

    // for animation and placement

    glm::vec3 position; // position of the object
    glm::vec3 scale; // scale of the object
    string name; // name of the object visualized in the ui
    bool isActive; // if the object is active or not

    // destructor
    ~ObstacleObject()
    {
        // we delete the models
        // if the models are the same, we delete only one
        if (objectModel == lowPolyModel)
            delete lowPolyModel;
        else
        {
            delete lowPolyModel;
            delete objectModel;
        }
    }
};