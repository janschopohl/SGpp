/*
 * AbstractGrowthStrategy.hpp
 *
 *  Created on: 04.12.2015
 *      Author: david
 */

#ifndef COMBIGRID_SRC_SGPP_COMBIGRID_GRID_POINTS_ABSTRACTGROWTHSTRATEGY_HPP_
#define COMBIGRID_SRC_SGPP_COMBIGRID_GRID_POINTS_ABSTRACTGROWTHSTRATEGY_HPP_

#include <cstddef>
#include <sgpp/globaldef.hpp>

namespace SGPP {
namespace combigrid {

/**
 * Defines a converter from a level to a number of points, i. e. an abstract base class for level-numPoints mappings.
 * AbstractGrowthStrategy-Objects are used in some subclasses of AbstractPointOrdering.
 */
class AbstractGrowthStrategy {
public:
	virtual ~AbstractGrowthStrategy();

	virtual size_t numPoints(size_t level) = 0;
};

} /* namespace combigrid */
} /* namespace SGPP */

#endif /* COMBIGRID_SRC_SGPP_COMBIGRID_GRID_POINTS_ABSTRACTGROWTHSTRATEGY_HPP_ */
