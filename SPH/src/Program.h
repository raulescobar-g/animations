#pragma  once
#ifndef PROGRAM_H
#define PROGRAM_H

#include <map>
#include <string>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

#define BASE_DIR "/Users/raulescobar/Documents/CSCE_450/SPH/resources/"

class Program {
	public:
		Program();
		~Program();

		Program(std::string vert_shader, std::string frag_shader, const std::vector<std::string>& attributes, const std::vector<std::string>& uniforms);
		Program(std::string comp_shader, const std::vector<std::string>& uniforms);
		
		void setVerbose(bool v) { verbose = v; }
		bool isVerbose() const { return verbose; }
		
		void init(std::string vert_shader, std::string frag_shader, const std::vector<std::string>& attributes, const std::vector<std::string>& uniforms);
		void init(std::string comp_shader, const std::vector<std::string>& uniforms);

		void bind();
		void unbind();

		void addAttributes(const std::vector<std::string> &names);
		void addUniforms(const std::vector<std::string> &names);
		GLint getAttribute(const std::string &name) const;
		GLint getUniform(const std::string &name) const;
		
	protected:
		std::string vShaderName;
		std::string fShaderName;
		std::string cShaderName;
		
	private:
		std::string base_dir = BASE_DIR;
		bool compile_shaders();

		GLuint pid;
		std::map<std::string,GLint> attributes;
		std::map<std::string,GLint> uniforms;
		bool verbose;
};

#endif
