#pragma  once
#ifndef Object_H
#define Object_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

class Object { 
    public:
	Object(std::shared_ptr<Shape> shape, glm::vec3 position, glm::quat rotation, float scale, int rotate=0): 
		shape(shape), position(position), rotation(rotation), scale(scale), rotate(rotate) {};
	std::shared_ptr<Shape> shape;
	glm::vec3 position;
	glm::quat rotation;
	float scale;
	int rotate;
};

#endif