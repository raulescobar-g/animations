#include "Program.h"

#include <iostream>
#include <cassert>

#include "GLSL.h"

using namespace std;

Program::Program() {};

Program::Program(std::string vert_shader, std::string frag_shader, const std::vector<std::string>& attributes, const std::vector<std::string>& uniforms){
	init(vert_shader, frag_shader, attributes, uniforms);
}

Program::Program(std::string comp_shader, const std::vector<std::string>& uniforms){
	init(comp_shader, uniforms);
}

void Program::init(string vert_shader, string frag_shader, const vector<string>& attributes, const vector<string>& uniforms){
	vShaderName = base_dir + vert_shader;
	cShaderName = "";
	fShaderName = base_dir + frag_shader;
	pid = 0;
	verbose = true;
	if (!compile_shaders()){
		std::cout<<"Something went wrong with the shaders. \n";
		exit(-1);
	}
	verbose = false;
	addAttributes(attributes);
	addUniforms(uniforms);
}

void Program::init(std::string comp_shader, const std::vector<std::string>& uniforms){
	vShaderName = "";
	cShaderName = base_dir + comp_shader;
	fShaderName = "";
	pid = 0;
	verbose = true;
	if (!compile_shaders()){
		std::cout<<"Something went wrong with the shaders. \n";
		exit(-1);
	}
	addUniforms(uniforms);
}

Program::~Program(){}


bool Program::compile_shaders(){
	GLint rc;

	if (cShaderName == "") {
		// Create shader handles
		GLuint VS = glCreateShader(GL_VERTEX_SHADER);
		GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
		GLSL::checkError(GET_FILE_LINE);

		// Read shader sources
		const char *vshader = GLSL::textFileRead(vShaderName.c_str());
		const char *fshader = GLSL::textFileRead(fShaderName.c_str());
		glShaderSource(VS, 1, &vshader, NULL);
		glShaderSource(FS, 1, &fshader, NULL);
		GLSL::checkError(GET_FILE_LINE);
		
		// Compile vertex shader
		glCompileShader(VS);
		glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
		GLSL::checkError(GET_FILE_LINE);
		if(!rc) {
			if(isVerbose()) {
				GLSL::printShaderInfoLog(VS);
				cout << "Error compiling vertex shader " << vShaderName << endl;
			}
			return false;
		}
		
		// Compile fragment shader
		glCompileShader(FS);
		glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
		GLSL::checkError(GET_FILE_LINE);
		if(!rc) {
			if(isVerbose()) {
				GLSL::printShaderInfoLog(FS);
				cout << "Error compiling fragment shader " << fShaderName << endl;
			}
			return false;
		}
		
		// Create the program and link
		pid = glCreateProgram();
		glAttachShader(pid, VS);
		glAttachShader(pid, FS);
		glLinkProgram(pid);
		glGetProgramiv(pid, GL_LINK_STATUS, &rc);
		GLSL::checkError(GET_FILE_LINE);
		if(!rc) {
			if(isVerbose()) {
				GLSL::printProgramInfoLog(pid);
				cout << "Error linking shaders " << vShaderName << " and " << fShaderName << endl;
			}
			return false;
		}
	} else {
		GLuint CS = glCreateShader(GL_COMPUTE_SHADER);
		const char *cshader = GLSL::textFileRead(cShaderName.c_str());
		glShaderSource(CS, 1, &cshader, NULL);
		glCompileShader(CS);
		glGetShaderiv(CS, GL_COMPILE_STATUS, &rc);
		if(!rc) {
			if(isVerbose()) {
				GLSL::printShaderInfoLog(CS);
				std::cout << "Error compiling compute shader " << cShaderName << std::endl;
			}
			return false;
		}
		pid = glCreateProgram();
		glAttachShader(pid, CS);

        glLinkProgram(pid);
        glGetProgramiv(pid, GL_LINK_STATUS, &rc);
        if(!rc) {
            if(isVerbose()) {
                GLSL::printProgramInfoLog(pid);
                std::cout << "Error linking compute shaders "<< std::endl;
            }
			return false;
        }
	}
	
	GLSL::checkError(GET_FILE_LINE);
	return true;
}

void Program::bind(){ glUseProgram(pid); }

void Program::unbind() { glUseProgram(0); }


void Program::addAttributes(const vector<string>& names) {
	for (auto name : names) {
		attributes[name] = glGetAttribLocation(pid, name.c_str());
	}
}


void Program::addUniforms(const vector<string>& names) {
	for (auto name : names) {
		uniforms[name] = glGetUniformLocation(pid, name.c_str());
	}
}

GLint Program::getAttribute(const string &name) const
{
	map<string,GLint>::const_iterator attribute = attributes.find(name.c_str());
	if(attribute == attributes.end()) {
		if(isVerbose()) {
			cout << name << " is not an attribute variable" << endl;
		}
		return -1;
	}
	return attribute->second;
}

GLint Program::getUniform(const string &name) const
{
	map<string,GLint>::const_iterator uniform = uniforms.find(name.c_str());
	if(uniform == uniforms.end()) {
		if(isVerbose()) {
			cout << name << " is not a uniform variable" << endl;
		}
		return -1;
	}
	return uniform->second;
}
