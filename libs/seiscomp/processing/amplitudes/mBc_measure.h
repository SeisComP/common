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


#include <vector>


class Measurement {
	public:
		Measurement(std::size_t nsamp);
		virtual ~Measurement();

	public:
		// offset of the data samples
		// must be measured prior to the measurement
		void setOffset(double x) { offset = x; }

		// Feed n broadband ground velocity values v.
		// feed() calls update() automatically!
		virtual void feed(std::size_t n, const double *v) = 0;

		double progress() const { return nsamp>0 ? double(processed)/nsamp : 0; }

	protected:
		// an offset to be subtracted before the actual measurement
		double offset;

		// number of samples anticipated to contribute to measurement
		std::size_t nsamp;

		// number of samples already processed
		std::size_t processed;
};


class Measurement_mBc : public Measurement {
	public:
		Measurement_mBc(std::size_t nsamp, double q=0.6);

	public:
		void feed(std::size_t n, const double *v) override;

	public:
		// Accessors to subevents. mBc specific and meant for debugging, e.g.
		// for plotting the individual subevents contributing to the sum.
		//
		// "Subevents" are individual extrema that contribute to the sum.
		std::size_t subeventCount() const;
		std::size_t subeventIndex(std::size_t i) const;
		double subeventValue(std::size_t i) const;

	public:
		double      vcum; // cumulative velocity
		double      vmax; // zero-to-peak maximum velocity
		std::size_t icum; // index at which vcum was observed ( = index of last subevent)
		std::size_t imax; // index at which vmax was observed

	private:
		// Cutoff threshold, normally fixed to 0.6, don't change!
		// See also Bormann & Saul (2009)
		double _q;

		std::vector<std::size_t> _subeventIndices;
		std::vector<double>      _subeventValues;

		int _previous;
		std::size_t _ipeak;
		double _vpeak;
};
