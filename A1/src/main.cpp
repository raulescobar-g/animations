#include <iostream>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Camera.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = ""; // Where the resources are loaded from

int keyPresses[256] = {0}; // only for English keyboards!

shared_ptr<Program> prog;
shared_ptr<Camera> camera;
shared_ptr<Shape> body1;
shared_ptr<Shape> prop1;
shared_ptr<Shape> body2;
shared_ptr<Shape> prop2;

double t, dt;

glm::mat4 B, G;
float c;


struct Object { 
	Object(shared_ptr<Shape> shape, glm::vec3 position, glm::quat rotation, float scale, bool rotate=false): 
		shape(shape), position(position), rotation(rotation), scale(scale), rotate(rotate) {};
	shared_ptr<Shape> shape;
	glm::vec3 position;
	glm::quat rotation;
	float scale;
	bool rotate;
};

struct Helicopter {
	Helicopter() {};
};

vector<Object> objects;
vector<glm::vec3> keyframes;

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyPresses[key]++;
}

static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		camera->mouseMoved(xmouse, ymouse);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(action == GLFW_PRESS) {
		bool shift = mods & GLFW_MOD_SHIFT;
		bool ctrl  = mods & GLFW_MOD_CONTROL;
		bool alt   = mods & GLFW_MOD_ALT;
		camera->mouseClicked(xmouse, ymouse, shift, ctrl, alt);
	}
}

int mod(int a, int b) {
	return a < 0 ? mod(a+b, b) : a%b;
}

static void init()
{
	GLSL::checkVersion();
	
	// Set background color
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test
	glEnable(GL_DEPTH_TEST);

	c  = 1.0/2.0f;
	B[0] = glm::vec4(0.0f, 2.0f, 0.0f, 0.0f);
	B[1] = glm::vec4(-1.0f, 0.0f, 1.0f, 0.0f);
	B[2] = glm::vec4(2.0f, -5.0f, 4.0f, -1.0f);
	B[3] = glm::vec4(-1.0f, 3.0f, -3.0f, 1.0f);

	keyPresses[(unsigned)'c'] = 1;
	
	prog = make_shared<Program>();
	prog->setShaderNames(RESOURCE_DIR + "phong_vert.glsl", RESOURCE_DIR + "phong_frag.glsl");
	prog->setVerbose(true);
	prog->init();
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addUniform("lightPos");
	prog->addUniform("ka");
	prog->addUniform("kd");
	prog->addUniform("ks");
	prog->addUniform("s");
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->setVerbose(false);
	
	camera = make_shared<Camera>();
	
	body1 = make_shared<Shape>();
	body1->loadMesh(RESOURCE_DIR + "helicopter_body1.obj");
	body1->init();
	objects.push_back(Object(body1, glm::vec3(0.0f, 0.0f, 0.0f),glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 1.0f));

	prop1 = make_shared<Shape>();
	prop1->loadMesh(RESOURCE_DIR + "helicopter_prop1.obj");
	prop1->init();
	objects.push_back(Object(prop1, glm::vec3(0.0f, 0.0f, 0.0f),glm::angleAxis(0.0001f, glm::vec3(0.0f, 0.4819f, 0.0f)), 1.0f, true));

	body2 = make_shared<Shape>();
	body2->loadMesh(RESOURCE_DIR + "helicopter_body2.obj");
	objects.push_back(Object(body2, glm::vec3(0.0f, 0.0f, 0.0f),glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 1.0f));
	body2->init();

	prop2 = make_shared<Shape>();
	prop2->loadMesh(RESOURCE_DIR + "helicopter_prop2.obj");
	prop2->init();
	objects.push_back(Object(prop2, glm::vec3(0.0f, 0.0f, 0.0f),glm::angleAxis(0.0001f, glm::vec3(0.6228f, 0.1179f, 0.1365f)), 1.0f, true));
	
	keyframes.push_back(glm::vec3(-1.0f, 0.0f, 1.0f));
	keyframes.push_back(glm::vec3(0.0f, 3.0f, -1.0f));
	keyframes.push_back(glm::vec3(7.0f, 0.0f, 2.0f));
	keyframes.push_back(glm::vec3(0.0f, -2.0f, -2.0f));
	keyframes.push_back(glm::vec3(-8.0f, -8.0f, 0.0f));

	// Initialize time.
	glfwSetTime(0.0);
	t = 0.0;
	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);
}

