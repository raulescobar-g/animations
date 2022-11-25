#include "Mesh.h"
#include <iostream>

#include "GLSL.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Mesh::Mesh() :
	posBufID(0),
	norBufID(0),
	texBufID(0),
	indBufID(0)
{
}

Mesh::Mesh(std::string meshFile) :
	posBufID(0),
	norBufID(0),
	texBufID(0),
	indBufID(0)
{
    loadMesh(meshFile, *this);
    fitToUnitBox(*this);
    init(*this);
}

void loadMesh(const std::string &meshName, Mesh& mesh){
    std::string file = BASE_DIR + meshName;
	// Load geometry
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, file.c_str());
	if(!rc) {
		std::cerr << errStr << std::endl;
	} else {
		// Some OBJ files have different indices for vertex positions, normals,
		// and texture coordinates. For example, a cube corner vertex may have
		// three different normals. Here, we are going to duplicate all such
		// vertices.
		// Loop over shapes
		for(size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			size_t index_offset = 0;
			for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					mesh.posBuf.push_back(glm::vec3(attrib.vertices[3*idx.vertex_index+0], attrib.vertices[3*idx.vertex_index+1], attrib.vertices[3*idx.vertex_index+2]));
					if(!attrib.normals.empty()) {
						mesh.norBuf.push_back(glm::vec3(attrib.normals[3*idx.normal_index+0], attrib.normals[3*idx.normal_index+1], attrib.normals[3*idx.normal_index+2]));
					}
					if(!attrib.texcoords.empty()) {
						mesh.texBuf.push_back(glm::vec2(attrib.texcoords[2*idx.texcoord_index+0], attrib.texcoords[2*idx.texcoord_index+1]));
					}
				}
				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}
}

void fitToUnitBox(Mesh& mesh){
	// Scale the vertex positions so that they fit within [-1, +1] in all three dimensions.
	glm::vec3 vmin = mesh.posBuf[0];
	glm::vec3 vmax = mesh.posBuf[0];
	for(int i = 0; i < (int)mesh.posBuf.size(); ++i) {
		glm::vec3 v = mesh.posBuf[i];
		vmin = glm::min(vmin, v);
		vmax = glm::max(vmax, v);
	}

	glm::vec3 center = 0.5f*(vmin + vmax);
	glm::vec3 diff = vmax - vmin;
	float diffmax = diff.x;
	diffmax = glm::max(diffmax, diff.y);
	diffmax = glm::max(diffmax, diff.z);
	float scale = 1.0f / diffmax;
	for(int i = 0; i < (int)mesh.posBuf.size(); ++i) {
		mesh.posBuf[i] = (mesh.posBuf[i] - center) * scale;
	}
}

void init(Mesh& mesh)
{
	// Send the position array to the GPU
	glGenBuffers(1, &mesh.posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.posBufID);
	glBufferData(GL_ARRAY_BUFFER, mesh.posBuf.size()*3*sizeof(float), &mesh.posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	if(!mesh.norBuf.empty()) {
		glGenBuffers(1, &mesh.norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.norBufID);
		glBufferData(GL_ARRAY_BUFFER, mesh.norBuf.size()*3*sizeof(float), &mesh.norBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the texture array to the GPU
	if(!mesh.texBuf.empty()) {
		glGenBuffers(1, &mesh.texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.texBufID);
		glBufferData(GL_ARRAY_BUFFER, mesh.texBuf.size()*2*sizeof(float), &mesh.texBuf[0], GL_STATIC_DRAW);
	}	
	
	if (!mesh.indBuf.empty()) {
		glGenBuffers(1, &mesh.indBufID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indBufID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indBuf.size()*sizeof(unsigned int), &mesh.indBuf[0], GL_STATIC_DRAW);
	}

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	GLSL::checkError(GET_FILE_LINE);
}

void draw(const Program& prog, const Mesh& mesh) {
	// Bind position buffer
	int h_pos = prog.getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	int h_nor = prog.getAttribute("aNor");
	if(h_nor != -1 && mesh.norBufID != 0) {
		glEnableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}
	
	// Bind texcoords buffer
	int h_tex = prog.getAttribute("aTex");
	if(h_tex != -1 && mesh.texBufID != 0) {
		glEnableVertexAttribArray(h_tex);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.texBufID);
		glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}
	
	// Draw
	if (mesh.indBuf.empty()){
        glDrawArrays(GL_TRIANGLES, 0, mesh.posBuf.size());
		if (h_tex != -1) glDisableVertexAttribArray(h_tex);
        if (h_nor != -1) glDisableVertexAttribArray(h_nor);
        glDisableVertexAttribArray(h_pos);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
     } else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indBufID);
        glDrawElements(GL_TRIANGLES, (int)mesh.indBuf.size(), GL_UNSIGNED_INT, (void *)0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
     } 
		
	GLSL::checkError(GET_FILE_LINE);
}

std::vector<glm::vec3> Mesh::operator *(glm::vec3 scale)  {
    std::vector<glm::vec3> result = posBuf;
    for (auto& x: result) {
        x *= scale;
    }
    return result;
}
