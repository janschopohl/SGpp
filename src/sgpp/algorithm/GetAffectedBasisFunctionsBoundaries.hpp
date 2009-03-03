/*****************************************************************************/
/* This file is part of sgpp, a program package making use of spatially      */
/* adaptive sparse grids to solve numerical problems                         */
/*                                                                           */
/* Copyright (C) 2009 Alexander Heinecke (Alexander.Heinecke@mytum.de)       */
/*                                                                           */
/* sgpp is free software; you can redistribute it and/or modify              */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 3 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* sgpp is distributed in the hope that it will be useful,                   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with sgpp; if not, write to the Free Software                       */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/* or see <http://www.gnu.org/licenses/>.                                    */
/*****************************************************************************/

#ifndef GETAFFECTEDBASISFUNCTIONSBOUNDARIES_HPP
#define GETAFFECTEDBASISFUNCTIONSBOUNDARIES_HPP

#include "grid/GridStorage.hpp"
#include "data/DataVector.h"

#include <vector>
#include <utility>
#include <iostream>

namespace sg {

/**
 * Basic algorithm for getting all affected basis functions.
 * This implicitly assumes a tensor-product approach and local support.
 * Grid points on the border are supported.
 *
 * @todo add graphical description
 */
template<class BASIS>
class GetAffectedBasisFunctionsBoundaries
{
public:
	GetAffectedBasisFunctionsBoundaries(GridStorage* storage) : storage(storage)
	{
	}

	~GetAffectedBasisFunctionsBoundaries() {}

	/**
	 * Returns evaluations of all basis functions that are non-zero at a given evaluation point.
	 * For a given evaluation point \f$x\f$, it stores tuples (std::pair) of
	 * \f$(i,\phi_i(x))\f$ in the result vector for all basis functions that are non-zero.
	 * If one wants to evaluate \f$f_N(x)\f$, one only has to compute
	 * \f[ \sum_{r\in\mathbf{result}} \alpha[r\rightarrow\mathbf{first}] \cdot r\rightarrow\mathbf{second}. \f]
	 *
	 * @param basis a sparse grid basis
	 * @param point evaluation point within the domain
	 * @param result a vector to store the results in
	 *
	 * @todo review this seems to be wrong
	 */
	void operator()(BASIS& basis, std::vector<double>& point, std::vector<std::pair<size_t, double> >& result)
	{
		GridStorage::grid_iterator working(storage);

		typedef GridStorage::index_type::level_type level_type;
		typedef GridStorage::index_type::index_type index_type;

		size_t bits = sizeof(index_type) * 8; // how many levels can we store in a index_type?

		size_t dim = storage->dim();

		index_type* source = new index_type[dim];

		for(size_t d = 0; d < dim; ++d)
		{
			// This does not really work on grids with borders.
			double temp = floor(point[d]*(1<<(bits-2)))*2;
			if(point[d] == 1.0)
			{
				source[d] = static_cast<index_type>(temp-1);
			}
			else
			{
				source[d] = static_cast<index_type>(temp+1);
			}

		}

		result.clear();
		rec(basis, point, 0, 1.0, working, source, result);

		delete [] source;

	}

protected:
	GridStorage* storage;

	/**
	 * Recursive traversal of the "tree" of basis functions for evaluation, used in operator().
	 * For a given evaluation point \f$x\f$, it stores tuples (std::pair) of
	 * \f$(i,\phi_i(x))\f$ in the result vector for all basis functions that are non-zero.
	 *
	 * @param basis a sparse grid basis
	 * @param point evaluation point within the domain
	 * @param current_dim the dimension currently looked at (recursion parameter)
	 * @param value the value of the evaluation of the current basis function up to (excluding) dimension current_dim (product of the evaluations of the one-dimensional ones)
	 * @param working iterator working on the GridStorage of the basis
	 * @param source array of indices for each dimension (identifying the indices of the current grid point)
	 * @param result a vector to store the results in
	 *
	 * @todo review this seems to be wrong
	 */
	void rec(BASIS& basis, std::vector<double>& point, size_t current_dim, double value, GridStorage::grid_iterator& working, GridStorage::index_type::index_type* source, std::vector<std::pair<size_t, double> >& result)
	{
		typedef GridStorage::index_type::level_type level_type;
		typedef GridStorage::index_type::index_type index_type;

		size_t i;

		// @TODO: Remove 'magic' number
		level_type src_level = static_cast<level_type>(sizeof(index_type) * 8 - 1);
		index_type src_index = source[current_dim];

		level_type work_level = 0;

		while(true)
		{
			size_t seq = working.seq();
			if(storage->end(seq))
			{
				//if (work_level == 1)
				//std::cout << "work_level: " << work_level << std::endl;

				break;
			}
			else
			{
				// handle boundaries if we are on level 1
				if (work_level == 0)
				{
					// level 0, index 0
					working.left_levelzero(current_dim);
					size_t seq_lz_left = working.seq();
					double new_value_l_zero_left = basis.eval(0, 0, point[current_dim]);

					if(current_dim == storage->dim()-1)
					{
						result.push_back(std::make_pair(seq_lz_left, value*new_value_l_zero_left));
					}
					else
					{
						rec(basis, point, current_dim + 1, value*new_value_l_zero_left, working, source, result);
					}

					// level 0, index 1
					working.right_levelzero(current_dim);
					size_t seq_lz_right = working.seq();
					double new_value_l_zero_right = basis.eval(0, 1, point[current_dim]);

					if(current_dim == storage->dim()-1)
					{
						result.push_back(std::make_pair(seq_lz_right, value*new_value_l_zero_right));
					}
					else
					{
						rec(basis, point, current_dim + 1, value*new_value_l_zero_right, working, source, result);
					}

					//working.top(current_dim);
					//seq = working.seq();
				}
				else
				{
					index_type work_index;
					level_type temp;

					working.get(current_dim, temp, work_index);

					double new_value = basis.eval(work_level, work_index, point[current_dim]);

					if(current_dim == storage->dim()-1)
					{
						result.push_back(std::make_pair(seq, value*new_value));
					}
					else
					{
						rec(basis, point, current_dim + 1, value*new_value, working, source, result);
					}
				}
			}

			// there are no levels left
			if(working.hint(current_dim))
			{
				break;
			}

			// this decides in which direction we should descend by evaluating the corresponding bit
			// the bits are coded from left to right starting with level 1 being in position src_level
			if (work_level == 0)
			{
				working.top(current_dim);
			}
			else
			{
				bool right = (src_index & (1 << (src_level - work_level))) > 0;

				if(right)
				{
					working.right_child(current_dim);
				}
				else
				{
					working.left_child(current_dim);
				}
			}
			++work_level;
		}

		working.top(current_dim);
	}

};

}

#endif /* GETAFFECTEDBASISFUNCTIONSBOUNDARIES_HPP */
