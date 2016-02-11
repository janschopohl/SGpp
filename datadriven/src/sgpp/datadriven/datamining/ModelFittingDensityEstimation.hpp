// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <sgpp/globaldef.hpp>

#include <sgpp/datadriven/datamining/ModelFittingBase.hpp>
#include <sgpp/datadriven/datamining/DataMiningConfiguration.hpp>
#include <sgpp/datadriven/datamining/DataMiningConfigurationDensityEstimation.hpp>
#include <sgpp/datadriven/datamining/SampleProvider.hpp>
#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/datatypes/DataMatrix.hpp>

namespace SGPP {
namespace datadriven {

class ModelFittingDensityEstimation: public datadriven::ModelFittingBase {
public:
	ModelFittingDensityEstimation(
			datadriven::DataMiningConfigurationDensityEstimation config);

	virtual ~ModelFittingDensityEstimation();

	void fit(datadriven::Dataset& dataset) override;

private:
	datadriven::DataMiningConfigurationDensityEstimation& configuration;
};

} /* namespace datadriven */
} /* namespace SGPP */