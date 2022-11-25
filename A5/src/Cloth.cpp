#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Cloth.h"
#include "Particle.h"
#include "Spring.h"
#include "MatrixStack.h"
#include "Program.h"
#include "GLSL.h"

using namespace std;
using namespace Eigen;

shared_ptr<Spring> createSpring(const shared_ptr<Particle> p0, const shared_ptr<Particle> p1, double E)
{
	auto s = make_shared<Spring>(p0, p1);
	s->E = E;
	Vector3d x0 = p0->x;
	Vector3d x1 = p1->x;
	Vector3d dx = x1 - x0;
	s->L = dx.norm();
	return s;
}

Cloth::Cloth(int rows, int cols,
			 const Vector3d &x00,
			 const Vector3d &x01,
			 const Vector3d &x10,
			 const Vector3d &x11,
			 double mass,
			 double stiffness)
{
	assert(rows > 1);
	assert(cols > 1);
	assert(mass > 0.0);
	assert(stiffness > 0.0);
	
	this->rows = rows;
	this->cols = cols;
	
	//
	// Create particles here
	//
	this->n = 0; // size of global vector (do not count fixed vertices)
	double r = 0.01; // Used for collisions
	int nVerts = rows*cols;

	Vector3d axisX = x01 - x00;
	Vector3d axisY = x10 - x00;

	double scale_x =  1.0f / (double) (cols-1);
	double scale_y =  1.0f / (double) (rows-1);

	std::vector<int> fixed_idx;

	fixed_idx.push_back((rows-1)*cols);
	fixed_idx.push_back(rows*cols - 1);

	int curr = 0;

	for(int i = 0; i < rows; ++i) {  // 0-2
		for(int j = 0; j < cols; ++j) {  // 0-2

			auto p = make_shared<Particle>();
			p->r = r;
			p->m = mass / (double) nVerts;
			p->x = x00 + axisX * (scale_x*(double)j) + axisY * (scale_y*(double)i);
			p->v.setZero();
			
			particles.push_back(p);

			if (i*cols + j == fixed_idx[0] || i*cols + j == fixed_idx[1]) {
				p->i = -1;
				p->fixed = true;
			} else {
				p->i = curr * 3;
				p->fixed = false;
				++curr;
				n+=3;
			}
			
		}
	}

	for (int i = 0; i < rows-1; ++i) {
		for (int j = 0; j < cols-1; ++j) {
			int k = i*cols + j;
			springs.push_back(createSpring(particles[k], particles[k+1], stiffness));
			springs.push_back(createSpring(particles[k], particles[k+cols], stiffness));
			springs.push_back(createSpring(particles[k+1], particles[k+cols], stiffness));
			springs.push_back(createSpring(particles[k], particles[k+cols+1], stiffness));
			if (j < cols-2) {
				springs.push_back(createSpring(particles[k], particles[k+2], stiffness));
			}
			if (i < rows-2) {
				springs.push_back(createSpring(particles[k], particles[k+2*cols], stiffness));
			}
		}
		int k = i*cols + (cols-1);
		springs.push_back(createSpring(particles[k], particles[k+cols], stiffness));
	}
	
	// Allocate system matrices and vectors
	M.resize(n,n);
	K.resize(n,n);
	v.resize(n);
	f.resize(n);
	
	// Build vertex buffers
	posBuf.clear();
	norBuf.clear();
	texBuf.clear();
	eleBuf.clear();
	posBuf.resize(nVerts*3);
	norBuf.resize(nVerts*3);
	updatePosNor();

	// Texture coordinates (don't change)
	for(int i = 0; i < rows; ++i) {
		for(int j = 0; j < cols; ++j) {
			texBuf.push_back(i/(rows-1.0));
			texBuf.push_back(j/(cols-1.0));
		}
	}

	// Elements (don't change)
	for(int i = 0; i < rows-1; ++i) {
		for(int j = 0; j < cols; ++j) {
			int k0 = i*cols + j;
			int k1 = k0 + cols;
			// Triangle strip
			eleBuf.push_back(k0);
			eleBuf.push_back(k1);
		}
	}
}

Cloth::~Cloth()
{
}

void Cloth::tare()
{
	for(int k = 0; k < (int)particles.size(); ++k) {
		particles[k]->tare();
	}
}

void Cloth::reset()
{
	for(int k = 0; k < (int)particles.size(); ++k) {
		particles[k]->reset();
	}
	updatePosNor();
}

