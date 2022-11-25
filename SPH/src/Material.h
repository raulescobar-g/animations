#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

struct Material {
    Material() : ka(0.5f), kd(0.5f), ks(0.5f), s(10.0f), a(1.0f) {};
    Material(glm::vec3 ka) : ka(ka), kd(0.5f), ks(0.5f), s(10.0f), a(1.0f) {};
    glm::vec3 ka,kd,ks;
    float s,a;
};

#endif