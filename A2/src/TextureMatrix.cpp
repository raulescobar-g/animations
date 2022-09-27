#include <iostream>
#include <fstream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "TextureMatrix.h"

using namespace std;
using namespace glm;

TextureMatrix::TextureMatrix()
{
	type = Type::NONE;
	T = mat3(1.0f);
}

TextureMatrix::~TextureMatrix()
{
	
}

void TextureMatrix::setType(const string &str)
{
	if(str.find("Body") != string::npos) {
		type = Type::BODY;
	} else if(str.find("Mouth") != string::npos) {
		type = Type::MOUTH;
	} else if(str.find("Eyes") != string::npos) {
		type = Type::EYES;
	} else if(str.find("Brows") != string::npos) {
		type = Type::BROWS;
	} else {
		type = Type::NONE;
	}
}

void TextureMatrix::update(unsigned int key)
{
	// Update T here
	if(type == Type::BODY) {
		// Do nothing
	} else if(type == Type::MOUTH) {
		// TODO
	} else if(type == Type::EYES) {
		// TODO
	} else if(type == Type::BROWS) {
		// TODO
	}
}