void update() {
	t = glfwGetTime();
	for (Object& object: objects) {
		if (object.rotate) {
			float angle = t;
			//object.rotation *= glm::angleAxis((float) 0.01f, glm::axis(object.rotation));
		}
	}
}

void render()
{
	// Update time.
	dt = glfwGetTime() - t;
	t += dt;
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	
	// Use the window size for camera.
	glfwGetWindowSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	
	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyPresses[(unsigned)'c'] % 2) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyPresses[(unsigned)'z'] % 2) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	
	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	camera->applyViewMatrix(MV);

	float u = std::fmod(t, (float)keyframes.size());

	int size = (int)keyframes.size();
	G[0] = glm::vec4(keyframes[mod((int)u - 1, size)], 0.0f);
	G[1] = glm::vec4(keyframes[mod((int)u , size)], 0.0f);
	G[2] = glm::vec4(keyframes[mod((int)u + 1, size)], 0.0f);
	G[3] = glm::vec4(keyframes[mod((int)u + 2, size)], 0.0f);
	
	float k = u - std::floor(u);
	glm::vec4 uVec(1.0f, k, k*k, k*k*k);
	// Compute position at u
	glm::vec4 offset = c * G*(B*uVec);
	

	for (const Object& object : objects) {
		MV->pushMatrix();
			
			MV->translate(object.position + glm::vec3(offset));
			MV->scale(object.scale, object.scale, object.scale);
			MV->multMatrix(glm::mat4_cast(object.rotation));
			prog->bind();
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniform3f(prog->getUniform("kd"), 1.0f, 0.0f, 0.0f);
			object.shape->draw(prog);
			prog->unbind();
		MV->popMatrix();
	}
	
	// Draw the frame and the grid with OpenGL 1.x (no GLSL)
	
	// Setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(P->topMatrix()));
	
	// Setup the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(MV->topMatrix()));
	
	// Draw frame
	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(1, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1);
	glEnd();

	glBegin(GL_LINE_STRIP);
	if(keyPresses[(unsigned)'k'] % 2) {
		glColor3f(0, 1, 0);
		for (int j = -1; j < ((int)keyframes.size()) - 1; ++j) {
			
			int size = (int)keyframes.size();
			G[0] = glm::vec4(keyframes[mod(j, size)], 0.0f);
			G[1] = glm::vec4(keyframes[mod(j+1, size)], 0.0f);
			G[2] = glm::vec4(keyframes[mod(j+2, size)], 0.0f);
			G[3] = glm::vec4(keyframes[mod(j+3, size)], 0.0f);
			
			
			for(int u_i = 0; u_i < 11; ++u_i) {
				float u = u_i / 10.0f;
				glm::vec4 uVec(1.0f, u, u*u, u*u*u);
				// Compute position at u
				glm::vec4 p = c * G*(B*uVec);
				glVertex3f(p.x, p.y, p.z);
			}
		}
	}
	glEnd();

	glLineWidth(1);
	// Draw grid
	float gridSizeHalf = 20.0f;
	int gridNx = 40;
	int gridNz = 40;
	glLineWidth(1);
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_LINES);
	for(int i = 0; i < gridNx+1; ++i) {
		float alpha = i / (float)gridNx;
		float x = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;
		glVertex3f(x, 0, -gridSizeHalf);
		glVertex3f(x, 0,  gridSizeHalf);
	}
	for(int i = 0; i < gridNz+1; ++i) {
		float alpha = i / (float)gridNz;
		float z = (1.0f - alpha) * (-gridSizeHalf) + alpha * gridSizeHalf;
		glVertex3f(-gridSizeHalf, 0, z);
		glVertex3f( gridSizeHalf, 0, z);
	}
	glEnd();
	
	// Pop modelview matrix
	glPopMatrix();
	
	// Pop projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	// Pop stacks
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "Raul Escobar", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		if(!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
			// update
			update();
			// Render scene.
			render();
			// Swap front and back buffers.
			glfwSwapBuffers(window);
		}
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
