#include <iostream>
#include <fstream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Shape.h"
#include "GLSL.h"
#include "Program.h"

using namespace std;
using namespace glm;

Shape::Shape() :
	prog(NULL),
	posBufID(0),
	norBufID(0),
	texBufID(0),
	dpId1(0),
	dnId1(0),
	dpId2(0),
	dnId2(0)
{
}

Shape::~Shape()
{
}

void Shape::loadObj(const string &filename, vector<float> &pos, vector<float> &nor, vector<float> &tex, bool loadNor, bool loadTex)
{
	
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str());
	if(!warn.empty()) {
		//std::cout << warn << std::endl;
	}
	if(!err.empty()) {
		std::cerr << err << std::endl;
	}
	if(!ret) {
		return;
	}
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			// Loop over vertices in the face.
			for(size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				pos.push_back(attrib.vertices[3*idx.vertex_index+0]);
				pos.push_back(attrib.vertices[3*idx.vertex_index+1]);
				pos.push_back(attrib.vertices[3*idx.vertex_index+2]);
				if(!attrib.normals.empty() && loadNor) {
					nor.push_back(attrib.normals[3*idx.normal_index+0]);
					nor.push_back(attrib.normals[3*idx.normal_index+1]);
					nor.push_back(attrib.normals[3*idx.normal_index+2]);
				}
				if(!attrib.texcoords.empty() && loadTex) {
					tex.push_back(attrib.texcoords[2*idx.texcoord_index+0]);
					tex.push_back(attrib.texcoords[2*idx.texcoord_index+1]);
				}
			}
			index_offset += fv;
		}
	}
}

void Shape::loadMesh(const string &meshName)
{
	// Load geometry
	meshFilename = meshName;
	loadObj(meshFilename, posBuf, norBuf, texBuf);
}

void Shape::init()
{
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	
	// Send the texcoord array to the GPU
	glGenBuffers(1, &texBufID);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);

	// Send the position array to the GPU
	glGenBuffers(1, &dpId1);
	glBindBuffer(GL_ARRAY_BUFFER, dpId1);
	glBufferData(GL_ARRAY_BUFFER, dp1.size()*sizeof(float), &dp1[0], GL_STATIC_DRAW);

	// Send the position array to the GPU
	glGenBuffers(1, &dnId1);
	glBindBuffer(GL_ARRAY_BUFFER, dnId1);
	glBufferData(GL_ARRAY_BUFFER, dn1.size()*sizeof(float), &dn1[0], GL_STATIC_DRAW);

	// Send the position array to the GPU
	glGenBuffers(1, &dpId2);
	glBindBuffer(GL_ARRAY_BUFFER, dpId2);
	glBufferData(GL_ARRAY_BUFFER, dp2.size()*sizeof(float), &dp2[0], GL_STATIC_DRAW);

	// Send the position array to the GPU
	glGenBuffers(1, &dnId2);
	glBindBuffer(GL_ARRAY_BUFFER, dnId2);
	glBufferData(GL_ARRAY_BUFFER, dn2.size()*sizeof(float), &dn2[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	GLSL::checkError(GET_FILE_LINE);
}

void Shape::draw() const
{
	assert(prog);

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

	
	if (dp1.size() > 0) {
		int h_dp1 = prog->getAttribute("dp1");
		glEnableVertexAttribArray(h_dp1);
		glBindBuffer(GL_ARRAY_BUFFER, dpId1);
		glVertexAttribPointer(h_dp1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

		int h_dn1 = prog->getAttribute("dn1");
		glEnableVertexAttribArray(h_dn1);
		glBindBuffer(GL_ARRAY_BUFFER, dnId1);
		glVertexAttribPointer(h_dn1, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	

		int h_dp2 = prog->getAttribute("dp2");
		glEnableVertexAttribArray(h_dp2);
		glBindBuffer(GL_ARRAY_BUFFER, dpId2);
		glVertexAttribPointer(h_dp2, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	

		int h_dn2 = prog->getAttribute("dn2");
		glEnableVertexAttribArray(h_dn2);
		glBindBuffer(GL_ARRAY_BUFFER, dnId2);
		glVertexAttribPointer(h_dn2, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	// Draw
	int count = (int)posBuf.size()/3; // number of indices to be rendered
	glDrawArrays(GL_TRIANGLES, 0, count);
	
	glDisableVertexAttribArray(h_tex);
	glDisableVertexAttribArray(h_nor);
	glDisableVertexAttribArray(h_pos);
	if (dp1.size() > 0) {
		int h_dp1 = prog->getAttribute("dp1");
		int h_dn1 = prog->getAttribute("dn1");
		int h_dp2 = prog->getAttribute("dp2");
		int h_dn2 = prog->getAttribute("dn2");
		glDisableVertexAttribArray(h_dp1);
		glDisableVertexAttribArray(h_dn1);
		glDisableVertexAttribArray(h_dp2);
		glDisableVertexAttribArray(h_dn2);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLSL::checkError(GET_FILE_LINE);
}

void Shape::addShape(const string &meshName) {
	
	std::string blendFilename = meshName;

	std::vector<float> delta_p;
	std::vector<float> delta_n;
	std::vector<float> p;
	std::vector<float> n;
	std::vector<float> _e;

	loadObj(blendFilename, p, n, _e);

	if (dp1.size() == 0 ) {
		for (int i = 0; i < p.size(); ++i) {
			dp1.push_back(p[i] - posBuf[i]);
		}
		for (int i = 0; i < n.size(); ++i){
			dn1.push_back(n[i] - norBuf[i]);
		}
	} else {
		if (emotion) {
			for (int i = 0; i < p.size(); ++i) {
				dp1[i] += p[i] - posBuf[i];
			}
			for (int i = 0; i < n.size(); ++i){
				dn1[i] += n[i] - norBuf[i];
			}
		} else {
			for (int i = 0; i < p.size(); ++i) {
				dp2.push_back(p[i] - posBuf[i]);
			}
			for (int i = 0; i < n.size(); ++i){
				dn2.push_back(n[i] - norBuf[i]);
			}
		}
	}	
}

