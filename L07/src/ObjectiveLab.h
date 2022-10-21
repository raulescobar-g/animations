#pragma once
#ifndef OBJECTIVE_LAB_H
#define OBJECTIVE_LAB_H

#include "Objective.h"

class ObjectiveLab : public Objective
{
public:
	ObjectiveLab();
	virtual ~ObjectiveLab();
	virtual double evalObjective(const Eigen::VectorXd &x);
	virtual double evalObjective(const Eigen::VectorXd &x, Eigen::VectorXd &g);
	virtual double evalObjective(const Eigen::VectorXd &x, Eigen::VectorXd &g, Eigen::MatrixXd &H);
};

#endif
