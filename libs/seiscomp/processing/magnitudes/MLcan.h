/* ################################################################################
* #    Copyright (C) 2024 by IGN Spain                                           #
* #                                                                              #
* #    author: J. Barco, E. Suarez                                               #
* #    email:  jbarco@transportes.gob.es   ,  eadiaz@transportes.gob.es          #
* #    last modified: 2024-03-20                 			   					 #
* #    derived from Mezcua & Rueda (2021)                                        #
* #    https://doi.org/10.1007/s00445-022-01553-9                                #
* #                                                                              #
* #    This program is free software; you can redistribute it and/or modify      #
* #    it under the terms of the GNU General Public License as published by      #
* #    the Free Software Foundation; either version 2 of the License, or         #
* #    (at your option) any later version.                                       #
* #                                                                              #
* #    This program is distributed in the hope that it will be useful,           #
* #    but WITHOUT ANY WARRANTY; without even the implied warranty of            #
* #    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             #
* #    GNU General Public License for more details.                              #
* #                                                                              #
* #    You should have received a copy of the GNU General Public License         #
* #    along with this program; if not, write to the                             #
* #    Free Software Foundation, Inc.,                                           #
* #    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 #
* ################################################################################ */


#ifndef SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_MLcan_H
#define SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_MLcan_H


#include <seiscomp/processing/magnitudeprocessor.h>
#include <seiscomp/processing/magnitudes/utils.h>
#include <seiscomp/math/geo.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API MagnitudeProcessor_MLcan : public MagnitudeProcessor {
	public:
		MagnitudeProcessor_MLcan();


	public:
		bool setup(const Settings &settings) override;

		std::string amplitudeType() const override;


	protected:
		Status computeMagnitude(double amplitude, const std::string &unit,
		                        double period, double snr,
		                        double delta, double depth,
		                        const DataModel::Origin *hypocenter,
		                        const DataModel::SensorLocation *receiver,
		                        const DataModel::Amplitude *,
		                        const Locale *,
		                        double &value) override;

	protected:
		bool initLocale(Locale *locale, const Settings &settings,
		                const std::string &configPrefix) override;

	private:
		double getChanCorr(const double c0_first, const double c0_second, std::string ChannelCode);
		double      _minDistanceKm{-1.0};
		double      _maxDistanceKm{8.0 * KM_OF_DEGREE};
		double      _maxDepth{80.0};
		std::string _distanceMode{"hypocentral"};
		std::string _calibrationType{"parametric"};
		// parameters for parametric magnitude calibration
		double      _c0_first{0.0};
		double		_c0_second{0.0};
		double      _c1{2.4449};
		double      _c2{0.00142};
		double      _c3{0.96657};
		double      _c4{-40};
		double      _c5{40};
		// parameters for non-parametric magnitude calibration
		LogA0  _logA0;
};


}
}


#endif
