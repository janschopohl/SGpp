// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <sgpp/datadriven/datamining/modules/hpo/FitterFactory.hpp>
#include <sgpp/datadriven/datamining/configuration/DataMiningConfigParser.hpp>
#include <sgpp/datadriven/datamining/modules/fitting/ModelFittingBase.hpp>
#include <sgpp/datadriven/datamining/modules/hpo/harmonica/ConfigurationBit.hpp>
#include <sgpp/datadriven/datamining/modules/hpo/parameters/ContinuousParameter.hpp>
#include <sgpp/datadriven/datamining/modules/hpo/parameters/DiscreteParameter.hpp>
#include <sgpp/datadriven/datamining/modules/fitting/FitterConfigurationLeastSquares.hpp>

#include <vector>

namespace sgpp {
namespace datadriven {

/**
 * Concrete factory to build instances of
 * #sgpp::datadriven::ModelFittingDensityDerivativeRatioEstimation
 */
class DensityDerivativeRatioEstimationFitterFactory : public FitterFactory {
 public:
  /**
   * Default constructor
   */
  explicit DensityDerivativeRatioEstimationFitterFactory(const DataMiningConfigParser &parser);

  /**
   * Assemble a #sgpp::datadriven::ModelFittingDensityDerivativeRatioEstimation object based on the
   * configuration determined by a previous set_() call.
   * @return Fully configured instance of a
   * #sgpp::datadriven::ModelFittingDensityDerivativeRatioEstimation object.
   */
  ModelFittingBase *buildFitter() override;

 protected:
  /**
   * Configuration for all parameters that are not optimized
   */
  FitterConfigurationLeastSquares baseConfig;
};
} /* namespace datadriven */
} /* namespace sgpp */