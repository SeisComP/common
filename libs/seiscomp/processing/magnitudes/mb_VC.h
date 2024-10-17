/* ################################################################################
* #    Copyright (C) 2024 by IGN Spain                                           #
* #                                                                              #
* #    author: J. Barco, E. Suarez                                               #
* #    email:  jbarco@transportes.gob.es   ,  eadiaz@transportes.gob.es          #
* #    last modified: 2024-03-20                                                 #
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


#ifndef SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_M_B_VC_H
#define SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_M_B_VC_H


#include <seiscomp//processing/magnitudeprocessor.h>


namespace Seiscomp {
namespace Processing {


class SC_SYSTEM_CLIENT_API MagnitudeProcessor_mB_VC : public MagnitudeProcessor {
	DECLARE_SC_CLASS(MagnitudeProcessor_mB_VC)

	public:
		MagnitudeProcessor_mB_VC();
		MagnitudeProcessor_mB_VC(const std::string& type);


		bool setup(const Settings &settings) override;

		Status computeMagnitude(double amplitude, const std::string &unit,
		                        double period, double snr,
		                        double delta, double depth,
		                        const DataModel::Origin *hypocenter,
		                        const DataModel::SensorLocation *receiver,
		                        const DataModel::Amplitude *,
		                        const Locale *,
		                        double &value) override;

		Status estimateMw(const Config::Config *config,
		                  double magnitude, double &Mw_estimate,
		                  double &Mw_stdError) override;

	private:
		double                  minDistanceDeg;
		double                  maxDistanceDeg;
};


}
}


#endif
