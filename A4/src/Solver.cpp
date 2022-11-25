#include "Solver.h"
#include <Eigen/Geometry>
#include <iostream> 

#define pi 3.14159

MatrixXd getR(int i, int j, int k, const VectorXd& theta) {
    MatrixXd R(3,3);

    if (k == i || k == j) {
        if (i == j) {
            R <<-cos(theta[k]),   sin(theta[k]),  0.0,
                -sin(theta[k]),  -cos(theta[k]),  0.0,
                 0.0,             0.0,            0.0;
        } else {
            R <<-sin(theta[k]),  -cos(theta[k]),  0.0,
                 cos(theta[k]),  -sin(theta[k]),  0.0,
                 0.0,             0.0,            0.0;
        }
    } else {
        R <<cos(theta[k]),  -sin(theta[k]),  0.0,
            sin(theta[k]),   cos(theta[k]),  0.0,
            0.0,             0.0,            1.0;
    }

    return R;
}

void Solver::setDof(int linkCount) { 
    dof = linkCount; 
    iterMax = dof * 10;
}

VectorXd Solver::p(const VectorXd &theta) {

    int n = theta.rows();
    assert(n == dof);

    MatrixXd M(3,3);
    MatrixXd T(3,3);
    MatrixXd R(3,3);

    M <<1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0;
    
    T <<1.0, 0.0, 1.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0;

    R <<cos(theta[0]),  -sin(theta[0]), 0.0,
        sin(theta[0]),  cos(theta[0]),  0.0,
        0.0,            0.0,            1.0;

    M *= R;

    for (int i = 1; i < n; ++i) {
        R <<cos(theta[i]),  -sin(theta[i]), 0.0,
            sin(theta[i]),  cos(theta[i]),  0.0,
            0.0,            0.0,            1.0;
        M = M * T * R;
    }
    
    Vector2d temp;
    Vector3d sol = M * Vector3d(1.0, 0.0, 1.0);
    temp <<sol(0), sol(1);
    return temp;
}

MatrixXd Solver::dp(const VectorXd &theta) {
    int n = theta.rows();
    assert(n == dof);
    
    MatrixXd P(2, n);

    MatrixXd M(3,3);
    MatrixXd T(3,3);
    MatrixXd R(3,3);

    T <<1.0, 0.0, 1.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0;

    for (int j = 0; j < n; ++j) {
        M <<1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0;
        
        if (j != 0) {
            R <<cos(theta[0]),  -sin(theta[0]), 0.0,
                sin(theta[0]),  cos(theta[0]),  0.0,
                0.0,            0.0,            1.0;
        } else {
            R <<-sin(theta[0]),  -cos(theta[0]), 0.0,
                cos(theta[0]),  -sin(theta[0]),  0.0,
                0.0,            0.0,            0.0;
        }

        M *= R;

        for (int i = 1; i < n; ++i) {
            if (j != i) {
                R <<cos(theta[i]),  -sin(theta[i]), 0.0,
                    sin(theta[i]),  cos(theta[i]),  0.0,
                    0.0,            0.0,            1.0;
            } else {
                R <<-sin(theta[i]),  -cos(theta[i]), 0.0,
                    cos(theta[i]),  -sin(theta[i]),  0.0,
                    0.0,            0.0,            0.0;
            }
            M = M * T * R;
        }
        VectorXd temp = M * Vector3d(1.0, 0.0, 1.0);
        P.col(j) << temp[0], temp[1];
    }
    return P;
}

Tensor<double,3> Solver::ddp(const VectorXd &theta) {
    int n = theta.rows();
    assert(n == dof);
    
    Tensor<double,3> ddP(n, n, 2);
    
    MatrixXd M(3,3);
    MatrixXd T(3,3);
    MatrixXd R(3,3);

    T <<1.0, 0.0, 1.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0;

    for(int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            R = getR(i,j,0,theta);
            M = R;
            for (int k = 1; k < n; ++k) {
                R = getR(i,j,k,theta);
                M = M * T * R;
            }
            VectorXd temp = M * Vector3d(1.0, 0.0, 1.0);
            ddP(i,j,0) = temp(0);
            ddP(i,j,1) =  temp(1);
        }
    }
    return ddP;
}

double Solver::f(const VectorXd &dt, const Vector2d &pt) {
    int n = dt.rows();
    assert(n == dof);

    VectorXd delta_p(n);
    delta_p = p(dt) - pt;

    return 0.5 * wt * (delta_p.dot(delta_p)) + 0.5 * wr * (dt.dot(dt));
}

VectorXd Solver::grad(const VectorXd &x, const Vector2d &pt) {
    int n = x.rows();
    assert(n == dof);

    MatrixXd dP(2,n);
    dP = dp(x);
    Vector2d delta_p;
    Vector2d _p = p(x);
    delta_p = _p - pt;

    VectorXd coeffWise(n);
    for (int i = 0; i < n; ++i) {
        coeffWise(i) = delta_p.dot(dP.col(i));
    }

    VectorXd sol = (wt * coeffWise) + (wr * x);
    return sol;
}

