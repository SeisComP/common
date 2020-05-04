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


#include <math.h>
#include "mBc_measure.h"


Measurement::Measurement(std::size_t nsamp)
: nsamp(nsamp) {
	offset = 0;
	processed = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Measurement::~Measurement() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Measurement_mBc::Measurement_mBc(std::size_t nsamp, double q)
: Measurement(nsamp)
, _q(q) {
	imax = 0;
	vcum = vmax = _vpeak = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Measurement_mBc::feed(std::size_t n, const double *v) {
	if ( processed == 0 && n > 0 )
		// deferred initialization until the first sample
		_previous = (v[0] > 0);

	// Start loop at the 1st unprocessed sample
	for ( std::size_t i = 0; i < n; ++i ) {
		double vi = v[i]-offset;  // demeaned data sample
		// Any zero crossing ?
		if ( (vi > 0) != _previous ) {
			// Does the current peak exceed the threshold?
			if ( _vpeak > _q * vmax ) {
				// save the position of that peak
				_subeventIndices.push_back(_ipeak);
				_subeventValues.push_back(_vpeak);
				// update cumulative amplitude
				vcum += _vpeak;
				icum  = _ipeak;
			}
			// Is the current peak a new maximum?
			if ( _vpeak > vmax ) {
				vmax = _vpeak;
				imax = _ipeak;
			}

			// Update the sign
			_previous = (vi > 0);

			// Start a new peak measurement by resetting the peak amplitude
			_vpeak = 0;
		}

		// We're within a peak -> update amplitude if necessary.
		vi = fabs(vi);
		if ( vi > _vpeak ) {
			_vpeak = vi;
			_ipeak = processed + i;
		}
	}

	// keep track of processed samples
	processed += n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::size_t Measurement_mBc::subeventCount() const {
	return _subeventIndices.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::size_t Measurement_mBc::subeventIndex(std::size_t i) const {
	return _subeventIndices[i];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Measurement_mBc::subeventValue(std::size_t i) const {
	return _subeventValues[i];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
