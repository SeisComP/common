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


#ifndef SEISCOMP_PICKER_STALTA_H
#define SEISCOMP_PICKER_STALTA_H


#include <vector>
#include <seiscomp/math/filter.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


// Recursive STA/LTA filter
//
// Very simple and doesn't need a lot of memory for buffering.

template<typename TYPE>
class STALTA : public InPlaceFilter<TYPE> {
	public:
		STALTA(double lenSTA=2, double lenLTA=50, double fsamp=1.);

	public:
		// Apply the picker in place to the (previously filtered) data.
		void apply(int ndata, TYPE *data) override;

		void reset();

		// Set the sampling frequency in Hz. Allows delayed
		// initialization when the data arrive.
		void setSamplingFrequency(double fsamp) override;

		int setParameters(int n, const double *params) override;

		InPlaceFilter<TYPE> *clone() const override;

	protected:
		// length of STA and LTA windows in seconds
		double _lenSTA, _lenLTA;

		// length of STA and LTA windows in samples
		int _numSTA, _numLTA;

		// number of samples processed since init/reset
		int _sampleCount;

		// The *current* and continuously updated STA and LTA values
		double _STA, _LTA; // must be double

		// sampling frequency in Hz
		double _fsamp;
};




// Another recursive STA/LTA filter
//
// In this version the LTA is not updated during an event,
// making it more sensitive to secondary signals.

template<typename TYPE>
class STALTA2 : public InPlaceFilter<TYPE> {
	public:
		STALTA2(double lenSTA=2, double lenLTA=50,
			double eventON=3., double eventOFF=1.,
			double fsamp=1.);

	public:
		// Apply the picker in place to the (previously filtered) data.
		void apply(int ndata, TYPE *data) override;

		void reset();

		// Set the sampling frequency in Hz. Allows delayed
		// initialization when the data arrive.
		void setSamplingFrequency(double fsamp) override;

		int setParameters(int n, const double *params) override;

		InPlaceFilter<TYPE> *clone() const override;

	protected:
		// length of STA and LTA windows in seconds
		double _lenSTA, _lenLTA;

		// length of STA and LTA windows in samples
		int _numSTA, _numLTA;

		// number of samples processed since init/reset
		int _sampleCount;

		// The *current* and continuously updated STA and LTA values
		double _STA, _LTA; // must be double

		// sampling frequency in Hz
		double _fsamp;

		// thresholds to declare an event on and off
		double _eventOn, _eventOff;

		// This value controls how much the LTA will be updated during
		// an event. Current values are only 0 and 1. With 0 meaning
		// completely frozen LTA during the event, 1 meaning the LTA
		// is fully updated.
		double _updateLTA;
};


// Classical, non-recursive STA/LTA filter
//
// As simple as it can get, but also needs a lot more memory.

template<typename TYPE>
class STALTA_Classic : public InPlaceFilter<TYPE> {
	public:
		STALTA_Classic(
			double lenSTA =  2.,
			double lenLTA = 50.,
			double fsamp  =  1.);

	public:
		// Apply the picker in place to the (previously filtered) data.
		void apply(int ndata, TYPE *data) override;

		void reset();

		// Set the sampling frequency in Hz. Allows delayed
		// initialization when the data arrive.
		void setSamplingFrequency(double fsamp) override;

		int setParameters(int n, const double *params) override;

		InPlaceFilter<TYPE> *clone() const override;

	protected:
		// length of STA and LTA windows in seconds
		double _lenSTA, _lenLTA;

		// length of STA and LTA windows in samples
		int _numSTA, _numLTA;

		// number of samples processed since init/reset
		int _sampleCount;

		// The *current* and continuously updated STA and LTA values
		double _STA, _LTA; // must be double

		// For this non-recursive filter we need to save all samples
		// that contribute to the STA and LTA. Note that the space
		// requirement is not optimized because the STA buffer is
		// redundant. This is for simplicity and because the STA
		// buffer length is small compared to the LTA buffer length.
		std::vector<double> _sta_buffer, _lta_buffer;

		// sampling frequency in Hz
		double _fsamp;
};


} // namespace Seiscomp::Math::Filter
} // namespace Seiscomp::Math
} // namespace Seiscomp


#endif
