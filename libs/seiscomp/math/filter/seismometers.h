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


#ifndef SEISCOMP_SEISMOMETERS_H
#define SEISCOMP_SEISMOMETERS_H


#include <complex>
#include <seiscomp/math/filter/biquad.h>


namespace Seiscomp {
namespace Math {


enum GroundMotion { Displacement, Velocity, Acceleration };

namespace SeismometerResponse {

typedef std::complex<double> Pole;
typedef std::complex<double> Zero;

struct FAP {
	FAP() {}
	FAP(double f, double a, double p)
	: frequency(f), amplitude(a), phaseAngle(p) {}

	bool operator<(const FAP &other) const {
		return frequency < other.frequency;
	}

	double frequency;  //! Frequency in Hz
	double amplitude;  //! Amplitude
	double phaseAngle; //! Phase angle in degree
};

typedef std::vector<Pole> Poles;
typedef std::vector<Zero> Zeros;
typedef std::vector<FAP> FAPs;

class PolesAndZeros {
	public:
		PolesAndZeros();

		PolesAndZeros(const Poles &poles, const Zeros &zeros,
		              double norm);

		PolesAndZeros(const PolesAndZeros &other);

	public:
		Poles  poles;
		Zeros  zeros;
		double norm;
};

class WoodAnderson : public PolesAndZeros {
	public:
		// Gutenberg(1935)              -> gain=2800, T0=0.8s, h=0.8
		// Uhrhammer and Collins (1990) -> gain=2080, T0=0.8s, h=0.7
		//
		// Note that use of the Uhrhammer and Collins (1990) version
		// is recommended by the IASPEI Magnitude Working Group
		// recommendations of 2011 September 9.
		struct Config {
			Config(double gain=2800, double T0=0.8, double h=0.8) :
				gain(gain), T0(T0), h(h) {}

			double gain, T0, h;
		};
		WoodAnderson(GroundMotion input, Config config=Config());
};


class Seismometer5sec : public PolesAndZeros {
public:
	Seismometer5sec(GroundMotion input);
};


}


namespace Filtering {


// Responses that soon need to be implemented are (at least)
// * STS2
// * STS1
// * Kirnos
// * corresponding restitution filters
// * generic filters that read PAZ from file/database/etc.

namespace IIR {

template <typename T>
class Filter : public SeismometerResponse::PolesAndZeros,
               public Math::Filtering::InPlaceFilter<T> {
	public:
		Filter();

		Filter(const SeismometerResponse::Poles &poles,
		       const SeismometerResponse::Zeros &zeros, double norm);

		Filter(const Filter &other);

	public:
		void setSamplingFrequency(double fsamp) override;
		int setParameters(int n, const double *params) override;

		void apply(int n, T *inout) override;
	
		Math::Filtering::InPlaceFilter<T> *clone() const override;

	private:
		Math::Filtering::IIR::BiquadCascade<T> _cascade;
};


template <typename T>
class WWSSN_SP_Filter : public Filter<T> {
	public:
		WWSSN_SP_Filter(GroundMotion input=Velocity);
		WWSSN_SP_Filter(const WWSSN_SP_Filter &other);

	public:
		int setParameters(int n, const double *params) override;
		Math::Filtering::InPlaceFilter<T> *clone() const override;

		void setInput(GroundMotion input);
};


template <typename T>
class WWSSN_LP_Filter : public Filter<T> {
	public:
		WWSSN_LP_Filter(GroundMotion input=Velocity);
		WWSSN_LP_Filter(const WWSSN_LP_Filter &other);

	public:
		int setParameters(int n, const double *params) override;
		Math::Filtering::InPlaceFilter<T> *clone() const override;

		void setInput(GroundMotion input);
};


template <typename T>
class WoodAndersonFilter : public Filter<T> {
	public:
		WoodAndersonFilter(GroundMotion input=Velocity, SeismometerResponse::WoodAnderson::Config config = SeismometerResponse::WoodAnderson::Config());
		WoodAndersonFilter(const WoodAndersonFilter &other);

	public:
		int setParameters(int n, const double *params) override;
		Math::Filtering::InPlaceFilter<T> *clone() const override;

		void setInput(GroundMotion input,
		              SeismometerResponse::WoodAnderson::Config config = SeismometerResponse::WoodAnderson::Config());
};


template <typename T>
class GenericSeismometer : public Filter<T> {
	public:
		GenericSeismometer(double cornerPeriod=1.,
		                   GroundMotion input=Velocity);
		GenericSeismometer(const GenericSeismometer &other);

	public:
		int setParameters(int n, const double *params) override;
		Math::Filtering::InPlaceFilter<T> *clone() const override;

		void setInput(GroundMotion input);

	private:
		double _cornerPeriod;
};

template <typename T>
class Seismometer5secFilter : public Filter<T> {
	public:
		Seismometer5secFilter(GroundMotion input=Velocity);
		Seismometer5secFilter(const Seismometer5secFilter &other);

	public:
		int setParameters(int n, const double *params) override;
		Math::Filtering::InPlaceFilter<T> *clone() const override;

		void setInput(GroundMotion input);
};


}


} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp


#endif
