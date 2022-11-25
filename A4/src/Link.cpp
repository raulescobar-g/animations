#include <assert.h>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>

#include "Link.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "Program.h"

using namespace std;
using namespace Eigen;

Link::Link() :
	parent(NULL),
	child(NULL),
	depth(0),
	position(0.0, 0.0, 0.0),
	angle(0.0)
{
	meshMat << 	1.0, 0.0, 0.0, 0.5,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0, 
				0.0, 0.0, 0.0, 1.0;

}

Link::~Link()
{
	
}

void Link::addChild(std::shared_ptr<Link> new_child)
{
	new_child->parent = shared_from_this();
	new_child->depth = depth + 1;
	child = new_child;
	child->setPosition(1.0, 0.0);
}

void Link::draw(const std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Shape> shape) const
{
	assert(prog);
	assert(MV);
	assert(shape);
	
	MV->pushMatrix();
		MV->translate(position); 
		MV->rotate(angle, 0.0f, 0.0f, 1.0f);

			MV->pushMatrix();
				MV->multMatrix(meshMat); 
				drawMesh(prog, MV, shape);
			MV->popMatrix();

		if (child) child->draw(prog, MV, shape);
		
    MV->popMatrix();
}

void Link::drawMesh(const std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Shape> shape) const {
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glUniform3f(prog->getUniform("ka"), 0.1f, 0.1f, 0.1f);
	glUniform3f(prog->getUniform("ks"), 0.1f, 0.1f, 0.1f);
	glUniform1f(prog->getUniform("s"), 200.0f);
	shape->draw();
}
