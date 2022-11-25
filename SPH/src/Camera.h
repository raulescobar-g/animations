#pragma  once
#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

struct Active {};

struct Camera {
	Camera();
	glm::vec4 perspective; // aspect, fovy, near, far
	glm::vec3 position;
	glm::vec2 rotation; // yaw, pitch
	glm::vec3 up;

	unsigned char inputs[256];
	float sensitivity, movement_speed;
};
	
glm::mat4 projectionMat(const Camera& camera);
glm::mat4 viewMat(const Camera& camera);
glm::vec3 dir(const glm::vec2& rotation);
#endif
