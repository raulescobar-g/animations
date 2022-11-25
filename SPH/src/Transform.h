#pragma once

#ifndef TRANSFORM_H
#define TRANSFORM_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct Transform {
    
    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;

    operator glm::mat4() {
        return glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
    }
};

#endif