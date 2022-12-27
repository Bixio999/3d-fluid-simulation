// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <utils/model.h>

#pragma once

struct ObstacleObject
{
    glm::mat4 modelMatrix;
    glm::mat4 prevModelMatrix;
    Model* objectModel;
    Model* lowPolyModel;

    // for animation and placement
    glm::vec3 position;
    glm::vec3 scale;
    string name;
    bool isActive;

    ~ObstacleObject()
    {
        if (objectModel == lowPolyModel)
            delete lowPolyModel;
        else
        {
            delete lowPolyModel;
            delete objectModel;
        }
    }
};