/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/


#ifndef SEISCOMP_MATH_WINDOWFUNC_H
#define SEISCOMP_MATH_WINDOWFUNC_H


#include <seiscomp/core/baseobject.h>
#include <seiscomp/core/typedarray.h>
#include <seiscomp/core.h>

#include <vector>


namespace Seiscomp {
namespace Math {


template <typename TYPE>
class SC_SYSTEM_CORE_API WindowFunc : public Core::BaseObject {
	public:
		virtual ~WindowFunc();


	public:
		/**
		 * @brief Applies the window function to the given data
		 * @param n Number of samples
		 * @param inout The data vector where each sample is multiplied with
		 *              the respective sample of the window function.
		 * @param width The width of the window function. The default is 0.5
		 *              which means 50% at either side. 0.1 would mean that the
		 *              left half of the window function is applied on 10% of
		 *              the left portion of the data vector and the right half
		 *              of the window function is applied on the right 10% of
		 *              the data vector. The value is clipped into range [0,0.5].
		 */
		void apply(int n, TYPE *inout, double width = 0.5) const;

		/**
		 * @brief Applies the window function to the given data
		 * @param inout The data vector where each sample is multiplied with
		 *              the respective sample of the window function.
		 * @param width The width of the window function. The default is 0.5
		 *              which means 50% at either side. 0.1 would mean that the
		 *              left half of the window function is applied on 10% of
		 *              the left portion of the data vector and the right half
		 *              of the window function is applied on the right 10% of
		 *              the data vector. The value is clipped into range [0,0.5].
		 */
		void apply(std::vector<TYPE> &inout, double width = 0.5) const;

		/**
		 * @brief Applies the window function to the given data
		 * @param inout The data array where each sample is multiplied with
		 *              the respective sample of the window function.
		 * @param width The width of the window function. The default is 0.5
		 *              which means 50% at either side. 0.1 would mean that the
		 *              left half of the window function is applied on 10% of
		 *              the left portion of the data vector and the right half
		 *              of the window function is applied on the right 10% of
		 *              the data vector. The value is clipped into range [0,0.5].
		 */
		void apply(TypedArray<TYPE> &inout, double width = 0.5) const;

		/**
		 * @brief Applies the window function to the given data
		 * @param inout The data array where each sample is multiplied with
		 *              the respective sample of the window function.
		 * @param width The width of the window function. The default is 0.5
		 *              which means 50% at either side. 0.1 would mean that the
		 *              left half of the window function is applied on 10% of
		 *              the left portion of the data vector and the right half
		 *              of the window function is applied on the right 10% of
		 *              the data vector. The value is clipped into range [0,0.5].
		 */
		void apply(TypedArray<TYPE> *inout, double width = 0.5) const;

		//! Apply methods for non-symmetric window lengths.
		void apply(int n, TYPE *inout, double left, double right) const;
		void apply(std::vector<TYPE> &inout, double left, double right) const;
		void apply(TypedArray<TYPE> &inout, double left, double right) const;
		void apply(TypedArray<TYPE> *inout, double left, double right) const;


	protected:
		/**
		 * @brief Applies the window function to the given data. This method has
		 *        to be implemented by derived classes. It is called by all
		 *        apply variants.
		 * @param n Number of samples
		 * @param inout The data vector where each sample is multiplied with
		 *              the respective sample of the window function.
		 * @param left The width of the window function at the left side.
		 *             The default is 0.5 which means 50% at the left side.
		 *             0.1 would mean that the left half of the window function
		 *             is applied on 10% of the left portion of the data vector.
		 *             The value is clipped into range [0,0.5].
		 * @param right The width of the window function at the right side
		 *              respectively.
		 */
		virtual void process(int n, TYPE *inout,
		                     double left = 0.5,
		                     double right = 0.5) const = 0;


	private:
		void checkAndProcess(int n, TYPE *inout,
		                     double left = 0.5,
		                     double right = 0.5) const;
};


}
}


#endif
