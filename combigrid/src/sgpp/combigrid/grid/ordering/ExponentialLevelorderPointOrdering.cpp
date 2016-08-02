/*
 * ExponentialLevelorderPointOrdering.cpp
 *
 *  Created on: 19.12.2015
 *      Author: david
 */

#include "ExponentialLevelorderPointOrdering.hpp"
#include "ExponentialLevelorderPermutationIterator.hpp"

namespace SGPP {
namespace combigrid {

ExponentialLevelorderPointOrdering::~ExponentialLevelorderPointOrdering() {
}

size_t ExponentialLevelorderPointOrdering::convertIndex(size_t level, size_t numPoints, size_t index) {
	size_t lastIndex = numPoints - 1;

	if(level == 0) {
		return 0;
	}

	if(index == 0) {
		return lastIndex/2;
	}
	if(index == 1) {
		return 0;
	}
	if(index == 2) {
		return lastIndex;
	}

	size_t dividedIndex = index - 1;
	size_t resultingLevel = 1;

	// after having subtracted 1, the indices 3, 4 have to be divided once; the indices 5, 6, 7, 8 have to be divided twice, etc.
	while(dividedIndex != 1) {
		dividedIndex /= 2;
		++resultingLevel;
	}

	size_t levelHalfPointDistance = (1L << (level - resultingLevel));
	size_t indexInLevel = index - ((1L << (resultingLevel - 1)) + 1);

	return levelHalfPointDistance + 2 * levelHalfPointDistance * indexInLevel;
}

size_t ExponentialLevelorderPointOrdering::numPoints(size_t level) {
	if(level == 0) {
		return 1;
	}

	return (static_cast<size_t>(1) << level) + 1;
}

std::shared_ptr<AbstractPermutationIterator> ExponentialLevelorderPointOrdering::getSortedPermutationIterator(size_t level,
		const std::vector<SGPP::float_t>& points, size_t numPoints) {
	return std::shared_ptr<AbstractPermutationIterator>(new ExponentialLevelorderPermutationIterator(level, numPoints));
}

} /* namespace combigrid */
} /* namespace SGPP */
