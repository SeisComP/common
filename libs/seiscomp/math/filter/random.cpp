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


#define SEISCOMP_COMPONENT Random


#include <seiscomp/logging/log.h>
#include <seiscomp/core/exceptions.h>
#include <seiscomp/math/filter/random.h>

namespace Seiscomp {
namespace Math {
namespace Filtering {


template<typename TYPE>
RandomUniform<TYPE>::RandomUniform(double minValue /*counts*/, double maxValue /*counts*/)
: _minimumValue(minValue), _maximumValue(maxValue) {}


template<typename TYPE>
void RandomUniform<TYPE>::apply(int n, TYPE *inout) {
	for ( int i = 0; i < n; ++i ) {
		inout[i] = static_cast<TYPE>(static_cast<double>(rand())/static_cast<double>(RAND_MAX) * (_maximumValue - _minimumValue) + _minimumValue);
	}
}


template<typename TYPE>
InPlaceFilter<TYPE>* RandomUniform<TYPE>::clone() const {
	return new RandomUniform<TYPE>(_minimumValue, _maximumValue);
}


template<typename TYPE>
void RandomUniform<TYPE>::setLength(double timeSpan) {}


template<typename TYPE>
void RandomUniform<TYPE>::setSamplingFrequency(double fsamp) {}


template<typename TYPE>
int RandomUniform<TYPE>::setParameters(int n, const double *params) {
	if ( n != 2 ) {
		return 2;
	}

	_minimumValue = params[0];
	_maximumValue = params[1];

	return n;
}


template<typename TYPE>
void RandomUniform<TYPE>::reset() {}


template<typename TYPE>
RandomNormal<TYPE>::RandomNormal(double meanValue /*counts*/, double stdDev /*counts*/)
: _meanValue(meanValue), _standardDev(stdDev) {}


template<typename TYPE>
void RandomNormal<TYPE>::apply(int n, TYPE *inout) {
	// random device class instance, source of 'true' randomness for initializing random seed
	std::random_device rd;
	// Mersenne twister PRNG, initialized with seed from previous random device instance
	std::mt19937_64 gen(rd());

	for ( int i = 0; i < n; ++i ) {
		std::normal_distribution<double> d(_meanValue, _standardDev);
		//inout[i] = distribution(generator);
		inout[i] = d(gen);
	}
}


template<typename TYPE>
InPlaceFilter<TYPE>* RandomNormal<TYPE>::clone() const {
	return new RandomNormal<TYPE>(_meanValue, _standardDev);
}


template<typename TYPE>
void RandomNormal<TYPE>::setLength(double timeSpan) {}


template<typename TYPE>
void RandomNormal<TYPE>::setSamplingFrequency(double fsamp) {}


template<typename TYPE>
int RandomNormal<TYPE>::setParameters(int n, const double *params) {
	if ( n != 2 ) {
		return 2;
	}

	_meanValue = params[0];
	_standardDev = params[1];

	return n;
}


template<typename TYPE>
void RandomNormal<TYPE>::reset() {}


// uniform distribution
INSTANTIATE_INPLACE_FILTER(RandomUniform, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(RandomUniform, "RUD");
// Gaussian normal distribution
INSTANTIATE_INPLACE_FILTER(RandomNormal, SC_SYSTEM_CORE_API);
REGISTER_INPLACE_FILTER(RandomNormal, "RND");


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp
