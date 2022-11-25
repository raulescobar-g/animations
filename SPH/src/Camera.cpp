#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"



Camera::Camera() :
	perspective(1.0f, 45.0f*glm::pi<float>()/180.0f, 0.1f, 100000.0f),
	position(0.0f,0.0f,-10.0f), rotation(0.0f), up(0.0f, 1.0f, 0.0f), sensitivity(0.005f), movement_speed(0.05f) {
	for (int i = 0; i < 256; ++i) {
		inputs[i] = false;
	}
}

glm::mat4 projectionMat(const Camera& camera) {
	return glm::perspective(camera.perspective.y, camera.perspective.x, camera.perspective.z, camera.perspective.w);
}

glm::mat4 viewMat(const Camera& camera) {
	return glm::lookAt(camera.position, camera.position + dir(camera.rotation), camera.up );
}

glm::vec3 dir(const glm::vec2& rotation) {
	return glm::normalize(glm::vec3(sin(rotation.x)*cos(rotation.y), sin(rotation.y), cos(rotation.x)*cos(rotation.y)));
}