MatrixXd Solver::Hess(const VectorXd &x, const Vector2d &pt) {
    
    int n = x.rows();
    MatrixXd I(n,n);
    I.setIdentity();

    MatrixXd dP = dp(x);
    Tensor<double,3> ddP = ddp(x);

    VectorXd delta_p(n);
    delta_p = p(x) - pt;

    MatrixXd dpddP(n,n);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            VectorXd p(2);
            p << ddP(i,j,0), ddP(i,j,1);

            dpddP(i,j) = delta_p.transpose() * p;
        }
    }

    MatrixXd _H = wt * (dP.transpose() * dP + dpddP) + wr * I;
    return _H;
}

VectorXd Solver::optimize(const VectorXd &xInit, const Vector2d target){

	int n = xInit.rows();
	VectorXd x = xInit;

	VectorXd g(n);
    g.setZero();

	VectorXd dx(n);
    dx.setZero();

    VectorXd dx_temp(n);
    dx.setZero();

    MatrixXd H(n,n);
    H.setZero();

	double f_GD, f_NM, f_dx, alpha;
	
	for (iter = 1; iter < iterMax; ++iter) {
		f_GD = eval(x, g, target);

        grad_debug(x,f_GD,g, target);

		alpha = alphaInit;
        int iterLs;
		for (iterLs = 1; iterLs < iterLsMax; ++iterLs) {
			dx_temp = -alpha * g;
			f_dx = eval(x + dx_temp, target);
			if (f_dx < f_GD) {
                dx = dx_temp;
				break;
            }
			alpha *= gamma;
		}
		
		x += dx;
		if (dx.norm() < tol) {
            std::cout<<"GDLS iterations: "<<iter<<std::endl;
			return x;
        }
	}
    std::cout<<"GDLS iterations: "<<iter<<std::endl;

    for (int i = 0; i < n; ++i) {
        while (x[i] > pi) 
            x[i] -= 2.0 * pi;
        while (x[i] < -pi) 
            x[i] += 2.0 * pi;
    }
    VectorXd temp = x;

    for (iter = 1; iter < iterMax; ++iter) {
		f_NM = eval(x, g, H, target);

        hess_debug(x,g,H, target);

        alpha = 1.0;
        int iterLs;
		for (iterLs = 1; iterLs < iterLsMax; ++iterLs) {
			dx_temp = -alpha * H.inverse() * g;
			f_dx = eval(x + dx_temp, target);
			if (f_dx < f_NM) {
                dx = dx_temp;
				break;
            }
			alpha *= gamma;
		}
		x += dx;
		if (dx.norm() < tol) {
			break;
        }
	}
    std::cout<<"NM iterations: "<<iter<<std::endl;

    for (int i = 0; i < n; ++i) {
        while (x[i] >= pi) 
            x[i] -= 2.0 * pi;
        while (temp[i] >= pi)
            temp[i] -= 2.0 * pi;

        while (x[i] <= -pi) 
            x[i] += 2.0 * pi;
        while(temp[i] <= -pi)
            temp[i] += 2.0 * pi;
    }

	return f_NM < f_GD ? x : temp;
}

double Solver::eval(const VectorXd &x, const Vector2d &pt){
	return f(x, pt);
}

double Solver::eval(const VectorXd &x, VectorXd &g, const Vector2d &pt){
	g = grad(x, pt);
	return f(x, pt);
}

double Solver::eval(const VectorXd &x, VectorXd &g, MatrixXd &H, const Vector2d &pt){
	H = Hess(x, pt);
	g = grad(x, pt);
	return f(x, pt);
}

void Solver::grad_debug(const VectorXd &x, const double func, const VectorXd& grad, const Vector2d &pt) {
    if (verbose) {
        int n = grad.rows();
        double e = 1e-7;
        VectorXd g_(n);
        for(int i = 0; i < n; ++i) {
            VectorXd theta_ = x;
            theta_(i) += e;
            double f_ = eval(theta_, pt);
            g_(i) = (f_ - func)/e;
        }
        std::cout << "g: " << (g_ - grad).norm() << std::endl;
    }
}

void Solver::hess_debug(const VectorXd &x, const VectorXd& grad, const MatrixXd& Hess, const Vector2d &pt) {
    if (verbose){
        int n = grad.rows();
        double e = 1e-7;
        MatrixXd H_(n, n);
        for(int i = 0; i < n; ++i) {
            VectorXd theta_ = x;
            theta_(i) += e;
            VectorXd g_(n);
            eval(theta_, g_, pt);
            H_.col(i) = (g_ - grad)/e;
        }
        std::cout << "H: " << (H_ - Hess).norm() << std::endl;
    }
}