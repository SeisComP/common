/* ################################################################################
* #    Copyright (C) 2024 by IGN Spain                                           #
* #                                                                              #
* #    author: J. Barco, E. Suarez                                               #
* #    email:  jbarco@transportes.gob.es   ,  eadiaz@transportes.gob.es          #
* #    last modified: 2024-03-20                  			   					 #
* #    based on Lopez, C. (2008). Nuevas formulas de magnitud para la            #
* #	   Peninsula Iberica y su entorno. Trabajo de investigación del              #
* #	   Master en Geofisica y Meteorologia. Departamento de Física de la Tierra,  #
* #	   Astronomia y Astrofisica. Universidad Complutense de Madrid. Madrid.      #
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


#ifndef SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_mb_Lg_H
#define SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_mb_Lg_H


#include <seiscomp/processing/magnitudeprocessor.h>
#include <seiscomp/processing/magnitudes/utils.h>
#include <seiscomp/math/geo.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API MagnitudeProcessor_mb_Lg : public MagnitudeProcessor {
	public:
		MagnitudeProcessor_mb_Lg();


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
		double      _minDistanceKm{-1.0};
		double      _maxDistanceKm{8.0 * KM_OF_DEGREE};
		double      _maxDepth{80.0};
		std::string _distanceMode{"hypocentral"};
		std::string _calibrationType{"parametric"};
		// parameters for parametric magnitude calibration
		double      _c0{1.17};
		double      _c1{0.0012};
		double      _c2{0.67};
		// parameters for non-parametric magnitude calibration
		LogA0  _logA0;
};


}
}


#endif
