#include "ObjectiveLab.h"
#include <cmath>

using namespace Eigen;

double f(const VectorXd &x) {
	double term[5];
	term[0] = 0.5 * sin(x[0]);
	term[1] = 0.5 * sin(x[1]);
	term[2] = 0.05 * x[0] * x[0];
	term[3] = 0.05 * x[0] * x[1];
	term[4] = 0.05 * x[1] * x[1];
	return term[0] + term[1] + term[2] + term[3] + term[4];
}

VectorXd grad(const VectorXd &x){
	double term[6];
	term[0] = 0.5 * cos(x[0]);
	term[1] = 0.1 * x[0];
	term[2] = 0.05 * x[1];

	term[3] = 0.5 * cos(x[1]);
	term[4] = 0.05 * x[0];
	term[5] = 0.1 * x[1];

	VectorXd g(2);
	g << term[0] + term[1] + term[2], term[3] + term[4] + term[5];
	return g;
}

ObjectiveLab::ObjectiveLab()
{
	
}

ObjectiveLab::~ObjectiveLab()
{
	
}

double ObjectiveLab::evalObjective(const VectorXd &x)
{
	return f(x);
}

double ObjectiveLab::evalObjective(const VectorXd &x, VectorXd &g)
{
	g = grad(x);
	return f(x);
}

double ObjectiveLab::evalObjective(const VectorXd &x, VectorXd &g, MatrixXd &H)
{
	MatrixXd h(2,2);
	h << -0.5 * sin(x[0]) + 0.1, 0.05, 0.05, -0.5 * sin(x[1]) + 0.1;

	H = h;
	g = grad(x);
	return f(x);
}


