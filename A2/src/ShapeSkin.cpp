#include <iostream>
#include <fstream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "ShapeSkin.h"
#include "GLSL.h"
#include "Program.h"
#include "TextureMatrix.h"
//#include "utils.h"

using namespace std;
using namespace glm;

ShapeSkin::ShapeSkin() :
	prog(NULL),
	elemBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0)
{
	T = make_shared<TextureMatrix>();
}

ShapeSkin::~ShapeSkin()
{
}

void ShapeSkin::setTextureMatrixType(const std::string &meshName)
{
	T->setType(meshName);
}

void ShapeSkin::loadMesh(const string &meshName)
{
	// Load geometry
	// This works only if the OBJ file has the same indices for v/n/t.
	// In other words, the 'f' lines must look like:
	// f 70/70/70 41/41/41 67/67/67
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string warnStr, errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &warnStr, &errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
		posBuf = attrib.vertices;
		norBuf = attrib.normals;
		texBuf = attrib.texcoords;
		assert(posBuf.size() == norBuf.size());
		// Loop over shapes
		for(size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			const tinyobj::mesh_t &mesh = shapes[s].mesh;
			size_t index_offset = 0;
			for(size_t f = 0; f < mesh.num_face_vertices.size(); f++) {
				size_t fv = mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = mesh.indices[index_offset + v];
					elemBuf.push_back(idx.vertex_index);
				}
				index_offset += fv;
				// per-face material (IGNORE)
				//shapes[s].mesh.material_ids[f];
			}
		}
	}
}

void ShapeSkin::loadAttachment(const std::string &filename)
{
	
	std::ifstream in;
	in.open(filename);
	if(!in.good()) {
		std::cout << "Cannot read: " << filename << std::endl;
	}
	std::cout << "Loading: " << filename << std::endl;
	
	std::string line;

	for (int i = 0; i < 5; ++i) std::getline(in, line); //comments

    

	std::stringstream ss(line);
	std::string vertices_s, bones_s, max_influences_s;
	ss >> vertices_s;
	ss >> bones_s;
	ss >> max_influences_s;

	max_influences = std::stoi(max_influences_s);
	int vertices = std::stoi(vertices_s);

	idxBuf.resize(vertices);
	weiBuf.resize(vertices);
	
	std::string influence_buf, idx_buf, weight_buf;
	for (int i = 0; i < vertices; ++i) {
		idxBuf.push_back(std::vector<int>());
		weiBuf.push_back(std::vector<float>());

        std::getline(in, line);
        std::stringstream ss(line);

		ss >> influence_buf;
		int influence_count = std::stoi(influence_buf);

		int idx;
		float weight;

        for (int j = 0; j < influence_count; ++j) {            
            
            ss >> idx_buf;
            ss >> weight_buf;

			idx = std::stoi(idx_buf);
			weight = std::stof(weight_buf);

            idxBuf[i].push_back(idx);
			weiBuf[i].push_back(weight);
        }
		for (int k = 0; k < max_influences - influence_count; ++k){
			idxBuf[i].push_back(0);
			weiBuf[i].push_back(0.0f);
		}
	}
	in.close();
}

void ShapeSkin::init()
{
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_DYNAMIC_DRAW);
	
	// Send the normal array to the GPU
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_DYNAMIC_DRAW);
	
	// Send the texcoord array to the GPU
	glGenBuffers(1, &texBufID);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	
	// Send the element array to the GPU
	glGenBuffers(1, &elemBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elemBuf.size()*sizeof(unsigned int), &elemBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	GLSL::checkError(GET_FILE_LINE);
}

void ShapeSkin::update(int k, AnimationData& data)
{

	std::vector<float> newPosBuf = calculate_position(k, data);
	std::vector<float> newNormBuf = calculate_normal(k, data);

	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &newPosBuf[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &newNormBuf[0], GL_DYNAMIC_DRAW);
	
	GLSL::checkError(GET_FILE_LINE);
}

void ShapeSkin::draw(int k) const
{
	assert(prog);

	// Send texture matrix
	glUniformMatrix3fv(prog->getUniform("T"), 1, GL_FALSE, glm::value_ptr(T->getMatrix()));
	
	int h_pos = prog->getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	int h_nor = prog->getAttribute("aNor");
	glEnableVertexAttribArray(h_nor);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

	int h_tex = prog->getAttribute("aTex");
	glEnableVertexAttribArray(h_tex);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Draw
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBufID);
	glDrawElements(GL_TRIANGLES, (int)elemBuf.size(), GL_UNSIGNED_INT, (const void *)0);
	
	glDisableVertexAttribArray(h_nor);
	glDisableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GLSL::checkError(GET_FILE_LINE);
}



std::vector<float> ShapeSkin::calculate_position(int frame, AnimationData& data) {
	std::vector<float> newPosBuf;

	for (int i = 0; i < posBuf.size()/3; ++i) {
		glm::vec4 x0(posBuf[i*3], posBuf[i*3+1], posBuf[i*3+2], 1.0f);
		glm::vec4 p(0.0f, 0.0f, 0.0f, 0.0f);
		for (int j = 0; j < max_influences; ++j) {
			if (weiBuf[i][j] == 0.0f && idxBuf[i][j] == 0) break;
			p += weiBuf[i][j] * data.transforms[frame*data.bones + idxBuf[i][j]]  * glm::inverse(data.transforms[idxBuf[i][j]])  * x0;
			
		}
		newPosBuf.push_back(p.x);
		newPosBuf.push_back(p.y);
		newPosBuf.push_back(p.z);
	}
	return newPosBuf;
	
}


std::vector<float> ShapeSkin::calculate_normal(int frame, AnimationData& data) {
	std::vector<float> newNormBuf;

	for (int i = 0; i < norBuf.size()/3; ++i) {
		glm::vec4 x0(norBuf[i*3], norBuf[i*3+1], norBuf[i*3+2], 0.0f);
		glm::vec4 p(0.0f, 0.0f, 0.0f, 0.0f);
		for (int j = 0; j < max_influences; ++j) {
			if (weiBuf[i][j] == 0.0f && idxBuf[i][j] == 0) break;
			p += weiBuf[i][j] * data.transforms[frame*data.bones + idxBuf[i][j]]  * glm::inverse(data.transforms[idxBuf[i][j]])  * x0;
			
		}
		//std::cout<<i<<std::endl;
		newNormBuf.push_back(p.x);
		newNormBuf.push_back(p.y);
		newNormBuf.push_back(p.z);
	}
	return newNormBuf;
	
}
