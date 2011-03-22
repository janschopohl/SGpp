/******************************************************************************
* Copyright (C) 2010 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#ifndef OPERATIONMULTIPLEEVALITERATIVEAVXLINEARSTRETCHED_HPP
#define OPERATIONMULTIPLEEVALITERATIVEAVXLINEARSTRETCHED_HPP

#include "operation/datadriven/OperationMultipleEvalVectorized.hpp"
#include "grid/GridStorage.hpp"
#include "tools/common/SGppStopwatch.hpp"

namespace sg
{

/**
 * This class implements OperationMultipleEval for a grids with LinearStretched basis ansatzfunctions without boundaries
 *
 * However in this case high efficient vector code (AVX instructions) are generated
 * to implement a iterative OperationB version. In addition cache blocking is used
 * in order to assure a most efficient cache usage.
 *
 * IMPORTANT REMARK:
 * In order to use this routine you have to keep following points in mind (for multVectorized and multTransposeVectorized):
 * 	- data MUST a have even number of points AND it must be transposed
 *  - result MUST have the same size as data points that should be evaluated
 */
class OperationMultipleEvalIterativeAVXLinearStretched : public OperationMultipleEvalVectorized
{
public:
	/**
	 * Construtor of OperationBLinearStretched
	 *
	 * Within the construct DataMatrix Level and DataMatrix Index are set up.
	 * If the grid changes during your calculations and you don't want to create
	 * a new instance of this class, you have to call rebuildLevelAndIndex before
	 * doing any further mult or multTranspose calls.
	 *
	 * @param storage Pointer to the grid's gridstorage obejct
	 * @param dataset dataset that should be evaluated
	 */
	OperationMultipleEvalIterativeAVXLinearStretched(GridStorage* storage, DataMatrix* dataset);

	/**
	 * Destructor
	 */
	virtual ~OperationMultipleEvalIterativeAVXLinearStretched();

	virtual double multVectorized(DataVector& alpha, DataVector& result);

	virtual double multTransposeVectorized(DataVector& source, DataVector& result);

	virtual void rebuildLevelAndIndex();

protected:
	/// Pointer to the grid's gridstorage object
	GridStorage* storage;
	/// Timer object to handle time measurements
	SGppStopwatch* myTimer;
};

}

#endif /* OPERATIONMULTIPLEEVALITERATIVEAVXLinearStretched_HPP */