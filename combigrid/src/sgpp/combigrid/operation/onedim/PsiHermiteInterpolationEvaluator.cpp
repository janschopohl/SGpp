// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/combigrid/operation/onedim/PsiHermiteInterpolationEvaluator.hpp>
#include <vector>

namespace sgpp {
namespace combigrid {

PsiHermiteInterpolationEvaluator::PsiHermiteInterpolationEvaluator()
    : evaluationPoint(0.0), basisCoefficients(), xValues() {}

PsiHermiteInterpolationEvaluator::~PsiHermiteInterpolationEvaluator() {}

PsiHermiteInterpolationEvaluator::PsiHermiteInterpolationEvaluator(
    const PsiHermiteInterpolationEvaluator& other)
    : evaluationPoint(other.evaluationPoint),
      basisCoefficients(other.basisCoefficients),
      xValues(other.xValues) {}




void PsiHermiteInterpolationEvaluator::computeBasisCoefficients() {
  size_t numPoints = xValues.size();

  basisCoefficients = std::vector<FloatScalarVector>(numPoints, FloatScalarVector(0.0));

  if (numPoints == 0) {
    return;
  }

  if (evaluationPoint <= xValues[0]) {
    if (std::abs(xValues[0]) > 1e-14) {
      basisCoefficients[0] = FloatScalarVector(evaluationPoint / xValues[0]);
    }
    return;
  }

  

  // TODO(holzmudd): could be optimized by using binary search
  for (size_t i = 1; i < numPoints; ++i) {
    if (evaluationPoint <= xValues[i]) {
      double x0 = xValues[i - 1];
      double x1 = xValues[i];


      // translate to uniformpoint
      double weight = (evaluationPoint-x0) / (x1 - x0);
      basisCoefficients[i - 1] = FloatScalarVector(evalUniform(weight));
      basisCoefficients[i] = FloatScalarVector(evalUniform(weight-1));

      return;
    }
  }

  // if we did not return in the loop, then evaluationPoint > all xValues...
  if (std::abs(xValues[numPoints - 1] - 1.0) > 1e-14) {
    basisCoefficients[numPoints - 1] =
        FloatScalarVector((evaluationPoint - 1.0) / (xValues[numPoints - 1] - 1.0));
  }
}

void PsiHermiteInterpolationEvaluator::setGridPoints(const std::vector<double>& newXValues) {
  xValues = newXValues;
  computeBasisCoefficients();
}

std::shared_ptr<AbstractLinearEvaluator<FloatScalarVector> >
PsiHermiteInterpolationEvaluator::cloneLinear() {
  return std::shared_ptr<AbstractLinearEvaluator<FloatScalarVector> >(
      new PsiHermiteInterpolationEvaluator(*this));
}

bool PsiHermiteInterpolationEvaluator::needsOrderedPoints() { return true; }

bool PsiHermiteInterpolationEvaluator::needsParameter() { return true; }

void PsiHermiteInterpolationEvaluator::setParameter(const FloatScalarVector& param) {
  evaluationPoint = param.value();
  computeBasisCoefficients();
}

double PsiHermiteInterpolationEvaluator::evalUniform(double x) {
  if (x > 1 || x < -1)
    return 0;

  else if (x < 0) {
    return -2.0 * pow(x, 3) + -3.0 * pow(x, 2) + 1.0;
  } else {
    return 2.0 * pow(x, 3) - 3.0 * pow(x, 2) + 1.0;
  }
}

} /* namespace combigrid */
} /* namespace sgpp*/