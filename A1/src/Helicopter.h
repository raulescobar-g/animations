#pragma  once
#ifndef Helicopter_H
#define Helicopter_H

#include "Shape.h"
#include "Object.h"

class Helicopter {
    public:
	Helicopter() {};
    void init(std::string resource_dir) {
        body1 = std::make_shared<Shape>();
        body1->loadMesh(resource_dir + "helicopter_body1.obj");
        body1->init();
        parts.push_back(Object(body1, glm::vec3(0.0f, 0.0f, 0.0f),glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 1.0f));

        prop1 = std::make_shared<Shape>();
        prop1->loadMesh(resource_dir + "helicopter_prop1.obj");
        prop1->init();
        parts.push_back(Object(prop1, glm::vec3(0.0f, 0.0f, 0.0f),glm::angleAxis(0.0001f, glm::vec3(0.0f, 0.4819f, 0.0f)), 1.0f, 1));

        body2 = std::make_shared<Shape>();
        body2->loadMesh(resource_dir + "helicopter_body2.obj");
        parts.push_back(Object(body2, glm::vec3(0.0f, 0.0f, 0.0f),glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 1.0f));
        body2->init();

        prop2 = std::make_shared<Shape>();
        prop2->loadMesh(resource_dir + "helicopter_prop2.obj");
        prop2->init();
        parts.push_back(Object(prop2, glm::vec3(0.0f, 0.0f, 0.0f),glm::angleAxis(0.0001f, glm::vec3(0.6228f, 0.1179f, 0.1365f)), 1.0f, 2));
	
    }

    std::vector<Object> parts;
    std::shared_ptr<Shape> body1;
    std::shared_ptr<Shape> prop1;
    std::shared_ptr<Shape> body2;
    std::shared_ptr<Shape> prop2;
    
};


#endif