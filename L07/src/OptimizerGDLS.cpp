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
	iter = 0;
	return x;
}
