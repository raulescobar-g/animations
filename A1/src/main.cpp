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

glm::mat4 B, G, R;
float c, tmax, smax, ds;

vector<pair<float,float> > table;


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

struct Keyframe {
	Keyframe(glm::vec3 pos, glm::quat quat): pos(pos), quat(quat) {};
	glm::vec3 pos;
	glm::quat quat;
};

vector<Object> objects;
vector<Keyframe> keyframes;

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

void fillTable() {

	int total = keyframes.size();
	float u1;
	float u = 0.0f;
	float s = 0.0f;
	table.push_back(make_pair(u, s));

	for (int i = 0; i < total; ++i){
		u = 0.0f;
		for (int j = 0; j < (int) 1.0f/ds; ++j) {
			u1 = u + ds;

			//cout<<u1<<endl;
			//cout<< mod(i-1, total) << ", "<< mod(i, total)<< ", "<< mod(i+1, total) <<", "<< mod(i+2, total) <<endl;
			G[0] = glm::vec4(keyframes[mod(i-1, total)].pos, 0.0f);
			G[1] = glm::vec4(keyframes[mod(i, total)].pos, 0.0f);
			G[2] = glm::vec4(keyframes[mod(i+1, total)].pos, 0.0f);
			G[3] = glm::vec4(keyframes[mod(i+2, total)].pos, 0.0f);

			glm::vec3 pA = c * G * B * glm::vec4(1.0f, u, u*u, u*u*u);
			glm::vec3 pB = c * G * B * glm::vec4(1.0f, u1, u1*u1, u1*u1*u1);

			s += glm::length(pB - pA);

			table.push_back(make_pair(i + u1, s));
			u += ds;
		}
	}
	// for (auto e: table) {
	// 	cout<<e.first<<" , "<<e.second<<endl;
	// }
	//exit(0);
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
	
	float pi = glm::pi<float>();
	keyframes.push_back(Keyframe(glm::vec3(-1.0f, 0.0f, 1.0f), glm::angleAxis(pi/3.0f, glm::vec3(1.0f, 0.0f, 0.0f))));
	keyframes.push_back(Keyframe(glm::vec3(0.0f, 3.0f, -1.0f), glm::angleAxis(pi/2.0f, glm::vec3(1.0f, 0.0f, 0.0f))));
	keyframes.push_back(Keyframe(glm::vec3(7.0f, 0.0f, 2.0f), glm::angleAxis(2.0f * pi / 3.0f, glm::vec3(1.0f, 0.0f, 0.0f))));
	keyframes.push_back(Keyframe(glm::vec3(0.0f, -2.0f, -2.0f), glm::angleAxis(pi, glm::vec3(1.0f, 0.0f, 0.0f))));
	keyframes.push_back(Keyframe(glm::vec3(-20.0f, -20.0f, 0.0f), glm::angleAxis(3.0f * pi / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f))));

	tmax = 25.0f;
	ds = 0.1f;
	
	fillTable();
	smax = table[table.size()-1].second;
	// Initialize time.
	glfwSetTime(0.0);
	t = 0.0;

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

glm::vec4 toVec4(glm::quat q) {
	return glm::vec4(q.x, q.y, q.z, q.w);
}

std::vector<glm::quat> getQuats(float u) {
	int size = keyframes.size();
	glm::quat q0 = keyframes[mod((int) u - 1, size)].quat;
	glm::quat q1 = keyframes[mod((int) u , size)].quat;
	glm::quat q2 = keyframes[mod((int) u + 1, size)].quat;
	glm::quat q3 = keyframes[mod((int) u + 2, size)].quat;
	if (glm::dot(q0, q1) < 0.0f){
		q1.x = -q1.x;
		q1.y = -q1.y;
		q1.z = -q1.z;
		q1.w = -q1.w;
	}
	if (glm::dot(q1, q2) < 0.0f) {
		q2.x = -q2.x;
		q2.y = -q2.y;
		q2.z = -q2.z;
		q2.w = -q2.w;
	}
	if (glm::dot(q2, q3) < 0.0f) {
		q3.x = -q3.x;
		q3.y = -q3.y;
		q3.z = -q3.z;
		q3.w = -q3.w;
	}
	std::vector res = { q0, q1, q2, q3 };
	return res;
}