void Cloth::updatePosNor()
{
	// Position
	for(int i = 0; i < rows; ++i) {
		for(int j = 0; j < cols; ++j) {
			int k = i*cols + j;
			Vector3d x = particles[k]->x;
			posBuf[3*k+0] = x(0);
			posBuf[3*k+1] = x(1);
			posBuf[3*k+2] = x(2);
		}
	}
	
	// Normal
	for(int i = 0; i < rows; ++i) {
		for(int j = 0; j < cols; ++j) {
			// Each particle has four neighbors
			//
			//      v1
			//     /|\
			// u0 /_|_\ u1
			//    \ | /
			//     \|/
			//      v0
			//
			// Use these four triangles to compute the normal
			int k = i*cols + j;
			int ku0 = k - 1;
			int ku1 = k + 1;
			int kv0 = k - cols;
			int kv1 = k + cols;
			Vector3d x = particles[k]->x;
			Vector3d xu0, xu1, xv0, xv1, dx0, dx1, c;
			Vector3d nor(0.0, 0.0, 0.0);
			int count = 0;
			// Top-right triangle
			if(j != cols-1 && i != rows-1) {
				xu1 = particles[ku1]->x;
				xv1 = particles[kv1]->x;
				dx0 = xu1 - x;
				dx1 = xv1 - x;
				c = dx0.cross(dx1);
				nor += c.normalized();
				++count;
			}
			// Top-left triangle
			if(j != 0 && i != rows-1) {
				xu1 = particles[kv1]->x;
				xv1 = particles[ku0]->x;
				dx0 = xu1 - x;
				dx1 = xv1 - x;
				c = dx0.cross(dx1);
				nor += c.normalized();
				++count;
			}
			// Bottom-left triangle
			if(j != 0 && i != 0) {
				xu1 = particles[ku0]->x;
				xv1 = particles[kv0]->x;
				dx0 = xu1 - x;
				dx1 = xv1 - x;
				c = dx0.cross(dx1);
				nor += c.normalized();
				++count;
			}
			// Bottom-right triangle
			if(j != cols-1 && i != 0) {
				xu1 = particles[kv0]->x;
				xv1 = particles[ku1]->x;
				dx0 = xu1 - x;
				dx1 = xv1 - x;
				c = dx0.cross(dx1);
				nor += c.normalized();
				++count;
			}
			nor /= count;
			nor.normalize();
			norBuf[3*k+0] = nor(0);
			norBuf[3*k+1] = nor(1);
			norBuf[3*k+2] = nor(2);
		}
	}
}

void Cloth::step(double h, const Vector3d &grav, const vector< shared_ptr<Particle> > spheres)
{
	M.setZero();
	K.setZero();
	v.setZero();
	f.setZero();

	for (auto p: particles) {
		if (p->fixed) continue;

		for (auto s: spheres) {
			auto dx = p->x - s->x;

			double d = p->r + s->r - dx.norm();

			if (d > 0.0) {
				Matrix3d I;
				I.setIdentity();
				Vector3d normal = dx / dx.norm();
				Vector3d force = c * d * normal;
				f.segment<3>(p->i) += force;

				Matrix3d Kc = c * d * I;
				K.block<3,3>(p->i, p->i) += Kc;
			}
		}
	}


	for (auto spring: springs) {
		auto p0 = spring->p0;
		auto p1 = spring->p1;

		Vector3d dx = p1->x - p0->x;
		double l = dx.norm();

		Vector3d force = spring->E * (l - spring->L) * (dx / l);

		if (!p0->fixed) f.segment<3>(p0->i) += force;
		if (!p1->fixed) f.segment<3>(p1->i) += -force;

		Matrix3d Ks;
		Ks.setZero();
		Matrix3d I;
		I.setIdentity();

		double lf = (l - spring->L)/ l;
		Ks = (spring->E / (l*l)) * ( ((1.0f - lf) * dx*dx.transpose()) + ((lf) * (dx.dot(dx)) * I) );

		if (!p0->fixed) K.block<3,3>(p0->i, p0->i) += -Ks;
		if (!p1->fixed) K.block<3,3>(p1->i, p1->i) += -Ks;

		if (!p0->fixed && !p1->fixed) {
			K.block<3,3>(p0->i, p1->i) += Ks;
			K.block<3,3>(p1->i, p0->i) += Ks;
		}
	}

	for (int i = 0; i < particles.size(); ++i) {
		if (!particles[i]->fixed) {
			int j = particles[i]->i;

			f.segment<3>(j) += grav * particles[i]->m;

			Vector3d m;
			m << particles[i]->m, particles[i]->m, particles[i]->m;

			M.block<3,3>(j,j) = m.asDiagonal();

			v.segment<3>(j) = particles[i]->v;
		}
	}

	// std::cout<<M - K*h*h<<std::endl;
	// std::cout<<M * v + h * f<<std::endl;

	VectorXd sol = (M - h*h*K).ldlt().solve(M*v + h*f);


	for (auto p: particles) {
		if (!p->fixed) {
			p->v = sol.segment<3>(p->i);
			p->x = p->x + p->v * h;
		}
	}

	
	// Update position and normal buffers
	updatePosNor();
}

void Cloth::init()
{
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_DYNAMIC_DRAW);
	
	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_DYNAMIC_DRAW);
	
	glGenBuffers(1, &texBufID);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &eleBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}

void Cloth::draw(shared_ptr<MatrixStack> MV, const shared_ptr<Program> p) const
{
	// Draw mesh
	glUniform3fv(p->getUniform("kdFront"), 1, Vector3f(1.0, 0.0, 0.0).data());
	glUniform3fv(p->getUniform("kdBack"),  1, Vector3f(1.0, 1.0, 0.0).data());
	MV->pushMatrix();
	glUniformMatrix4fv(p->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	int h_pos = p->getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	int h_nor = p->getAttribute("aNor");
	glEnableVertexAttribArray(h_nor);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	for(int i = 0; i < rows; ++i) {
		glDrawElements(GL_TRIANGLE_STRIP, 2*cols, GL_UNSIGNED_INT, (const void *)(2*cols*i*sizeof(unsigned int)));
	}
	glDisableVertexAttribArray(h_nor);
	glDisableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	MV->popMatrix();
}
