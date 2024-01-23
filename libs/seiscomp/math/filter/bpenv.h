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


#ifndef SEISCOMP_FILTERING_BPENV_H
#define SEISCOMP_FILTERING_BPENV_H


#include <seiscomp/math/filter.h>
#include <seiscomp/math/filter/butterworth.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {

// Band pass and envelope filter
//
// This is a recursive band pass and envelope filter combination. Strictly
// speaking the envelope would require frequency-domain filtering because it
// requires the Hilbert transform of the signal. We cannot compute the Hilbert
// transform using time-domain recursive filters. By applying a rather narrow
// band pass to the data, however, we can compute an approximate Hilbert
// transform. This is because for a narrow-band signal the Hilbert transform
// will be the scaled time derivative. This actually works quite well in
// practice as long as the band pass is not too wide and has relatively steep
// corners to block energy from outside the passband. A band width of one
// octave and a filter order of four was found to work well in most cases.
// It is therefore highly recommended to stick to these defaults and only set
// the center frequency.
//
// This filter may be used in a filter chain. Its name is BPENV and it accepts
// up to three arguments:
//   - center frequency in Hz (must be specified)
//   - bandwidth in octaves (default: 1)
//   - filter order (default: 4)
//
// Author:
//    Joachim Saul, GFZ Potsdam
//    saul@gfz-potsdam.de


template <typename T>
class BandPassEnvelope : public InPlaceFilter<T> {
	public:
		BandPassEnvelope(double centerFrequency=1, double bandwidthOctaves=1, int order=4, double fsamp=0);

		// Apply filter to data vector in place
		void apply(int ndata, T *data) override;

		// Apply a full reset
		void reset();

		// Set the sampling frequency in Hz. Allows delayed
		// initialization when the data arrive.
		void setSamplingFrequency(double fsamp) override;

		int setParameters(int n, const double *params) override;

		InPlaceFilter<T> *clone() const override;


	private:
		double _centerFrequency;
		double _bandwidthOctaves;
		int    _order;
		double _samplingFrequency;
		double _K, _yp;

		using FilterPtr = typename Core::SmartPointer<Math::Filtering::InPlaceFilter<T>>::Impl;
		FilterPtr _bp;

		// Flag to indicate some pending initialization
		bool _afterReset;
};


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp


#endif
