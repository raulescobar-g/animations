#include <iostream>
#include <memory>
#include "ObjectiveLab.h"
#include "OptimizerGD.h"
#include "OptimizerGDLS.h"
#include "OptimizerNM.h"

using namespace std;
using namespace Eigen;

int main(int argc, char** argv)
{
	auto objective = make_shared<ObjectiveLab>();
	auto optimizerGD   = make_shared<OptimizerGD>();
	auto optimizerGDLS = make_shared<OptimizerGDLS>();
	auto optimizerNM   = make_shared<OptimizerNM>();
	
	optimizerGD->setAlpha(1.0);
	optimizerGDLS->setAlphaInit(5.0);
	
	VectorXd xInit(2);
	xInit << 4.0, 7.0;
	//xInit << -1.0, -0.5;

	VectorXd x(2);
	VectorXd g(2);
	MatrixXd H(2,2);
	double f = objective->evalObjective(xInit, g, H);
	cout << "f = " << f << ";" << endl;
	cout << "g = [\n" << g << "];" << endl;
	cout << "H = [\n" << H << "];" << endl;
	
	cout << "=== Gradient Descent ===" << endl;
	x = optimizerGD->optimize(objective, xInit);
	cout << "x = [\n" << x << "];" << endl;
	cout << optimizerGD->getIter() << " iterations" << endl;
	
	cout << "=== Gradient Descent with Line Search ===" << endl;
	x = optimizerGDLS->optimize(objective, xInit);
	cout << "x = [\n" << x << "];" << endl;
	cout << optimizerGDLS->getIter() << " iterations" << endl;
	
	cout << "=== Newton's Method ===" << endl;
	x = optimizerNM->optimize(objective, xInit);
	cout << "x = [\n" << x << "];" << endl;
	cout << optimizerNM->getIter() << " iterations" << endl;
	
	return 0;
}
