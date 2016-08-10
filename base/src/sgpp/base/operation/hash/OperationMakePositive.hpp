// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/operation/hash/OperationMakePositiveCandidateSetAlgorithm.hpp>
#include <sgpp/base/operation/hash/OperationMakePositiveInterpolationAlgorithm.hpp>

#include <sgpp/globaldef.hpp>

#include <vector>
#include <map>

namespace sgpp {
namespace base {

enum class MakePositiveCandidateSearchAlgorithm { FullGrid, Intersections };
enum class MakePositiveInterpolationAlgorithm { SetToZero };

/**
 * This class enforces the function value range of a sparse grid function to be larger than 0.
 * It uses a discretization based approach where we add the minimum amount of full grid points
 * to enforce the positivity.
 */
class OperationMakePositive {
 public:
  typedef std::map<size_t, base::HashGridPoint> gridPointCandidatesMap;

  /**
   * Constructor
   *
   * @param grid Grid
   * @param candidateSearchAlgorithm defines how to generate the full grid candidate set
   * @param interpolationAlgorithm defines how to compute the coefficients for the new grid points
   * @param verbose print information or not
   */
  explicit OperationMakePositive(base::Grid& grid,
                                 MakePositiveCandidateSearchAlgorithm candidateSearchAlgorithm =
                                     MakePositiveCandidateSearchAlgorithm::Intersections,
                                 MakePositiveInterpolationAlgorithm interpolationAlgorithm =
                                     MakePositiveInterpolationAlgorithm::SetToZero,
                                 bool verbose = false);

  /**
   * Descrutor
   */
  virtual ~OperationMakePositive();

  /**
   * Make the sparse grid function defined by grid and coefficient vector positive.
   *
   * @param newGrid Grid where the new grid is stored
   * @param newAlpha coefficient vector of new grid
   * @param resetGrid if set, the grid in newGrid is deleted and a copy of the object variable is
   * used
   */
  void makePositive(base::Grid*& newGrid, base::DataVector& newAlpha, bool resetGrid = true);

  /**
   *
   * @return number of newly added grid points
   */
  size_t numAddedGridPoints();

  /**
   *
   * @return number of newly added grid points for guaranteeing positivity
   */
  size_t numAddedGridPointsForPositivity();

 private:
  /**
   * Enforce the function values at each grid point to larger than the specified tolerance. The ones
   * which are not are set to zero. For this function we need the hierarchization and
   * dechierarchization operations.
   *
   * @param alpha coefficient vector
   */
  void makeCurrentNodalValuesPositive(base::DataVector& alpha, double tol = -1e-14);

  /**
   * Enforce the function values at the new grid points to be positive. It is similar to the
   * ´makeCurrentNodalValuesPositive´ but does the hierarchization directly.
   *
   * @param grid Grid
   * @param alpha coefficient vector
   * @param newGridPoints new grid points for which we need to compute the coefficients
   * @param tol tolerance for positivity
   */
  void forceNewNodalValuesToBePositive(base::Grid& grid, base::DataVector& alpha,
                                       std::vector<size_t>& newGridPoints, double tol = -1e-14);

  /**
   * Extract the non existing candidates from the candidate set which have the currently checked
   * level sum.
   *
   * @param newGrid Grid
   * @param candidates candidate set
   * @param currentLevelSum currently checked grid points with this level sum
   * @param finalCandidates candidate set where all the grid points do not exist in newGrid and
   * their level sum is equal to currentLevelSum
   */
  void extractNonExistingCandidatesByLevelSum(
      Grid& newGrid, std::vector<std::shared_ptr<base::HashGridPoint>>& candidates,
      size_t currentLevelSum, std::vector<std::shared_ptr<base::HashGridPoint>>& finalCandidates);

  /**
   * Adds all the candidates from which the function value is smaller than the tolerance.
   *
   * @param grid Grid
   * @param alpha coefficient vector
   * @param candidates candidate set
   * @param addedGridPoints newly added grid points
   * @param tol tolerance
   */
  void addFullGridPoints(base::Grid& grid, base::DataVector& alpha,
                         std::vector<std::shared_ptr<base::HashGridPoint>>& candidates,
                         std::vector<size_t>& addedGridPoints, double tol = -1e-14);

  /// grid
  base::Grid& grid;

  /// range for level sums to be tested
  size_t minimumLevelSum;
  size_t maximumLevelSum;

  /// number of needed new grid points
  size_t numNewGridPointsForPositivity;
  /// number of infact newly added grid points
  size_t numNewGridPoints;

  /// candidate search algorithm
  std::shared_ptr<base::OperationMakePositiveCandidateSetAlgorithm> candidateSearch;
  std::shared_ptr<base::OperationMakePositiveInterpolationAlgorithm> interpolationMethod;

  /// verbosity
  bool verbose;
};

} /* namespace base */
} /* namespace sgpp */
