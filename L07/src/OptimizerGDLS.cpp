#include "OptimizerGDLS.h"
#include "Objective.h"
#include <iostream>

using namespace std;
using namespace Eigen;

OptimizerGDLS::OptimizerGDLS() :
	alphaInit(1.0),
	gamma(0.5),
	tol(1e-6),
	iterMax(100),
	iter(0)
{
	
}

OptimizerGDLS::~OptimizerGDLS()
{
	
}

VectorXd OptimizerGDLS::optimize(const shared_ptr<Objective> objective, const VectorXd &xInit)
{
	int n = xInit.rows();
	VectorXd x = xInit;
	VectorXd g(n);
	VectorXd dx(n);
	double f, f_dx, alpha;

	int iterLs;
	int iterLsMax = 100;
	

	for (iter = 1; iter < iterMax; ++iter) {
		f = objective->evalObjective(x, g);
		alpha = alphaInit;
		for (iterLs = 1; iterLs < iterLsMax; ++iterLs) {
			dx = -alpha * g;
			f_dx = objective->evalObjective(x + dx);
			if (f_dx < f) 
				break;
			alpha *= gamma;
		}
		
		x += dx;

		if (dx.norm() < tol) 
			break;
	}

	return x;
}
