#pragma  once
#ifndef Keyframes_H
#define Keyframes_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

class Keyframe {
    public:
	Keyframe(glm::vec3 pos, glm::quat quat): pos(pos), quat(quat) {};
	glm::vec3 pos;
	glm::quat quat;
};

#endif