float stou(float s) {
	float u0, u1;
	float alpha;
	
	for (int i = 0; i < table.size(); ++i) {
		
		if (s > table[i].second && s < table[i+1].second) {
			
			u0 = table[i].first;
			u1 = table[i+1].first;
			alpha = (s - table[i].second) / (table[i+1].second - table[i].second);
			break;
		}
	}
	//cout<<alpha<<", "<<s0<<", "<<s1<<endl;
	return (1-alpha) * u0 + alpha * u1;
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

	int size = (int)keyframes.size();
	float tNorm = std::fmod(t, tmax) / tmax;
	float sNorm = tNorm;
	float s = sNorm * smax;

	float u;
	if (keyPresses[(unsigned) 's']%2 == 0){
		u = stou(s);
	} else if (keyPresses[(unsigned) 's']%2 == 1) {
		u = s;
	} else {
		u = stou(-2 * tNorm * tNorm * tNorm + 3 * tNorm * tNorm);
	}

	float k = u - std::floor(u);

	std::vector<glm::quat> qS = getQuats(u);
	
	G[0] = glm::vec4(keyframes[mod((int) u - 1, size)].pos, 0.0f);
	G[1] = glm::vec4(keyframes[mod((int) u , size)].pos, 0.0f);
	G[2] = glm::vec4(keyframes[mod((int) u + 1, size)].pos, 0.0f);
	G[3] = glm::vec4(keyframes[mod((int) u + 2, size)].pos, 0.0f);
	
	glm::vec4 uVec(1.0f, k, k*k, k*k*k);
	// Compute position at u
	glm::vec4 offset = c * G*(B*uVec);	

	G[0] = glm::vec4(toVec4(qS[0]));
	G[1] = glm::vec4(toVec4(qS[1]));
	G[2] = glm::vec4(toVec4(qS[2]));
	G[3] = glm::vec4(toVec4(qS[3]));

	glm::vec4 qVec = c * G *(B*uVec);	

	glm::quat q(qVec[3], qVec[0], qVec[1], qVec[2]); 
	R = glm::mat4_cast(glm::normalize(q)); 
	R[3] = glm::vec4(glm::vec3(offset), 1.0f);

	for (const Object& object : objects) {
		MV->pushMatrix();
			
			//MV->translate(object.position + glm::vec3(offset));
			MV->scale(object.scale, object.scale, object.scale);
			MV->multMatrix(R);
			prog->bind();
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniform3f(prog->getUniform("kd"), 1.0f, 0.0f, 0.0f);
			object.shape->draw(prog);
			prog->unbind();

		MV->popMatrix();
	}

	if (keyPresses[(unsigned) 'k'] % 2) {
		for (const Keyframe& kf: keyframes) {
			R = glm::mat4_cast(glm::normalize(kf.quat)); 
			R[3] = glm::vec4(kf.pos, 1.0f);

			for (const Object& object : objects) {
				MV->pushMatrix();
					
					//MV->translate(object.position + glm::vec3(offset));
					MV->scale(object.scale, object.scale, object.scale);
					MV->multMatrix(R);
					prog->bind();
					glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
					glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
					glUniform3f(prog->getUniform("kd"), 1.0f, 0.0f, 0.0f);
					object.shape->draw(prog);
					prog->unbind();

				MV->popMatrix();
			}
		}
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

	
		
	glColor3f(1.0f, 0.0f, 0.0f);
	glPointSize(10.0f);
	glBegin(GL_POINTS);
	float _ds = 0.2f;
	for(float _s = 0.0f; _s < smax; _s += _ds) {
		// Convert from s to (concatenated) u
		
		float uu = keyPresses[(unsigned) 's']%2==0 ? stou(_s) : _s;
		if (keyPresses[(unsigned) 's']%2 == 0){
			uu = stou(_s);
		} else if (keyPresses[(unsigned) 's']%2 == 1) {
			uu = _s;
		} else {
			uu = stou(-2 * _s * _s * _s + 3 * _s * _s);
		}
		// Convert from concatenated u to the usual u between 0 and 1.
		float kfloat;
		float _u = std::modf(uu, &kfloat);
		// k is the index of the starting control point
		int k = (int)std::floor(kfloat);
		// Compute spline point at u
		glm::mat4 Gk;
		for(int i = 0; i < 4; ++i) {
			Gk[i] = glm::vec4(keyframes[mod(k+i, keyframes.size())].pos, 0.0f);
		}
		glm::vec4 uVec(1.0f, _u, _u*_u, _u*_u*_u);
		glm::vec3 P(c * Gk * (B * uVec));
		glVertex3fv(&P[0]);
	}
	glEnd();
	

	glBegin(GL_LINE_STRIP);
	
	if(keyPresses[(unsigned)'k'] % 2) {
		glColor3f(0, 1, 0);
		for (int j = -1; j < ((int)keyframes.size()) - 1; ++j) {
			
			int size = (int)keyframes.size();
			G[0] = glm::vec4(keyframes[mod(j, size)].pos, 0.0f);
			G[1] = glm::vec4(keyframes[mod(j+1, size)].pos, 0.0f);
			G[2] = glm::vec4(keyframes[mod(j+2, size)].pos, 0.0f);
			G[3] = glm::vec4(keyframes[mod(j+3, size)].pos, 0.0f);
			
			
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
