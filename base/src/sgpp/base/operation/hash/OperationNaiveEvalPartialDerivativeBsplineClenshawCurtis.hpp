// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef OPERATIONEVALPARTIALDERIVATIVEBSPLINECLENSHAWCURTIS_HPP
#define OPERATIONEVALPARTIALDERIVATIVEBSPLINECLENSHAWCURTIS_HPP

#include <sgpp/globaldef.hpp>
#include <sgpp/base/operation/hash/OperationNaiveEvalPartialDerivative.hpp>
#include <sgpp/base/grid/GridStorage.hpp>
#include <sgpp/base/operation/hash/common/basis/BsplineClenshawCurtisBasis.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>

namespace SGPP {
  namespace base {

    /**
     * Operation for evaluating partial derivatives of B-spline
     * linear combinations on Clenshaw-Curtis grids.
     */
    class OperationNaiveEvalPartialDerivativeBsplineClenshawCurtis :
      public OperationNaiveEvalPartialDerivative {
      public:
        /**
         * Constructor.
         *
         * @param storage   storage of the sparse grid
         * @param degree    B-spline degree
         */
        OperationNaiveEvalPartialDerivativeBsplineClenshawCurtis(
          GridStorage* storage, size_t degree)
          : storage(storage),
            base(degree) {
        }

        /**
         * Virtual destructor.
         */
        virtual ~OperationNaiveEvalPartialDerivativeBsplineClenshawCurtis() {
        }

        /**
         * @param alpha     coefficient vector
         * @param point     evaluation point
         * @param derivDim  dimension in which the partial derivative should be taken
         * @return          value of the partial derivative of the linear combination
         */
        virtual float_t evalPartialDerivative(DataVector& alpha,
                                              DataVector& point,
                                              size_t derivDim);

      protected:
        /// storage of the sparse grid
        GridStorage* storage;
        /// 1D B-spline basis
        SBsplineClenshawCurtisBase base;
    };

  }
}

#endif /* OPERATIONEVALPARTIALDERIVATIVEBSPLINECLENSHAWCURTIS_HPP */