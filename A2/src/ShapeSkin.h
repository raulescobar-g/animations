#pragma once
#ifndef SHAPESKIN_H
#define SHAPESKIN_H

#include <memory>

#define GLEW_STATIC
#include <GL/glew.h>

#include "AnimationData.h"

class MatrixStack;
class Program;
class TextureMatrix;

class ShapeSkin
{
public:
	ShapeSkin();
	virtual ~ShapeSkin();
	void setTextureMatrixType(const std::string &meshName);
	void loadMesh(const std::string &meshName);
	void loadAttachment(const std::string &filename);
	void setProgram(std::shared_ptr<Program> p) { prog = p; }
	void init();
	void update(int k, AnimationData& data);
	void draw(int k) const;
	void setTextureFilename(const std::string &f) { textureFilename = f; }
	std::string getTextureFilename() const { return textureFilename; }
	std::shared_ptr<TextureMatrix> getTextureMatrix() { return T; }
	
private:
	std::vector<float> calculate_position(int frame, AnimationData& data); 
	std::vector<float> calculate_normal(int frame, AnimationData& data); 
	std::shared_ptr<Program> prog;
	std::vector<unsigned int> elemBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	GLuint elemBufID;
	GLuint posBufID;
	GLuint norBufID;
	GLuint texBufID;
	std::string textureFilename;
	std::shared_ptr<TextureMatrix> T;

	std::vector<std::vector<int> > idxBuf;
	std::vector<std::vector<float> > weiBuf;
	int max_influences;
};

#endif
