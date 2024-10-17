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



#include <seiscomp/processing/magnitudes/mb_VC.h>
#include <seiscomp/seismology/magnitudes.h>
#include <math.h>

namespace Seiscomp {
namespace Processing {


namespace {

std::string ExpectedAmplitudeUnit = "nm/s";

}


IMPLEMENT_SC_CLASS_DERIVED(MagnitudeProcessor_mB_VC, MagnitudeProcessor, "MagnitudeProcessor_mB_VC");
REGISTER_MAGNITUDEPROCESSOR(MagnitudeProcessor_mB_VC, "mB_VC");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeProcessor_mB_VC::MagnitudeProcessor_mB_VC::setup(const Settings &settings) {
	if ( !MagnitudeProcessor::setup(settings) )
		return false;

	minDistanceDeg = 5.0; // default minimum distance
	maxDistanceDeg = 105.0; // default maximum distance

	// distance range in degree
	try { minDistanceDeg = settings.getDouble("magnitudes." + type() + ".minDist"); }
	catch ( ... ) {}

	try { maxDistanceDeg = settings.getDouble("magnitudes." + type() + ".maxDist"); }
	catch ( ... ) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_mB_VC::MagnitudeProcessor_mB_VC()
 : MagnitudeProcessor("mB_VC") {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor_mB_VC::MagnitudeProcessor_mB_VC(const std::string& type)
 : MagnitudeProcessor(type) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mB_VC::computeMagnitude(
        double amplitude, const std::string &unit,
        double, double,
        double delta, double depth,
        const DataModel::Origin *, const DataModel::SensorLocation *,
        const DataModel::Amplitude *, const Locale *, double &value) {
	// Clip depth to 0
	if ( depth < 0 ) depth = 0;

	if ( delta < minDistanceDeg || delta > maxDistanceDeg )
		return DistanceOutOfRange;

	if ( amplitude <= 0 )
		return AmplitudeOutOfRange;

	if ( !convertAmplitude(amplitude, unit, ExpectedAmplitudeUnit) )
		return InvalidAmplitudeUnit;

	bool status = Magnitudes::compute_mb_VC(amplitude*1.E-3, 2*M_PI, delta, depth+1, &value);
	value -= 0.14; // HACK until we have an optimal calibration function
	return status ? OK : Error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeProcessor::Status MagnitudeProcessor_mB_VC::estimateMw(
	const Config::Config *,
	double magnitude,
	double &Mw_estimate,
	double &Mw_stdError)
{
    // Relationships Between Mw and Other Earthquake Size Parameters in the Spanish IGN Seismic
    // Catalog
	// Cabanas et al. (2015) 
    // DOI 10.1007/s00024-014-1025-2
        const double a=1.213, b=-1.528;
                Mw_estimate = a * magnitude + b;

        Mw_stdError = 0.4;

	return OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
