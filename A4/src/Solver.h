#pragma once
#ifndef SOLVER_H    
#define SOLVER_H

#include <Eigen/Dense>
#include <memory>
#include "Link.h"
#include <unsupported/Eigen/CXX11/Tensor>


using namespace Eigen;

class Solver {
    public:
        Solver(){};
        ~Solver(){};

        void setDof(int linkCount);
        void toggleVerbose(){ verbose = !verbose; }

        VectorXd optimize(const VectorXd &xInit, const Vector2d target);

    private:
        Tensor<double,3> ddp(const VectorXd &x);
        MatrixXd dp(const VectorXd &x);
        VectorXd p(const VectorXd &x);

        double f(const VectorXd &x, const Vector2d &pt);
        VectorXd grad(const VectorXd &dt, const Vector2d &pt);
        MatrixXd Hess(const VectorXd &dt, const Vector2d &pt);

        double eval(const VectorXd &x, const Vector2d &pt);
        double eval(const VectorXd &x, VectorXd &g, const Vector2d &pt);
        double eval(const VectorXd &x, VectorXd &g, MatrixXd &H, const Vector2d &pt);

        void grad_debug(const VectorXd &x, const double func, const VectorXd& grad, const Vector2d &pt);
        void hess_debug(const VectorXd &x, const VectorXd& grad, const MatrixXd& Hess, const Vector2d &pt);

        int iter = 0;
        int iterMax; // fix this
        int iterLsMax = 30;
        int dof;
        double alphaInit = 1e-4;
        double tol = 1e-3;
        double wt = 1e3;
        double wr = 1e0;
        double gamma = 0.5;
        bool verbose = false;
};

#endif