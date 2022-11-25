#pragma once
#ifndef MESH_H
#define MESH_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector> 
#include <string>

#include "Program.h"

struct Mesh {
    Mesh();
    Mesh(std::string);
    std::vector<glm::vec3> operator *(glm::vec3 scale);

    std::vector<glm::vec3> posBuf;
    std::vector<glm::vec3> norBuf;
    std::vector<glm::vec2> texBuf;
    std::vector<unsigned int> indBuf;
    unsigned posBufID;
    unsigned norBufID;
    unsigned texBufID;
    unsigned indBufID;
};

void init(Mesh& mesh);
void draw(const Program& prog, const Mesh& mesh);
void loadMesh(const std::string &meshName, Mesh& mesh);
void fitToUnitBox(Mesh& mesh);


#endif