#pragma once

#ifndef ANIMATIONDATA_H
#define ANIMATIONDATA_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


class AnimationData {
    public: 
    AnimationData() = default;
    AnimationData(std::string filename){
        std::ifstream in;
        in.open(filename);
        if(!in.good()) {
            std::cout << "Cannot read: " << filename << std::endl;
            return;
        }
        std::cout << "Loading: " << filename << std::endl;
        
        std::string line;

        for (int i = 0; i < 4; ++i) std::getline(in, line); //comments

        

        std::stringstream ss(line);
        std::string frames_s, bones_s;
        ss >> frames_s;
        ss >> bones_s;

        bones = std::stoi(bones_s);
        frames = std::stoi(frames_s);
        
        std::string qx, qy, qz, qw, x0, x1, x2;
        for (int i = 0; i < frames; ++i) {

            std::getline(in, line);
            std::stringstream ss(line);

            if(in.eof()) {
                break;
            }
            if(line.empty()) {
                continue;
            }
            // Skip comments
            if(line.at(0) == '#') {
                continue;
            }

            for (int j = 0; j < bones; ++j) {            
                
                ss >> qx;
                ss >> qy;
                ss >> qz;
                ss >> qw;
                ss >> x0;
                ss >> x1;
                ss >> x2;

                glm::mat4 T = glm::mat4_cast(glm::quat(std::stof(qw) ,std::stof(qx) ,std::stof(qy) ,std::stof(qz)));
                T[3] = glm::vec4(glm::vec3(std::stof(x0), std::stof(x1), std::stof(x2)) ,1.0f);
                transforms.push_back(T);
            }
        }
        in.close();
    }


    std::vector<glm::mat4> transforms;
    int bones, frames;

};

#endif