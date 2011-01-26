/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/

#ifndef STEPSIZECONTROLH_HPP
#define STEPSIZECONTROLH_HPP

#include "application/common/ScreenOutput.hpp"
#include "solver/ODESolver.hpp"
#include "algorithm/pde/BlackScholesODESolverSystem.hpp"
#include <string>

namespace sg
{

/**
 * This class implements a step size control using Crank-Nicolson with different step sizes
 * for solving ordinary partial equations
 *
 * @version $HEAD$
 */
class StepsizeControlH : public ODESolver
{
private:

	/// Pointer to ScreenOutput object
	ScreenOutput* myScreen;

	/// epsilon for the step size control
	double myEps;


public:
	/**
	 * Std-Constructer
	 *
	 * @param Mode the mode of the euler that should be executed, must be ExEul or ImEul
	 * @param imax number of maximum executed iterations
	 * @param timestepSize the size of one timestep
	 * @param eps the epsilon for the step size control
	 * @param screen possible pointer to a ScreenOutput object
	 */
	StepsizeControlH(size_t imax, double timestepSize, double eps, ScreenOutput* screen = NULL);

	/**
	 * Std-Destructor
	 */
	virtual ~StepsizeControlH();

	virtual void solve(SLESolver& LinearSystemSolver, OperationODESolverSystem& System, bool bIdentifyLastStep = false, bool verbose = false);
};

}

#endif /* STEPSIZECONTROLH_HPP */