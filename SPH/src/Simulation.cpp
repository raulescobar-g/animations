#include "Simulation.h"

#include <memory>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Mesh.h"
#include "Material.h"
#include "Entity.h"
#include "MatrixStack.h"
#include "Camera.h"
#include "Transform.h"
#include "Particles.h"

#define pi 3.141592653589f

std::ostream& operator<< (std::ostream &out, glm::vec3 const& x) {
	out<<"<"<<x.x<<", "<<x.y<<", "<<x.z<<">\n";
	return out;
}
std::ostream& operator<< (std::ostream &out, glm::vec4 const& x) {
	out<<"<"<<x.x<<", "<<x.y<<", "<<x.z<<", "<< x.w<<">\n";
	return out;
}

Simulation::Simulation() {
	for (int i = 0; i < 256; ++i) {
		options[i] = false;
	}
}

Simulation::~Simulation() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

Entity Simulation::create_entity(std::string tag){
	Entity entity(registry.create(), this);
	entity.add_component<Tag>(tag);
	return entity;
}

void Simulation::create_window(const char * window_name) {
    // Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(480 , 480 , window_name, NULL, NULL);
	if(!window) {
		glfwTerminate();
		std::cout<<"Failed to setup window! \n";
		exit(-1);
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
}

void Simulation::init_programs(){
	std::vector<std::string> mesh_attributes = {"aPos", "aNor"};
	std::vector<std::string> mesh_uniforms = {"MV", "iMV", "P", "ka", "kd", "ks", "s", "a", "lightPos"};

	pbr_program = Program("phong_vert.glsl", "phong_frag.glsl", mesh_attributes, mesh_uniforms);

	std::vector<std::string> fluid_attributes = {"aPos", "aNor"};
	std::vector<std::string> fluid_uniforms = {"MV", "iMV", "P", "ka", "kd", "ks", "s", "a", "lightPos"};

fluid_program = Program("phong_instanced_vert.glsl", "phong_instanced_frag.glsl", fluid_attributes, fluid_uniforms);
}

void Simulation::init_cameras(){
	auto&& camera = create_entity("Main camera");
	camera.add_component<Camera>();
	camera.add_tag_component<Active>();
}

void Simulation::set_scene() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	auto fluid = create_entity("Fluid");
	fluid.add_component<Particles>(100);
	fluid.add_component<Mesh>("sphere.obj");
	fluid.add_component<Material>(glm::vec3(0.1f, 0.3f, 0.85f));

	// set all time params
	current_time = glfwGetTime();
	total_time = 0.0f;
}

void Simulation::fixed_timestep_update() {
	new_time = glfwGetTime();
	frame_time = new_time - current_time;
	current_time = new_time;
	total_time += frame_time;

	while (total_time >= dt) {
		if (!options[(unsigned) 'p']) update();
		total_time -= dt;
	}
}

void Simulation::integrate(float h) {

} 

void Simulation::update() {
	auto view = registry.view<Particles>();
	auto entity = view.front();
	auto& particles = view.get<Particles>(entity);
	particles.update(dt);
}


void Simulation::resize_window() {
	// try putting this elsewhere
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
}

void Simulation::render_scene() {
	auto view = registry.view<Camera, Active>();
	auto entity = view.front();
	auto& camera = view.get<Camera>(entity);

	glfwPollEvents();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Matrix stacks
	MatrixStack P;
	MatrixStack MV;
	MV.pushMatrix();
	P.pushMatrix();

	P *= projectionMat(camera);
	MV *= viewMat(camera);

	draw_entities(MV, P);

	MV.popMatrix();
	P.popMatrix();
}

void Simulation::draw_entities(MatrixStack& MV, MatrixStack& P) {
	glm::mat4 iMV;
	glm::vec3 world_light_pos = MV * lightPos;

	pbr_program.bind();
	for (auto&& [entity, mesh, material, transform]: registry.view<Mesh, Material, Transform>().each()) {
		P.pushMatrix();
		MV.pushMatrix();

			MV *= (glm::mat4) transform;
			iMV = glm::transpose(glm::inverse(MV.topMatrix()));

			glUniform3f(pbr_program.getUniform("lightPos"), world_light_pos.x, world_light_pos.y, world_light_pos.z);
			glUniformMatrix4fv(pbr_program.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P.topMatrix()));
			glUniformMatrix4fv(pbr_program.getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV.topMatrix()));
			glUniformMatrix4fv(pbr_program.getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));
			glUniform3f(pbr_program.getUniform("ka"), material.ka.x, material.ka.y, material.ka.z);
			glUniform3f(pbr_program.getUniform("kd"), material.kd.x, material.kd.y, material.kd.z);
			glUniform3f(pbr_program.getUniform("ks"), material.ks.x, material.ks.y, material.ks.z);
			glUniform1f(pbr_program.getUniform("s"), material.s );
			glUniform1f(pbr_program.getUniform("a"), material.a );
			draw(pbr_program, mesh);

		MV.popMatrix();	
		P.popMatrix();
	}
	pbr_program.unbind();




	fluid_program.bind();
	for (auto&& [entity, fluid, mesh, material] : registry.view<Particles, Mesh, Material>().each()) {
		P.pushMatrix();
		MV.pushMatrix();

		iMV = glm::transpose(glm::inverse(MV.topMatrix()));

		glUniform3f(fluid_program.getUniform("lightPos"), world_light_pos.x, world_light_pos.y, world_light_pos.z);
		glUniformMatrix4fv(fluid_program.getUniform("P"), 1, GL_FALSE, glm::value_ptr(P.topMatrix()));
		glUniformMatrix4fv(fluid_program.getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV.topMatrix()));
		glUniformMatrix4fv(fluid_program.getUniform("iMV"), 1, GL_FALSE, glm::value_ptr(iMV));
		glUniform3f(fluid_program.getUniform("ka"), material.ka.x, material.ka.y, material.ka.z);
		glUniform3f(fluid_program.getUniform("kd"), material.kd.x, material.kd.y, material.kd.z);
		glUniform3f(fluid_program.getUniform("ks"), material.ks.x, material.ks.y, material.ks.z);
		glUniform1f(fluid_program.getUniform("s"), material.s );
		glUniform1f(fluid_program.getUniform("a"), material.a );
		fluid.render(fluid_program, mesh);

		MV.popMatrix();	
		P.popMatrix();
	} 
	fluid_program.unbind();
}

