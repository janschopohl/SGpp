// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/datadriven/operation/hash/OperationTestModLinear.hpp>
#include <sgpp/base/operation/hash/common/basis/LinearModifiedBasis.hpp>

#include <sgpp/datadriven/algorithm/test_dataset.hpp>


#include <sgpp/globaldef.hpp>


namespace SGPP {
  namespace datadriven {

    float_t OperationTestModLinear::test(SGPP::base::DataVector& alpha, SGPP::base::DataMatrix& data, SGPP::base::DataVector& classes) {
      base::LinearModifiedBasis<unsigned int, unsigned int> base;
      return test_dataset(this->storage, base, alpha, data, classes);
    }

    float_t OperationTestModLinear::testMSE(SGPP::base::DataVector& alpha, SGPP::base::DataMatrix& data, SGPP::base::DataVector& refValues) {
      base::LinearModifiedBasis<unsigned int, unsigned int> base;
      return test_dataset_mse(this->storage, base, alpha, data, refValues);
    }

    float_t OperationTestModLinear::testWithCharacteristicNumber(SGPP::base::DataVector& alpha, SGPP::base::DataMatrix& data, SGPP::base::DataVector& classes, SGPP::base::DataVector& charaNumbers) {
      base::LinearModifiedBasis<unsigned int, unsigned int> base;
      return test_datasetWithCharacteristicNumber(this->storage, base, alpha, data, classes, charaNumbers, 0.0);
    }

    void OperationTestModLinear::calculateROCcurve(SGPP::base::DataVector& alpha, SGPP::base::DataMatrix& data, SGPP::base::DataVector& classes, SGPP::base::DataVector& thresholds, SGPP::base::DataMatrix& ROC_curve) {
      base::LinearModifiedBasis<unsigned int, unsigned int> base;
      test_calculateROCcurve(this->storage, base, alpha, data, classes, thresholds, ROC_curve);
    }

  }
}