#include "Simulation.h"

#include <memory>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

#include "Mesh.h"
#include "Material.h"
#include "Entity.h"
#include "MatrixStack.h"
#include "Camera.h"
#include "Transform.h"

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
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

Entity Simulation::create_entity(std::string tag){
	Entity entity(registry.create(), this);
	entity.add_component<Tag>(tag);
	return entity;
}

void Simulation::create_window(const char * window_name, const char * glsl_version) {
    // Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(480 , 480 , window_name, NULL, NULL);
	if(!window) {
		glfwTerminate();
		std::cout<<"Failed to setup window! \n";
		exit(-1);
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	

	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    //ImGui_ImplOpenGL3_Init(glsl_version);
	 ImGui_ImplOpenGL2_Init();
	glfwSwapInterval(1);
}

void Simulation::init_programs(){
	std::vector<std::string> mesh_attributes = {"aPos", "aNor"};
	std::vector<std::string> mesh_uniforms = {"MV", "iMV", "P", "ka", "kd", "ks", "s", "a", "lightPos"};

	pbr_program = Program("phong_vert.glsl", "phong_frag.glsl", mesh_attributes, mesh_uniforms);
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
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Info: ");
	ImGui::Text("%.3f dt", dt);
	ImGui::Text("%.1f frametime", 1000.0f / ImGui::GetIO().Framerate);
	if (ImGui::Button("Cull backfaces"))
		options[(unsigned) 'c'] = !options[(unsigned) 'c'];
	if (ImGui::Button("Wireframes"))
		options[(unsigned) 'v'] = !options[(unsigned) 'v'];
	if (ImGui::Button("Camera Lock"))
		options[(unsigned) 'x'] = !options[(unsigned) 'x'];
	if (ImGui::Button("Reset"))
		options[(unsigned) 'r'] = !options[(unsigned) 'r'];
	if (ImGui::Button("Pause"))
		options[(unsigned) 'p'] = !options[(unsigned) 'p'];

	ImGui::End();
	ImGui::Render();

	
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
	
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

	ImVec2 mouse = ImGui::GetMousePos();

	if (options[(unsigned) 'x']) {
		if (o_x > 0.0 && o_y > 0.0) {
			float xdiff = (mouse.x - o_x) * camera.sensitivity;
			float ydiff = (mouse.y - o_y) * camera.sensitivity;

			camera.rotation.x -= xdiff;
			camera.rotation.y -= ydiff;

			camera.rotation.y = glm::min(pi/2.0f, camera.rotation.y);
			camera.rotation.y = glm::max(-pi/2.0f, camera.rotation.y);
		}
		o_x = mouse.x;
		o_y = mouse.y;
	} else {
		o_x = -1.0;
		o_y = -1.0;
	}
}

void Simulation::input_capture(){
	
	if (ImGui::IsKeyDown(ImGuiKey_Escape)) { 
		glfwSetWindowShouldClose(window, GL_TRUE);
	} 	

	auto view = registry.view<Camera, Active>();
	auto entity = view.front();
	auto& camera = view.get<Camera>(entity);

	
	camera.inputs[(unsigned)'w'] = ImGui::IsKeyDown(ImGuiKey_W);								 
	camera.inputs[(unsigned)'s'] = ImGui::IsKeyDown(ImGuiKey_S);
	camera.inputs[(unsigned)'d'] = ImGui::IsKeyDown(ImGuiKey_D);
	camera.inputs[(unsigned)'a'] = ImGui::IsKeyDown(ImGuiKey_A);
	camera.inputs[(unsigned)'q'] = ImGui::IsKeyDown(ImGuiKey_Space);
	camera.inputs[(unsigned)'e'] = ImGui::IsKeyDown(ImGuiKey_LeftShift);
	camera.inputs[(unsigned)'Z'] = ImGui::IsKeyDown(ImGuiKey_Z) && ImGui::IsKeyDown(ImGuiKey_LeftShift);
	camera.inputs[(unsigned)'z'] = ImGui::IsKeyDown(ImGuiKey_Z) && !ImGui::IsKeyDown(ImGuiKey_LeftShift);
	
	options[(unsigned) 'x'] = ImGui::IsKeyReleased(ImGuiKey_F) ? !options[(unsigned) 'x'] : options[(unsigned) 'x'];
	options[(unsigned)'c'] = ImGui::IsKeyReleased(ImGuiKey_C) ? !options[(unsigned) 'c'] : options[(unsigned) 'c'];
	options[(unsigned)'v'] = ImGui::IsKeyReleased(ImGuiKey_V) ? !options[(unsigned) 'v'] : options[(unsigned) 'v'];
	options[(unsigned)'p'] = ImGui::IsKeyReleased(ImGuiKey_P) ? !options[(unsigned) 'p'] : options[(unsigned) 'p'];
	
	

	if (options[(unsigned) 'r']) {
		std::cout<<"fix the reset\n";
		options[(unsigned) 'r'] = false;
	}
}
