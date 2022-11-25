#include "Particles.h"
// a = -(u * del) u + dp + nu * dd + rho * g

Particles::~Particles(){
    delete[] state;
    delete[] dstate;
}


void Particles::init() {
    float scale = 0.3f;
    state = new glm::vec4[size*2];
    dstate = new glm::vec4[size*2];

    int l = (int) std::sqrt(size);
    float wall = 5.0f;
    float x_inc = wall / (float)l;
    float y_inc = wall / (float)l;

    for (int i = 0; i < size*2; ++i) {
        state[i] = glm::vec4(0.1f);
        dstate[i] = glm::vec4(0.0f);
    }

    for (int i = 0; i < l; ++i) {
        for (int j = 0; j < l; ++j) {
            int idx = i*l + j;
            float x = (float) i * x_inc;
            float y = (float) j * y_inc;
            state[idx] = glm::vec4(x, 0.0f, y, scale);
            state[idx+size] = glm::vec4(0.0f);
        }
    }

    glGenBuffers( 1, &posSSbo);
    glBindBuffer( GL_ARRAY_BUFFER, posSSbo );
    glBufferData( GL_ARRAY_BUFFER, size * sizeof(glm::vec4), &state[0], GL_DYNAMIC_DRAW );
}

void Particles::update(float dt){

    for (int i = 0; i < size; ++i) {
        dstate[i] = glm::vec4(glm::vec3(state[i + size]), 0.0f);
    }
    for (int i = size; i < size*2; ++i) {
        dstate[i] = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    }

    for (int i = 0; i < size * 2; ++i) {
        state[i] += dstate[i] * dt;
    }


    glBindBuffer( GL_ARRAY_BUFFER, posSSbo );
    glBufferData( GL_ARRAY_BUFFER, size * sizeof(glm::vec4), &state[0], GL_DYNAMIC_DRAW );
}

void Particles::render(const Program& prog, const Mesh& sphere){
    // Bind position buffer
	int h_pos = prog.getAttribute("aPos");
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, sphere.posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	int h_nor = prog.getAttribute("aNor");
	if(h_nor != -1 && sphere.norBufID != 0) {
		glEnableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, sphere.norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

    int positionsID = prog.getAttribute("position");
    glEnableVertexAttribArray(positionsID);
    glBindBuffer(GL_ARRAY_BUFFER, posSSbo);
    glVertexAttribPointer(positionsID, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glVertexAttribDivisor(h_pos, 0); 
    glVertexAttribDivisor(h_nor, 0); 
    glVertexAttribDivisor(positionsID, 1);
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, sphere.posBuf.size() / 3, size);

    glDisableVertexAttribArray(h_pos);
    glDisableVertexAttribArray(h_nor);
    glDisableVertexAttribArray(positionsID);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// check if these are actually derivatives
float Particles::W_poly6(float dist, float rad) {
    if(dist >= 0.0f && dist <= rad){
        double c = 315.0f/(64.0f * 3.14159f * std::pow(rad,9));
        return c * std::pow(std::pow(rad,2)-std::pow(dist,2),3);
    }
    return 0.0f;
}

float Particles::W_spiky(float dist, float rad) {
    if(dist >= 0.0f && dist <= rad){
        double c = 15.0f/(3.14159f * std::pow(rad,6));
        return c * std::pow(rad-dist,3);
    }
    return 0.0f;
}

float Particles::W_visc(float dist, float rad) {
    if(dist >= 0.0f && dist <= rad){
        double c = 45.0f/(3.14159f * std::pow(rad,6));
        return c * (rad - dist);
    }
    return 0.0f;
}