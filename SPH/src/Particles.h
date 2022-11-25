#pragma once

#ifndef PARTICLES_H
#define PARTICLES_H

#include <glm/glm.hpp>
#include "Mesh.h"
#include "Program.h"

struct Particles {
    Particles(): size(500) { init(); };
    Particles(int n): size(n) { init(); };
    ~Particles();

    int size;
    glm::vec3 gravity = glm::vec3(0.0f, -10.0f, 0.0f);
    float mpp = 0.1f;
    float rho = 1.0f;
    float nu = 0.01f;
    float r = 0.1f;
    float sigma = 1.0f;
    float k = 0.03f;

    glm::vec4 * state;
    glm::vec4 * dstate; 

    float * pressure;
    float * density;
    glm::vec4 * viscosity;
    glm::vec4 * tension;

    void init();
    void update(float dt);
    void render(const Program& prog, const Mesh& sphere);

    float W_poly6(float r, float h);
    float W_spiky(float r, float h);
    float W_visc(float r, float h);

    unsigned int posSSbo;
};

#endif