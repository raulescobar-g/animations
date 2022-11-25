#include <iostream>
#include <string>
#define GLEW_STATIC
#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"
#include "Simulation.h"


int main(int argc, char **argv) {

	Simulation &sim = Simulation::get_instance();

	glfwSetErrorCallback(&Simulation::error_callback);
	if(!glfwInit()) return -1;

	sim.create_window("Smooth particle hydrodynamics");

	glewExperimental = true;
	if(glewInit() != GLEW_OK) return -1;
	
	glGetError();
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// GLuint vaoId = 0;
	// glGenVertexArrays(1, &vaoId);
	// glBindVertexArray(vaoId);

	sim.init_programs();
	sim.init_cameras();
	sim.set_scene();	
	while (!sim.window_closed()) {
		sim.resize_window();
		sim.input_capture();
		sim.look_around();
		sim.move_camera();
		sim.fixed_timestep_update();
		sim.render_scene();
		sim.swap_buffers();
	}
	return 0;
}
