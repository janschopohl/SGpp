/******************************************************************************
* Copyright (C) 2011 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#include "tools/MPI/SGppMPITools.hpp"
#include "solver/sle/BiCGStabMPI.hpp"
#include "algorithm/pde/BlackScholesParabolicPDESolverSystemEuropeanParallelMPI.hpp"
#include "application/pde/BlackScholesSolverMPI.hpp"

#include "solver/ode/Euler.hpp"
#include "solver/ode/CrankNicolson.hpp"
#include "solver/ode/AdamsBashforth.hpp"
#include "solver/ode/VarTimestep.hpp"
#include "solver/ode/StepsizeControlH.hpp"
#include "solver/ode/StepsizeControlBDF.hpp"
#include "solver/ode/StepsizeControlEJ.hpp"
#include "grid/Grid.hpp"
#include "exception/application_exception.hpp"
#include "basis/operations_factory.hpp"
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <fstream>
#include <iomanip>

namespace sg
{
namespace parallel
{

BlackScholesSolverMPI::BlackScholesSolverMPI(bool useLogTransform, std::string OptionType) : pde::ParabolicPDESolver()
{
	this->bStochasticDataAlloc = false;
	this->bGridConstructed = false;
	this->myScreen = NULL;
	this->useCoarsen = false;
	this->coarsenThreshold = 0.0;
	this->adaptSolveMode = "none";
	this->refineMode = "classic";
	this->numCoarsenPoints = -1;
	this->useLogTransform = useLogTransform;
	this->refineMaxLevel = 0;
	this->nNeededIterations = 0;
	this->dNeededTime = 0.0;
	this->staInnerGridSize = 0;
	this->finInnerGridSize = 0;
	this->avgInnerGridSize = 0;
	if (OptionType == "European")
	{
		this->tOptionType = OptionType;
	}
	else
	{
		throw new application_exception("BlackScholesSolverMPI::BlackScholesSolverMPI : An unsupported option type has been chosen! all & European are supported!");
	}
}

BlackScholesSolverMPI::~BlackScholesSolverMPI()
{
	if (this->bStochasticDataAlloc)
	{
		delete this->mus;
		delete this->sigmas;
		delete this->rhos;
	}
	if (this->myScreen != NULL)
	{
		delete this->myScreen;
	}
}

void BlackScholesSolverMPI::getGridNormalDistribution(DataVector& alpha, std::vector<double>& norm_mu, std::vector<double>& norm_sigma)
{
	if (this->bGridConstructed)
	{
		double tmp;
		double value;
		StdNormalDistribution myNormDistr;

		for (size_t i = 0; i < this->myGrid->getStorage()->size(); i++)
		{
			std::string coords = this->myGridStorage->get(i)->getCoordsStringBB(*(this->myBoundingBox));
			std::stringstream coordsStream(coords);

			value = 1.0;
			for (size_t j = 0; j < this->dim; j++)
			{
				coordsStream >> tmp;

				if (this->useLogTransform == false)
				{
					value *= myNormDistr.getDensity(tmp, norm_mu[j], norm_sigma[j]);
				}
				else
				{
					value *= myNormDistr.getDensity(exp(tmp), norm_mu[j], norm_sigma[j]);
				}
			}

			alpha[i] = value;
		}
	}
	else
	{
		throw new application_exception("BlackScholesSolverMPI::getGridNormalDistribution : The grid wasn't initialized before!");
	}
}

void BlackScholesSolverMPI::constructGrid(BoundingBox& BoundingBox, size_t level)
{
	this->dim = BoundingBox.getDimensions();
	this->levels = level;

	this->myGrid = new LinearTrapezoidBoundaryGrid(BoundingBox);

	GridGenerator* myGenerator = this->myGrid->createGridGenerator();
	myGenerator->regular(this->levels);
	delete myGenerator;

	this->myBoundingBox = this->myGrid->getBoundingBox();
	this->myGridStorage = this->myGrid->getStorage();

	//std::string serGrid;
	//myGrid->serialize(serGrid);
	//std::cout << serGrid << std::endl;

	this->bGridConstructed = true;
}

void BlackScholesSolverMPI::refineInitialGridWithPayoff(DataVector& alpha, double strike, std::string payoffType, double dStrikeDistance)
{
	size_t nRefinements = 0;

	if (this->useLogTransform == false)
	{
		if (this->bGridConstructed)
		{

			DataVector refineVector(alpha.getSize());

			if (payoffType == "std_euro_call" || payoffType == "std_euro_put")
			{
				double tmp;
				double* dblFuncValues = new double[dim];
				double dDistance = 0.0;

				for (size_t i = 0; i < this->myGrid->getStorage()->size(); i++)
				{
					std::string coords = this->myGridStorage->get(i)->getCoordsStringBB(*(this->myBoundingBox));
					std::stringstream coordsStream(coords);

					for (size_t j = 0; j < this->dim; j++)
					{
						coordsStream >> tmp;

						dblFuncValues[j] = tmp;
					}

					tmp = 0.0;
					for (size_t j = 0; j < this->dim; j++)
					{
						tmp += dblFuncValues[j];
					}

					if (payoffType == "std_euro_call")
					{
						dDistance = fabs(((tmp/static_cast<double>(this->dim))-strike));
					}
					if (payoffType == "std_euro_put")
					{
						dDistance = fabs((strike-(tmp/static_cast<double>(this->dim))));
					}

					if (dDistance <= dStrikeDistance)
					{
						refineVector[i] = dDistance;
						nRefinements++;
					}
					else
					{
						refineVector[i] = 0.0;
					}
				}

				delete[] dblFuncValues;

				SurplusRefinementFunctor* myRefineFunc = new SurplusRefinementFunctor(&refineVector, nRefinements, 0.0);

				this->myGrid->createGridGenerator()->refine(myRefineFunc);

				delete myRefineFunc;

				alpha.resize(this->myGridStorage->size());

				// reinit the grid with the payoff function
				initGridWithPayoff(alpha, strike, payoffType);
			}
			else
			{
				throw new application_exception("BlackScholesSolverMPI::refineInitialGridWithPayoff : An unsupported payoffType was specified!");
			}
		}
		else
		{
			throw new application_exception("BlackScholesSolverMPI::refineInitialGridWithPayoff : The grid wasn't initialized before!");
		}
	}
}

void BlackScholesSolverMPI::refineInitialGridWithPayoffToMaxLevel(DataVector& alpha, double strike, std::string payoffType, double dStrikeDistance, size_t maxLevel)
{
	size_t nRefinements = 0;

	if (this->useLogTransform == false)
	{
		if (this->bGridConstructed)
		{

			DataVector refineVector(alpha.getSize());

			if (payoffType == "std_euro_call" || payoffType == "std_euro_put")
			{
				double tmp;
				double* dblFuncValues = new double[dim];
				double dDistance = 0.0;

				for (size_t i = 0; i < this->myGrid->getStorage()->size(); i++)
				{
					std::string coords = this->myGridStorage->get(i)->getCoordsStringBB(*this->myBoundingBox);
					std::stringstream coordsStream(coords);

					for (size_t j = 0; j < this->dim; j++)
					{
						coordsStream >> tmp;

						dblFuncValues[j] = tmp;
					}

					tmp = 0.0;
					for (size_t j = 0; j < this->dim; j++)
					{
						tmp += dblFuncValues[j];
					}

					if (payoffType == "std_euro_call")
					{
						dDistance = fabs(((tmp/static_cast<double>(this->dim))-strike));
					}
					if (payoffType == "std_euro_put")
					{
						dDistance = fabs((strike-(tmp/static_cast<double>(this->dim))));
					}

					if (dDistance <= dStrikeDistance)
					{
						refineVector[i] = dDistance;
						nRefinements++;
					}
					else
					{
						refineVector[i] = 0.0;
					}
				}

				delete[] dblFuncValues;

				SurplusRefinementFunctor* myRefineFunc = new SurplusRefinementFunctor(&refineVector, nRefinements, 0.0);

				this->myGrid->createGridGenerator()->refineMaxLevel(myRefineFunc, maxLevel);

				delete myRefineFunc;

				alpha.resize(this->myGridStorage->size());

				// reinit the grid with the payoff function
				initGridWithPayoff(alpha, strike, payoffType);
			}
			else
			{
				throw new application_exception("BlackScholesSolverMPI::refineInitialGridWithPayoffToMaxLevel : An unsupported payoffType was specified!");
			}
		}
		else
		{
			throw new application_exception("BlackScholesSolverMPI::refineInitialGridWithPayoffToMaxLevel : The grid wasn't initialized before!");
		}
	}
}

void BlackScholesSolverMPI::setStochasticData(DataVector& mus, DataVector& sigmas, DataMatrix& rhos, double r)
{
	this->mus = new DataVector(mus);
	this->sigmas = new DataVector(sigmas);
	this->rhos = new DataMatrix(rhos);
	this->r = r;

	bStochasticDataAlloc = true;
}

void BlackScholesSolverMPI::solveExplicitEuler(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose, bool generateAnimation, size_t numEvalsAnimation)
{
	if (this->bGridConstructed && this->bStochasticDataAlloc)
	{
		solver::Euler* myEuler = new solver::Euler("ExEul", numTimesteps, timestepsize, generateAnimation, numEvalsAnimation, myScreen);
		parallel::BiCGStabMPI* myCG = new parallel::BiCGStabMPI(maxCGIterations, epsilonCG);
		pde::OperationParabolicPDESolverSystem* myBSSystem = NULL;

		myBSSystem = new parallel::BlackScholesParabolicPDESolverSystemEuropeanParallelMPI(*this->myGrid, alpha, *this->mus, *this->sigmas, *this->rhos, this->r, timestepsize, "ExEul", this->useLogTransform, this->useCoarsen, this->coarsenThreshold, this->adaptSolveMode, this->numCoarsenPoints, this->refineThreshold, this->refineMode, this->refineMaxLevel);

		base::SGppStopwatch* myStopwatch = new base::SGppStopwatch();
		this->staInnerGridSize = getNumberInnerGridPoints();

		std::cout << "Using Explicit Euler to solve " << numTimesteps << " timesteps:" << std::endl;
		myStopwatch->start();
		myEuler->solve(*myCG, *myBSSystem, true, verbose);
		this->dNeededTime = myStopwatch->stop();

		std::cout << std::endl << "Final Grid size: " << getNumberGridPoints() << std::endl;
		std::cout << "Final Grid size (inner): " << getNumberInnerGridPoints() << std::endl << std::endl << std::endl;

		std::cout << "Average Grid size: " << static_cast<double>(myBSSystem->getSumGridPointsComplete())/static_cast<double>(numTimesteps) << std::endl;
		std::cout << "Average Grid size (Inner): " << static_cast<double>(myBSSystem->getSumGridPointsInner())/static_cast<double>(numTimesteps) << std::endl << std::endl << std::endl;

		if (this->myScreen != NULL)
		{
			std::cout << "Time to solve: " << this->dNeededTime << " seconds" << std::endl;
			this->myScreen->writeEmptyLines(2);
		}

		this->finInnerGridSize = getNumberInnerGridPoints();
		this->avgInnerGridSize = static_cast<size_t>((static_cast<double>(myBSSystem->getSumGridPointsInner())/static_cast<double>(numTimesteps))+0.5);
		this->nNeededIterations = myEuler->getNumberIterations();

		delete myBSSystem;
		delete myCG;
		delete myEuler;
		delete myStopwatch;
	}
	else
	{
		throw new application_exception("BlackScholesSolverMPI::solveExplicitEuler : A grid wasn't constructed before or stochastic parameters weren't set!");
	}
}

void BlackScholesSolverMPI::solveImplicitEuler(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose, bool generateAnimation, size_t numEvalsAnimation)
{
	if (this->bGridConstructed && this->bStochasticDataAlloc)
	{
		solver::Euler* myEuler = new solver::Euler("ImEul", numTimesteps, timestepsize, generateAnimation, numEvalsAnimation, myScreen);
		parallel::BiCGStabMPI* myCG = new parallel::BiCGStabMPI(maxCGIterations, epsilonCG);
		pde::OperationParabolicPDESolverSystem* myBSSystem = NULL;

		myBSSystem = new parallel::BlackScholesParabolicPDESolverSystemEuropeanParallelMPI(*this->myGrid, alpha, *this->mus, *this->sigmas, *this->rhos, this->r, timestepsize, "ImEul", this->useLogTransform, this->useCoarsen, this->coarsenThreshold, this->adaptSolveMode, this->numCoarsenPoints, this->refineThreshold, this->refineMode, this->refineMaxLevel);

		base::SGppStopwatch* myStopwatch = new base::SGppStopwatch();
		this->staInnerGridSize = getNumberInnerGridPoints();

		std::cout << "Using Implicit Euler to solve " << numTimesteps << " timesteps:" << std::endl;
		myStopwatch->start();
		myEuler->solve(*myCG, *myBSSystem, true, verbose);
		this->dNeededTime = myStopwatch->stop();

		std::cout << std::endl << "Final Grid size: " << getNumberGridPoints() << std::endl;
		std::cout << "Final Grid size (inner): " << getNumberInnerGridPoints() << std::endl << std::endl << std::endl;

		std::cout << "Average Grid size: " << static_cast<double>(myBSSystem->getSumGridPointsComplete())/static_cast<double>(numTimesteps) << std::endl;
		std::cout << "Average Grid size (Inner): " << static_cast<double>(myBSSystem->getSumGridPointsInner())/static_cast<double>(numTimesteps) << std::endl << std::endl << std::endl;

		if (this->myScreen != NULL)
		{
			std::cout << "Time to solve: " << this->dNeededTime << " seconds" << std::endl;
			this->myScreen->writeEmptyLines(2);
		}

		this->finInnerGridSize = getNumberInnerGridPoints();
		this->avgInnerGridSize = static_cast<size_t>((static_cast<double>(myBSSystem->getSumGridPointsInner())/static_cast<double>(numTimesteps))+0.5);
		this->nNeededIterations = myEuler->getNumberIterations();

		delete myBSSystem;
		delete myCG;
		delete myEuler;
		delete myStopwatch;
	}
	else
	{
		throw new application_exception("BlackScholesSolver::solveImplicitEuler : A grid wasn't constructed before or stochastic parameters weren't set!");
	}
}

void BlackScholesSolverMPI::solveCrankNicolson(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, size_t NumImEul)
{
	if (this->bGridConstructed && this->bStochasticDataAlloc)
	{
		parallel::BiCGStabMPI* myCG = new parallel::BiCGStabMPI(maxCGIterations, epsilonCG);
		pde::OperationParabolicPDESolverSystem* myBSSystem = NULL;

		myBSSystem = new parallel::BlackScholesParabolicPDESolverSystemEuropeanParallelMPI(*this->myGrid, alpha, *this->mus, *this->sigmas, *this->rhos, this->r, timestepsize, "CrNic", this->useLogTransform, this->useCoarsen, this->coarsenThreshold, this->adaptSolveMode, this->numCoarsenPoints, this->refineThreshold, this->refineMode, this->refineMaxLevel);

		base::SGppStopwatch* myStopwatch = new base::SGppStopwatch();
		this->staInnerGridSize = getNumberInnerGridPoints();

		size_t numCNSteps;
		size_t numIESteps;

		numCNSteps = numTimesteps;
		if (numTimesteps > NumImEul)
		{
			numCNSteps = numTimesteps - NumImEul;
		}
		numIESteps = NumImEul;

		solver::Euler* myEuler = new solver::Euler("ImEul", numIESteps, timestepsize, false, 0, this->myScreen);
		solver::CrankNicolson* myCN = new solver::CrankNicolson(numCNSteps, timestepsize, this->myScreen);

		myStopwatch->start();
		if (numIESteps > 0)
		{
			std::cout << "Using Implicit Euler to solve " << numIESteps << " timesteps:" << std::endl;
			myBSSystem->setODESolver("ImEul");
			myEuler->solve(*myCG, *myBSSystem, false, false);
		}
		myBSSystem->setODESolver("CrNic");
		std::cout << "Using Crank Nicolson to solve " << numCNSteps << " timesteps:" << std::endl << std::endl << std::endl << std::endl;
		myCN->solve(*myCG, *myBSSystem, true, false);
		this->dNeededTime = myStopwatch->stop();

		std::cout << std::endl << "Final Grid size: " << getNumberGridPoints() << std::endl;
		std::cout << "Final Grid size (inner): " << getNumberInnerGridPoints() << std::endl << std::endl << std::endl;

		std::cout << "Average Grid size: " << static_cast<double>(myBSSystem->getSumGridPointsComplete())/static_cast<double>(numTimesteps) << std::endl;
		std::cout << "Average Grid size (Inner): " << static_cast<double>(myBSSystem->getSumGridPointsInner())/static_cast<double>(numTimesteps) << std::endl << std::endl << std::endl;

		if (this->myScreen != NULL)
		{
			std::cout << "Time to solve: " << this->dNeededTime << " seconds" << std::endl;
			this->myScreen->writeEmptyLines(2);
		}

		this->finInnerGridSize = getNumberInnerGridPoints();
		this->avgInnerGridSize = static_cast<size_t>((static_cast<double>(myBSSystem->getSumGridPointsInner())/static_cast<double>(numTimesteps))+0.5);
		this->nNeededIterations = myEuler->getNumberIterations() + myCN->getNumberIterations();

		delete myBSSystem;
		delete myCG;
		delete myCN;
		delete myEuler;
		delete myStopwatch;
	}
	else
	{
		throw new application_exception("BlackScholesSolver::solveCrankNicolson : A grid wasn't constructed before or stochastic parameters weren't set!");
	}
}


void BlackScholesSolverMPI::solveAdamsBashforth(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose)
{
	throw new application_exception("BlackScholesSolverMPI::solveAdamsBashforth : An unsupported ODE Solver type has been chosen!");
}

void BlackScholesSolverMPI::solveSCAC(size_t numTimesteps, double timestepsize, double epsilon, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose)
{
	throw new application_exception("BlackScholesSolverMPI::solveSCAC : An unsupported ODE Solver type has been chosen!");
}

void BlackScholesSolverMPI::solveSCH(size_t numTimesteps, double timestepsize, double epsilon, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose)
{
	throw new application_exception("BlackScholesSolverMPI::solveSCH : An unsupported ODE Solver type has been chosen!");
}

void BlackScholesSolverMPI::solveSCBDF(size_t numTimesteps, double timestepsize, double epsilon, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose)
{
	throw new application_exception("BlackScholesSolverMPI::solveSCBDF : An unsupported ODE Solver type has been chosen!");
}

void BlackScholesSolverMPI::solveSCEJ(size_t numTimesteps, double timestepsize, double epsilon, double myAlpha, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose)
{
	throw new application_exception("BlackScholesSolverMPI::solveSCEJ : An unsupported ODE Solver type has been chosen!");
}

void BlackScholesSolverMPI::solveX(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose, void *myODESolverV, std::string Solver)
{
	throw new application_exception("BlackScholesSolverMPI::solveX : An unsupported ODE Solver type has been chosen!");
}

void BlackScholesSolverMPI::initGridWithPayoff(DataVector& alpha, double strike, std::string payoffType)
{
	if (this->useLogTransform)
	{
		initLogTransformedGridWithPayoff(alpha, strike, payoffType);
	}
	else
	{
		initCartesianGridWithPayoff(alpha, strike, payoffType);
	}
}

double BlackScholesSolverMPI::get1DEuroCallPayoffValue(double assetValue, double strike)
{
	if (assetValue <= strike)
	{
		return 0.0;
	}
	else
	{
		return assetValue - strike;
	}
}

std::vector<size_t> BlackScholesSolverMPI::getAlgorithmicDimensions()
{
	return this->myGrid->getAlgorithmicDimensions();
}

void BlackScholesSolverMPI::setAlgorithmicDimensions(std::vector<size_t> newAlgoDims)
{
	if (tOptionType == "all")
	{
		this->myGrid->setAlgorithmicDimensions(newAlgoDims);
	}
	else
	{
		throw new application_exception("BlackScholesSolver::setAlgorithmicDimensions : Set algorithmic dimensions is only supported when choosing option type all!");
	}
}

void BlackScholesSolverMPI::initScreen()
{
	this->myScreen = new ScreenOutput();
	this->myScreen->writeTitle("SGpp - Black Scholes Solver, 2.0.0", "TUM (C) 2009-2010, by Alexander Heinecke");
	this->myScreen->writeStartSolve("Multidimensional Black Scholes Solver");
}

void BlackScholesSolverMPI::setEnableCoarseningData(std::string adaptSolveMode, std::string refineMode, size_t refineMaxLevel, int numCoarsenPoints, double coarsenThreshold, double refineThreshold)
{
	this->useCoarsen = true;
	this->coarsenThreshold = coarsenThreshold;
	this->refineThreshold = refineThreshold;
	this->refineMaxLevel = refineMaxLevel;
	this->adaptSolveMode = adaptSolveMode;
	this->refineMode = refineMode;
	this->numCoarsenPoints = numCoarsenPoints;
}

void BlackScholesSolverMPI::initCartesianGridWithPayoff(DataVector& alpha, double strike, std::string payoffType)
{
	double tmp;

	if (this->bGridConstructed)
	{
		for (size_t i = 0; i < this->myGrid->getStorage()->size(); i++)
		{
			std::string coords = this->myGridStorage->get(i)->getCoordsStringBB(*this->myBoundingBox);
			std::stringstream coordsStream(coords);
			double* dblFuncValues = new double[dim];

			for (size_t j = 0; j < this->dim; j++)
			{
				coordsStream >> tmp;

				dblFuncValues[j] = tmp;
			}

			if (payoffType == "std_euro_call")
			{
				tmp = 0.0;
				for (size_t j = 0; j < dim; j++)
				{
					tmp += dblFuncValues[j];
				}
				alpha[i] = std::max<double>(((tmp/static_cast<double>(dim))-strike), 0.0);
			}
			else if (payoffType == "std_euro_put")
			{
				tmp = 0.0;
				for (size_t j = 0; j < dim; j++)
				{
					tmp += dblFuncValues[j];
				}
				alpha[i] = std::max<double>(strike-((tmp/static_cast<double>(dim))), 0.0);
			}
			else
			{
				throw new application_exception("BlackScholesSolver::initCartesianGridWithPayoff : An unknown payoff-type was specified!");
			}

			delete[] dblFuncValues;
		}

		base::OperationHierarchisation* myHierarchisation = sg::GridOperationFactory::createOperationHierarchisation(*this->myGrid);
		myHierarchisation->doHierarchisation(alpha);
		delete myHierarchisation;
	}
	else
	{
		throw new application_exception("BlackScholesSolver::initCartesianGridWithPayoff : A grid wasn't constructed before!");
	}
}

void BlackScholesSolverMPI::initLogTransformedGridWithPayoff(DataVector& alpha, double strike, std::string payoffType)
{
	double tmp;

	if (this->bGridConstructed)
	{
		for (size_t i = 0; i < this->myGrid->getStorage()->size(); i++)
		{
			std::string coords = this->myGridStorage->get(i)->getCoordsStringBB(*this->myBoundingBox);
			std::stringstream coordsStream(coords);
			double* dblFuncValues = new double[dim];

			for (size_t j = 0; j < this->dim; j++)
			{
				coordsStream >> tmp;

				dblFuncValues[j] = tmp;
			}

			if (payoffType == "std_euro_call")
			{
				tmp = 0.0;
				for (size_t j = 0; j < dim; j++)
				{
					tmp += exp(dblFuncValues[j]);
				}
				alpha[i] = std::max<double>(((tmp/static_cast<double>(dim))-strike), 0.0);
			}
			else if (payoffType == "std_euro_put")
			{
				tmp = 0.0;
				for (size_t j = 0; j < dim; j++)
				{
					tmp += exp(dblFuncValues[j]);
				}
				alpha[i] = std::max<double>(strike-((tmp/static_cast<double>(dim))), 0.0);
			}
			else
			{
				throw new application_exception("BlackScholesSolver::initLogTransformedGridWithPayoff : An unknown payoff-type was specified!");
			}

			delete[] dblFuncValues;
		}

		base::OperationHierarchisation* myHierarchisation = sg::GridOperationFactory::createOperationHierarchisation(*this->myGrid);
		myHierarchisation->doHierarchisation(alpha);
		delete myHierarchisation;
	}
	else
	{
		throw new application_exception("BlackScholesSolver::initLogTransformedGridWithPayoff : A grid wasn't constructed before!");
	}
}

size_t BlackScholesSolverMPI::getNeededIterationsToSolve()
{
	return this->nNeededIterations;
}

double BlackScholesSolverMPI::getNeededTimeToSolve()
{
	return this->dNeededTime;
}

size_t BlackScholesSolverMPI::getStartInnerGridSize()
{
	return this->staInnerGridSize;
}

size_t BlackScholesSolverMPI::getFinalInnerGridSize()
{
	return this->finInnerGridSize;
}

size_t BlackScholesSolverMPI::getAverageInnerGridSize()
{
	return this->avgInnerGridSize;
}

}

}