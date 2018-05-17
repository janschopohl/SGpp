/* Copyright (C) 2008-today The SG++ project
 * This file is part of the SG++ project. For conditions of distribution and
 * use, please see the copyright notice provided with SG++ or at
 * sgpp.sparsegrids.org
 *
 * DataTransformation.cpp
 *
 *  Created on: 30.01.2018
 *      Author: Lars Wolfsteller
 */

#include <sgpp/datadriven/datamining/modules/dataSource/DataTransformationBuilder.hpp>
#include <sgpp/datadriven/datamining/modules/dataSource/RosenblattTransformation.hpp>

namespace sgpp {
namespace datadriven {

DataTransformation* DataTransformationBuilder::buildTransformation(
    DataTransformationConfig config, Dataset* dataset) {
  if (config.type == DataTransformationType::ROSENBLATT) {
    return new RosenblattTransformation(dataset, config.rosenblattConfig);
  } else {
    return nullptr;
  }
}

} /* namespace datadriven */
} /* namespace sgpp */