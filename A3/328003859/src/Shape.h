#pragma once
#ifndef SHAPE_H
#define SHAPE_H

#include <memory>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

class MatrixStack;
class Program;

class Shape
{
public:
	Shape();
	virtual ~Shape();
	void loadObj(const std::string &filename, std::vector<float> &pos, std::vector<float> &nor, std::vector<float> &tex, bool loadNor = true, bool loadTex = true);
	void loadMesh(const std::string &meshName);
	void setProgram(std::shared_ptr<Program> p) { prog = p; }
	virtual void init();
	virtual void draw() const;
	void setTextureFilename(const std::string &f) { textureFilename = f; }
	std::string getTextureFilename() const { return textureFilename; }
	void addShape(const std::string &meshName);
	void isEmotion(bool e) { 
		if (e) {
			for (int i = 0; i < posBuf.size(); ++i) {
				dp2.push_back(0.0f);
				dn2.push_back(0.0f);
			}
		}
		emotion = e; 
	}

	
protected:
	std::string meshFilename;
	std::string textureFilename;
	std::shared_ptr<Program> prog;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	std::vector<float> dp1;
	std::vector<float> dn1;
	std::vector<float> dp2;
	std::vector<float> dn2;

	std::vector<std::vector<float>> delta_posBuf;
	std::vector<std::vector<float>> delta_norBuf;
	
	GLuint posBufID, norBufID, texBufID, dpId1, dpId2, dnId1, dnId2;
	bool emotion;
};

#endif