bool Simulation::window_closed() {
	return glfwWindowShouldClose(window);
}

void Simulation::swap_buffers() {
	glfwSwapBuffers(window);
}

void Simulation::move_camera() {
	glm::vec3 buff(0.0f,0.0f,0.0f);

	auto view = registry.view<Camera, Active>();
	auto entity = view.front();
	auto& camera = view.get<Camera>(entity);

	glm::vec3 facing = dir(camera.rotation);

	if (camera.inputs[(unsigned)'w']) 
		buff += glm::normalize(facing - glm::dot(camera.up, facing) * camera.up); 
	
	if (camera.inputs[(unsigned)'s']) 
		buff -= glm::normalize(facing - glm::dot(camera.up, facing) * camera.up); 
	
	if (camera.inputs[(unsigned)'a']) 
		buff -= glm::normalize(glm::cross(facing, camera.up));
	
	if (camera.inputs[(unsigned)'d']) 
		buff += glm::normalize(glm::cross(facing, camera.up));
	
	if (camera.inputs[(unsigned)'q']) 
		buff += camera.up;
	
	if (camera.inputs[(unsigned)'e']) 
		buff -= camera.up;
	

	if (glm::length(buff) > eps) 
		camera.position += glm::normalize(buff) * camera.movement_speed; 
	

	!options[(unsigned)'c'] ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	options[(unsigned)'v'] ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	if (camera.inputs[(unsigned) 'z']) 
		camera.perspective.y = glm::max(camera.perspective.y - 0.01f,4.0f * pi/180.0f);

	if (camera.inputs[(unsigned) 'Z']) 
		camera.perspective.y = glm::min(camera.perspective.y + 0.01f, 114.0f * pi/180.0f);
}

void Simulation::error_callback_impl(int error, const char *description) { 
	std::cerr << description << std::endl; 
}

void Simulation::look_around() {
	auto view = registry.view<Camera, Active>();
	auto entity = view.front();
	auto& camera = view.get<Camera>(entity);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (options[(unsigned) 'x']) {
		if (o_x > 0.0 && o_y > 0.0) {
			float xdiff = (xpos - o_x) * camera.sensitivity;
			float ydiff = (ypos - o_y) * camera.sensitivity;

			camera.rotation.x -= xdiff;
			camera.rotation.y -= ydiff;

			camera.rotation.y = glm::min(pi/2.0f, camera.rotation.y);
			camera.rotation.y = glm::max(-pi/2.0f, camera.rotation.y);
		}
		o_x = xpos;
		o_y = ypos;
	} else {
		o_x = -1.0;
		o_y = -1.0;
	}
}

void Simulation::input_capture(){
	
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { 
		glfwSetWindowShouldClose(window, GL_TRUE);
	} 	

	auto view = registry.view<Camera, Active>();
	auto entity = view.front();
	auto& camera = view.get<Camera>(entity);

	
	camera.inputs[(unsigned)'w'] = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;								 
	camera.inputs[(unsigned)'s'] = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
	camera.inputs[(unsigned)'d'] = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
	camera.inputs[(unsigned)'a'] = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
	camera.inputs[(unsigned)'q'] = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
	camera.inputs[(unsigned)'e'] = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
	
	// options[(unsigned) 'x'] = ImGui::IsKeyReleased(ImGuiKey_F) ? !options[(unsigned) 'x'] : options[(unsigned) 'x'];
	// options[(unsigned)'c'] = ImGui::IsKeyReleased(ImGuiKey_C) ? !options[(unsigned) 'c'] : options[(unsigned) 'c'];
	// options[(unsigned)'v'] = ImGui::IsKeyReleased(ImGuiKey_V) ? !options[(unsigned) 'v'] : options[(unsigned) 'v'];
	// options[(unsigned)'p'] = ImGui::IsKeyReleased(ImGuiKey_P) ? !options[(unsigned) 'p'] : options[(unsigned) 'p'];
	
	if (options[(unsigned) 'r']) {
		std::cout<<"fix the reset\n";
		options[(unsigned) 'r'] = false;
	}
}
