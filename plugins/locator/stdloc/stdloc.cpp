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

#define SEISCOMP_COMPONENT StdLoc

#include <seiscomp/core.h>
#include <seiscomp/core/plugin.h>
#include <seiscomp/core/system.h>
#include <seiscomp/datamodel/arrival.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/geo/coordinate.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/math/math.h>
#include <seiscomp/math/mean.h>
#include <seiscomp/seismology/locatorinterface.h>
#include <seiscomp/seismology/ttt.h>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <array>

#include "solver.h"
#include "stdloc.h"

#include "eigv.h"
#include "chi2.h"

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;
using namespace LeastSquares;

using IDList = Seiscomp::Seismology::LocatorInterface::IDList;
using LocatorException = Seiscomp::Seismology::LocatorException;
using StationNotFoundException = Seiscomp::Seismology::StationNotFoundException;
using PickNotFoundException = Seiscomp::Seismology::PickNotFoundException;


namespace { // Utility functions 


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
vector<string> splitString(const string &str, const string &split = ",") {
	vector<string> tokens;
	Core::split(tokens, str, split.c_str(), false);
	return tokens;
}


double computeMean(const vector<double> &values) {
	if ( values.size() == 0 ) {
		return 0;
	}

	return accumulate(values.begin(), values.end(), 0.0) / values.size();
}


double computeCircularMean(const vector<double> &angles, bool useRadiant) {
	double y = 0, x = 0;
	for ( size_t i = 0; i < angles.size(); ++i ) {
		double angle = useRadiant ? angles[i] : deg2rad(angles[i]);
		x += cos(angle);
		y += sin(angle);
	}
	double mean = atan2(y, x);
	return useRadiant ? mean : rad2deg(mean);
}


double computedWeightedCircularMean(const vector<double> &angles,
                                    const vector<double> &weights,
                                    bool useRadiant) {
	double y = 0, x = 0;
	for ( size_t i = 0; i < angles.size(); ++i ) {
		double angle = useRadiant ? angles[i] : deg2rad(angles[i]);
		x += cos(angle) * weights[i];
		y += sin(angle) * weights[i];
	}
	double mean = atan2(y, x);
	return useRadiant ? mean : rad2deg(mean);
}


double computeMedian(const vector<double> &values) {
	if ( values.size() == 0 ) {
		return 0;
	}

	vector<double> tmp(values);
	const auto middleItr = tmp.begin() + tmp.size() / 2;
	nth_element(tmp.begin(), middleItr, tmp.end());
	double median = *middleItr;
	if (tmp.size() % 2 == 0) {
		const auto leftMiddleItr = max_element(tmp.begin(), middleItr);
		median = (*leftMiddleItr + *middleItr) / 2;
	}
	return median;
}


void computeCoordinates(double distance, double azimuth, double clat,
                        double clon, double &lat, double &lon) {
	Math::Geo::delandaz2coord(Math::Geo::km2deg(distance), azimuth,
	                          clat, clon, &lat, &lon);
	lon = Geo::GeoCoordinate::normalizeLon(lon);
}


double computeDistance(double lat1, double lon1, double lat2, double lon2,
                       double *azimuth = nullptr,
                       double *backAzimuth = nullptr) {
	double dist, az, baz;
	Math::Geo::delazi(lat1, lon1, lat2, lon2, &dist, &az, &baz);

	if (azimuth)
		*azimuth = az;
	if (backAzimuth)
		*backAzimuth = baz;

	return dist;
}


double computePickWeight(DataModel::Pick *pick,
                         const vector<double>& uncertaintyClasses) {

	double uncertainty = -1; // secs
	try {
		// symmetric uncertainty
		uncertainty = pick->time().uncertainty();
	} catch ( Core::ValueException & ) {
		// asymmetric uncertainty
		try {
			uncertainty =
					(pick->time().lowerUncertainty() + pick->time().upperUncertainty()) /
					2.0;
		} catch ( Core::ValueException & ) {
		}
	}

	// set lowest class as default
	unsigned uncertaintyClass = uncertaintyClasses.size()-1;

	if (uncertainty >= 0 && isfinite(uncertainty) && 
			uncertaintyClasses.size() > 1 &&
			uncertainty < uncertaintyClasses.back() ) {

		for ( unsigned curr = 0, next = 1; 
		      next < uncertaintyClasses.size(); curr++, next++ ) {
			if (uncertainty >= uncertaintyClasses.at(curr) && 
			    uncertainty <= uncertaintyClasses.at(next)) {
				uncertaintyClass = curr;
				break;
			}
		}
	}

	return 1 / pow(2, uncertaintyClass);
}


bool invertMatrix4x4(const std::array<std::array<double,4>,4> &in, 
                     std::array<std::array<double,4>,4> &out) {
	//
	// generated using github.com/willnode/N-Matrix-Programmer
	// and then refactored
	//
	double A2323 = in[2][2] * in[3][3] - in[2][3] * in[3][2];
	double A1323 = in[2][1] * in[3][3] - in[2][3] * in[3][1];
	double A1223 = in[2][1] * in[3][2] - in[2][2] * in[3][1];
	double A0323 = in[2][0] * in[3][3] - in[2][3] * in[3][0];
	double A0223 = in[2][0] * in[3][2] - in[2][2] * in[3][0];
	double A0123 = in[2][0] * in[3][1] - in[2][1] * in[3][0];
	double A2313 = in[1][2] * in[3][3] - in[1][3] * in[3][2];
	double A1313 = in[1][1] * in[3][3] - in[1][3] * in[3][1];
	double A1213 = in[1][1] * in[3][2] - in[1][2] * in[3][1];
	double A2312 = in[1][2] * in[2][3] - in[1][3] * in[2][2];
	double A1312 = in[1][1] * in[2][3] - in[1][3] * in[2][1];
	double A1212 = in[1][1] * in[2][2] - in[1][2] * in[2][1];
	double A0313 = in[1][0] * in[3][3] - in[1][3] * in[3][0];
	double A0213 = in[1][0] * in[3][2] - in[1][2] * in[3][0];
	double A0312 = in[1][0] * in[2][3] - in[1][3] * in[2][0];
	double A0212 = in[1][0] * in[2][2] - in[1][2] * in[2][0];
	double A0113 = in[1][0] * in[3][1] - in[1][1] * in[3][0];
	double A0112 = in[1][0] * in[2][1] - in[1][1] * in[2][0];

	double det =
	  in[0][0] * (in[1][1] * A2323 - in[1][2] * A1323 + in[1][3] * A1223) -
	  in[0][1] * (in[1][0] * A2323 - in[1][2] * A0323 + in[1][3] * A0223) +
	  in[0][2] * (in[1][0] * A1323 - in[1][1] * A0323 + in[1][3] * A0123) -
	  in[0][3] * (in[1][0] * A1223 - in[1][1] * A0223 + in[1][2] * A0123);

	if (det == 0) {
		return false;
	}

	det = 1.0 / det;

	out[0][0] = det *   ( in[1][1] * A2323 - in[1][2] * A1323 + in[1][3] * A1223 );
	out[0][1] = det * - ( in[0][1] * A2323 - in[0][2] * A1323 + in[0][3] * A1223 );
	out[0][2] = det *   ( in[0][1] * A2313 - in[0][2] * A1313 + in[0][3] * A1213 );
	out[0][3] = det * - ( in[0][1] * A2312 - in[0][2] * A1312 + in[0][3] * A1212 );
	out[1][0] = det * - ( in[1][0] * A2323 - in[1][2] * A0323 + in[1][3] * A0223 );
	out[1][1] = det *   ( in[0][0] * A2323 - in[0][2] * A0323 + in[0][3] * A0223 );
	out[1][2] = det * - ( in[0][0] * A2313 - in[0][2] * A0313 + in[0][3] * A0213 );
	out[1][3] = det *   ( in[0][0] * A2312 - in[0][2] * A0312 + in[0][3] * A0212 );
	out[2][0] = det *   ( in[1][0] * A1323 - in[1][1] * A0323 + in[1][3] * A0123 );
	out[2][1] = det * - ( in[0][0] * A1323 - in[0][1] * A0323 + in[0][3] * A0123 );
	out[2][2] = det *   ( in[0][0] * A1313 - in[0][1] * A0313 + in[0][3] * A0113 );
	out[2][3] = det * - ( in[0][0] * A1312 - in[0][1] * A0312 + in[0][3] * A0112 );
	out[3][0] = det * - ( in[1][0] * A1223 - in[1][1] * A0223 + in[1][2] * A0123 );
	out[3][1] = det *   ( in[0][0] * A1223 - in[0][1] * A0223 + in[0][2] * A0123 );
	out[3][2] = det * - ( in[0][0] * A1213 - in[0][1] * A0213 + in[0][2] * A0113 );
	out[3][3] = det *   ( in[0][0] * A1212 - in[0][1] * A0212 + in[0][2] * A0112 );

	return true;
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


namespace { // StdLoc implementation 


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ADD_SC_PLUGIN(
	"Standard Locator",
	"Luca Scarabello <luca.scarabello@erdw.ethz.ch>",
	1, 0, 0
)
REGISTER_LOCATOR(StdLoc, "StdLoc");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const IDList StdLoc::_allowedParameters = {
	"method",
	"tttType",
	"tttModel",
	"PSTableOnly",
	"usePickUncertainties",
	"pickUncertaintyClasses",
	"enableConfidenceEllipsoid",
	"confLevel",
	"GridSearch.center",
	"GridSearch.autoLatLon",
	"GridSearch.size",
	"GridSearch.cellSize",
	"GridSearch.errorType",
	"GridSearch.maxRms",
	"LeastSquares.iterations",
	"LeastSquares.dampingFactor",
	"LeastSquares.solverType",
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StdLoc::init(const Config::Config &config) {
	vector<string> profileNames;
	try {
		profileNames = config.getStrings("StdLoc.profiles");
	}
	catch ( ... ) {}

	Profile defaultProf;
	defaultProf.name = "";
	defaultProf.method = Profile::Method::GridAndLsqr;
	defaultProf.tttType = "libtau";
	defaultProf.tttModel = "iasp91";
	defaultProf.PSTableOnly = true;
	defaultProf.usePickUncertainties = false;
	defaultProf.pickUncertaintyClasses = {0.000,0.025,0.050,0.100,0.200,0.400};
	defaultProf.enableConfidenceEllipsoid = true;
	defaultProf.confLevel = 0.9;
	defaultProf.gridSearch.autoLatLon = true;
	defaultProf.gridSearch.originLat = 0.;
	defaultProf.gridSearch.originLon = 0.;
	defaultProf.gridSearch.originDepth = 5.;
	defaultProf.gridSearch.xExtent = 20.;
	defaultProf.gridSearch.yExtent = 20.;
	defaultProf.gridSearch.zExtent = 5.;
	defaultProf.gridSearch.cellXExtent = 2.5;
	defaultProf.gridSearch.cellYExtent = 2.5;
	defaultProf.gridSearch.cellZExtent = 5.0;
	defaultProf.gridSearch.errorType = "L1";
	defaultProf.gridSearch.maxRms = -1;
	defaultProf.leastSquare.iterations = 20;
	defaultProf.leastSquare.dampingFactor = 0;
	defaultProf.leastSquare.solverType = "LSMR";

	_currentProfile = defaultProf;

	_profiles.clear();

	for ( const string &profileName : profileNames ) {
		Profile prof(defaultProf);
		prof.name = profileName;

		string prefix = string("StdLoc.profile.") + prof.name + ".";

		try {
			string method = config.getString(prefix + "method");
			if ( method == "LeastSquares" ) {
				prof.method = Profile::Method::LeastSquares;
			}
			else if ( method == "GridSearch" ) {
				prof.method = Profile::Method::GridSearch;
			}
			else if ( method == "GridSearch+LeastSquares" ) {
				prof.method = Profile::Method::GridAndLsqr;
			}
			else {
				SEISCOMP_ERROR("Profile %s: unrecognized method %s",
				               prof.name.c_str(), method.c_str());
				return false;
			}
		}
		catch ( ... ) {}

		try { prof.tttType = config.getString(prefix + "tableType"); }
		catch ( ... ) {}

		try { prof.tttModel = config.getString(prefix + "tableModel"); }
		catch ( ... ) {}

		try { prof.PSTableOnly = config.getBool(prefix + "PSTableOnly"); }
		catch ( ... ) {}

		try {
			prof.usePickUncertainties = config.getBool(prefix + "usePickUncertainties");
		}
		catch ( ... ) {}

		try {
			vector<string> tokens = config.getStrings(prefix + "pickUncertaintyClasses");
			if ( tokens.size() < 2 ) {
				SEISCOMP_ERROR("Profile %s: pickUncertaintyClasses should contain at least "
				               "2 values", prof.name.c_str());
				return false;
			}
			prof.pickUncertaintyClasses.clear();
			for ( const string& tok : tokens ) {
				double time;
				if ( !Core::fromString(time, tok) ) {
					SEISCOMP_ERROR("Profile %s: pickUncertaintyClasses is invalid",
				                 prof.name.c_str());
					return false;
				}
				prof.pickUncertaintyClasses.push_back(time);
			}
		}
		catch ( ... ) {}

		try {
			prof.enableConfidenceEllipsoid = config.getBool(prefix + "enableConfidenceEllipsoid");
		}
		catch ( ... ) {}

		try {
			prof.confLevel = config.getDouble(prefix + "confLevel");
		}
		catch ( ... ) {}

		try {
			prof.gridSearch.autoLatLon = config.getBool(prefix + "GridSearch.autoLatLon");
		}
		catch ( ... ) {}

		try {
			vector<string> tokens = config.getStrings(prefix + "GridSearch.center");
			if ( tokens.size() != 3 ||
			     !Core::fromString(prof.gridSearch.originDepth, tokens.at(2)) ) {
				SEISCOMP_ERROR("Profile %s: GridSearch.center is invalid",
				               prof.name.c_str());
				return false;
			}

			if ( prof.gridSearch.autoLatLon ) {
				prof.gridSearch.originLat = 0.0;
				prof.gridSearch.originLon = 0.0;
			}
			else if ( !Core::fromString(prof.gridSearch.originLat, tokens.at(0)) ||
			          !Core::fromString(prof.gridSearch.originLon, tokens.at(1)) ) {
				SEISCOMP_ERROR("Profile %s: GridSearch.center is invalid",
				               prof.name.c_str());
				return false;
			}
		}
		catch ( ... ) {}

		try {
			vector<string> tokens = config.getStrings(prefix + "GridSearch.size");
			if (tokens.size() != 3 ||
			    !Core::fromString(prof.gridSearch.xExtent, tokens.at(0)) ||
			    !Core::fromString(prof.gridSearch.yExtent, tokens.at(1)) ||
			    !Core::fromString(prof.gridSearch.zExtent, tokens.at(2)) ) {
				SEISCOMP_ERROR("Profile %s: GridSearch.size is invalid",
				               prof.name.c_str());
				return false;
			}
		}
		catch ( ... ) {}

		try {
			vector<string> tokens = config.getStrings(prefix + "GridSearch.cellSize");
			if ( tokens.size() != 3 ||
			     !Core::fromString(prof.gridSearch.cellXExtent, tokens.at(0)) ||
			     !Core::fromString(prof.gridSearch.cellYExtent, tokens.at(1)) ||
			     !Core::fromString(prof.gridSearch.cellZExtent, tokens.at(2))) {
				SEISCOMP_ERROR("Profile %s: GridSearch.cellSize is invalid",
				               prof.name.c_str());
				return false;
			}
		}
		catch ( ... ) {}

		try {
			prof.gridSearch.errorType = config.getString(prefix + "GridSearch.errorType");
			if ( prof.gridSearch.errorType != "L1" &&
			     prof.gridSearch.errorType != "L2") {
				SEISCOMP_ERROR("Profile %s: GridSearch.errorType is invalid",
				               prof.name.c_str());
				return false;
			}
		}
		catch ( ... ) {}

		try {
			prof.gridSearch.maxRms = config.getDouble(prefix + "GridSearch.maxRms");
		}
		catch ( ... ) {}

		try {
			prof.leastSquare.iterations = config.getInt(prefix + "LeastSquares.iterations");
		}
		catch ( ... ) {}

		try {
			prof.leastSquare.dampingFactor = config.getDouble(prefix + "LeastSquares.dampingFactor");
		}
		catch ( ... ) {}

		try {
			prof.leastSquare.solverType = config.getString(prefix + "LeastSquares.solverType");
			if ( prof.leastSquare.solverType != "LSMR" &&
			     prof.leastSquare.solverType != "LSQR") {
				SEISCOMP_ERROR("Profile %s: leastSquare.solverType is invalid",
				               prof.name.c_str());
				return false;
			}
		}
		catch ( ... ) {}

		_profiles[prof.name] = prof;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IDList StdLoc::profiles() const {
	IDList keys;
	transform(
		begin(_profiles), end(_profiles), back_inserter(keys),
		[](decltype(_profiles)::value_type const &pair) {
			return pair.first;
	});
	return keys;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::setProfile(const string &name) {
	if ( _currentProfile.name == name ) {
		return;
	}

	if ( _profiles.find(name) == _profiles.end() ) {
		return;
	}

	_currentProfile = _profiles.at(name);
	loadTTT();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string StdLoc::parameter(const string &name) const {
	if ( name == "method" ) {
		if ( _currentProfile.method == Profile::Method::LeastSquares ) {
			return "LeastSquares";
		}
		else if (_currentProfile.method == Profile::Method::GridSearch) {
			return "GridSearch";
		}
		else if (_currentProfile.method == Profile::Method::GridAndLsqr) {
			return "GridSearch+LeastSquares";
		}
	}
	else if ( name == "tttType" ) {
		return _currentProfile.tttType;
	}
	else if ( name == "tttModel" ) {
		return _currentProfile.tttModel;
	}
	else if ( name == "PSTableOnly" ) {
		return _currentProfile.PSTableOnly ? "y" : "n";
	}
	else if ( name == "usePickUncertainties" ) {
		return _currentProfile.usePickUncertainties ? "y" : "n";
	}
	else if ( name == "pickUncertaintyClasses" ) {
		string value;
		for ( double time :_currentProfile.pickUncertaintyClasses ) {
			if ( !value.empty() ) {
				value += ",";
			}
			value += Core::toString(time);
		}
		return value;
	}
	else if ( name == "enableConfidenceEllipsoid" ) {
		return _currentProfile.enableConfidenceEllipsoid ? "y" : "n";
	}
	else if ( name == "confLevel" ) {
		return Core::toString(_currentProfile.confLevel);
	}
	else if ( name == "LeastSquares.iterations" ) {
		return Core::toString(_currentProfile.leastSquare.iterations);
	}
	else if ( name == "LeastSquares.dampingFactor" ) {
		return Core::toString(_currentProfile.leastSquare.dampingFactor);
	}
	else if ( name == "LeastSquares.solverType" ) {
		return _currentProfile.leastSquare.solverType;
	}
	else if ( name == "GridSearch.autoLatLon" ) {
		return _currentProfile.gridSearch.autoLatLon ? "y" : "n";
	}
	else if ( name == "GridSearch.center" ) {
		if ( _currentProfile.gridSearch.autoLatLon ) {
			return "lat,lon," +
			        Core::toString(_currentProfile.gridSearch.originDepth);
		}
		else {
			return Core::toString(_currentProfile.gridSearch.originLat) + "," +
			       Core::toString(_currentProfile.gridSearch.originLon) + "," +
			       Core::toString(_currentProfile.gridSearch.originDepth);
		}
	}
	else if ( name == "GridSearch.size" ) {
		return Core::toString(_currentProfile.gridSearch.xExtent) + "," +
		       Core::toString(_currentProfile.gridSearch.yExtent) + "," +
		       Core::toString(_currentProfile.gridSearch.zExtent);
	}
	else if ( name == "GridSearch.cellSize" ) {
		return Core::toString(_currentProfile.gridSearch.cellXExtent) + "," +
		       Core::toString(_currentProfile.gridSearch.cellYExtent) + "," +
		       Core::toString(_currentProfile.gridSearch.cellZExtent);
	}
	else if ( name == "GridSearch.errorType" ) {
		return _currentProfile.gridSearch.errorType;
	}
	else if ( name == "GridSearch.maxRms" ) {
		return Core::toString(_currentProfile.gridSearch.maxRms);
	}

	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StdLoc::setParameter(const string &name, const string &value) {

	if ( name == "method" ) {
		if ( value == "LeastSquares" ) {
			_currentProfile.method = Profile::Method::LeastSquares;
			return true;
		}
		else if ( value == "GridSearch" ) {
			_currentProfile.method = Profile::Method::GridSearch;
			return true;
		}
		else if ( value == "GridSearch+LeastSquares" ) {
			_currentProfile.method = Profile::Method::GridAndLsqr;
			return true;
		}
		else {
			SEISCOMP_ERROR("Unrecognized method %s", value.c_str());
			return false;
		}
	}
	else if ( name == "tttType" ) {
		_currentProfile.tttType = value;
		return true;
	}
	else if ( name == "tttModel" ) {
		_currentProfile.tttModel = value;
		return true;
	}
	else if ( name == "PSTableOnly" ) {
		_currentProfile.PSTableOnly = (value == "y");
		return true;
	}
	else if ( name == "usePickUncertainties" ) {
		_currentProfile.usePickUncertainties = (value == "y");
		return true;
	}
	else if ( name == "pickUncertaintyClasses" ) {
		vector<string> tokens = splitString(value);
		if ( tokens.size() < 2 ) {
			SEISCOMP_ERROR("Profile %s: pickUncertaintyClasses should contain at least "
			               "2 values", _currentProfile.name.c_str());
			return false;
		}
		_currentProfile.pickUncertaintyClasses.clear();
		for ( const string& tok : tokens ) {
			double time;
			if ( !Core::fromString(time, tok) ) {
				SEISCOMP_ERROR("Profile %s: pickUncertaintyClasses is invalid",
				               _currentProfile.name.c_str());
				return false;
			}
			_currentProfile.pickUncertaintyClasses.push_back(time);
		}
		return true;
	}
	else if ( name == "enableConfidenceEllipsoid" ) {
		_currentProfile.enableConfidenceEllipsoid = (value == "y");
		return true;
	}
	else if ( name == "confLevel" ) {
		double tmp;
		if ( !Core::fromString(tmp, value) ) {
			return false;
		}
		_currentProfile.confLevel = tmp;
		return true;
	}
	else if ( name == "LeastSquares.iterations" ) {
		int tmp;
		if ( !Core::fromString(tmp, value) ) {
			return false;
		}
		_currentProfile.leastSquare.iterations = tmp;
		return true;
	}
	else if ( name == "LeastSquares.dampingFactor" ) {
		double tmp;
		if ( !Core::fromString(tmp, value) ) {
			return false;
		}
		_currentProfile.leastSquare.dampingFactor = tmp;
		return true;
	}
	else if ( name == "LeastSquares.solverType" ) {
		if ( value != "LSMR" && value != "LSQR" ) {
			return false;
		}
		_currentProfile.leastSquare.solverType = value;
		return true;
	}
	else if ( name == "GridSearch.autoLatLon" ) {
		_currentProfile.gridSearch.autoLatLon = (value == "y");
		return true;
	}
	else if ( name == "GridSearch.center" ) {
		vector<string> tokens = splitString(value);
		if ( tokens.size() != 3 ||
		     !Core::fromString(_currentProfile.gridSearch.originDepth,
		                       tokens.at(2))) {
			SEISCOMP_ERROR("Profile %s: GridSearch.center is invalid",
			               _currentProfile.name.c_str());
			return false;
		}

		if ( _currentProfile.gridSearch.autoLatLon ) {
			_currentProfile.gridSearch.originLat = 0.0;
			_currentProfile.gridSearch.originLon = 0.0;
		}
		else if ( !Core::fromString(_currentProfile.gridSearch.originLat,
		                            tokens.at(0)) ||
		          !Core::fromString(_currentProfile.gridSearch.originLon,
		                            tokens.at(1)) ) {
			SEISCOMP_ERROR("Profile %s: GridSearch.center is invalid",
			               _currentProfile.name.c_str());
			return false;
		}
		return true;
	}
	else if ( name == "GridSearch.size" ) {
		vector<string> tokens = splitString(value);
		if ( tokens.size() != 3 ||
		     !Core::fromString(_currentProfile.gridSearch.xExtent, tokens.at(0)) ||
		     !Core::fromString(_currentProfile.gridSearch.yExtent, tokens.at(1)) ||
		     !Core::fromString(_currentProfile.gridSearch.zExtent, tokens.at(2)) ) {
			return false;
		}
		return true;
	}
	else if ( name == "GridSearch.cellSize" ) {
		vector<string> tokens = splitString(value);
		if ( tokens.size() != 3 ||
		     !Core::fromString(_currentProfile.gridSearch.cellXExtent,
		                       tokens.at(0)) ||
		     !Core::fromString(_currentProfile.gridSearch.cellYExtent,
		                       tokens.at(1)) ||
		     !Core::fromString(_currentProfile.gridSearch.cellZExtent,
		                       tokens.at(2)) ) {
			return false;
		}
		return true;
	}
	else if ( name == "GridSearch.errorType" ) {
		if ( value != "L1" && value != "L2" ) {
			return false;
		}
		_currentProfile.gridSearch.errorType = value;
		return true;
	}
	else if ( name == "GridSearch.maxRms" ) {
		double tmp;
		if ( !Core::fromString(tmp, value) ) {
			return false;
		}
		_currentProfile.gridSearch.maxRms = tmp;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StdLoc::loadTTT() {
	if ( _tttType == _currentProfile.tttType &&
	     _tttModel == _currentProfile.tttModel ) {
		return true;
	}

	SEISCOMP_DEBUG("Loading ttt %s %s", _currentProfile.tttType.c_str(),
	               _currentProfile.tttModel.c_str());

	_tttType = "";
	_tttModel = "";

	_ttt = TravelTimeTableInterface::Create(_currentProfile.tttType.c_str());
	if ( !_ttt ) {
		SEISCOMP_ERROR("Failed to create TravelTimeTableInterface %s",
		               _currentProfile.tttType.c_str());
		return false;
	}

	if ( !_ttt->setModel(_currentProfile.tttModel) ) {
		SEISCOMP_ERROR("Failed to set model %s for TravelTimeTableInterface %s",
		               _currentProfile.tttModel.c_str(),
		               _currentProfile.tttType.c_str());
		return false;
	}

	_tttType = _currentProfile.tttType;
	_tttModel = _currentProfile.tttModel;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin *StdLoc::locate(PickList &pickList) {
	SEISCOMP_DEBUG("Locating Origin using PickList with profile '%s'",
	               _currentProfile.name.c_str());

	if ( _currentProfile.method != Profile::Method::GridSearch &&
	     _currentProfile.method != Profile::Method::GridAndLsqr ) {
		throw LocatorException("Cannot locate without an initial location: add the GridSearch");
	}

	loadTTT();

	vector<double> weights, sensorLat, sensorLon, sensorElev;
	computeAdditionlPickInfo(pickList, weights, sensorLat, sensorLon, sensorElev);

	double originLat, originLon, originDepth;
	Core::Time originTime;
	vector<double> travelTimes;
	CovMtrx covm;

	locateGridSearch(pickList, weights, sensorLat, sensorLon, sensorElev,
	                 originLat, originLon, originDepth, originTime, travelTimes,
	                 covm, _currentProfile.enableConfidenceEllipsoid);

	return createOrigin(pickList, weights, sensorLat, sensorLon, sensorElev,
	                    originLat, originLon, originDepth, originTime,
	                    travelTimes, covm);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin *StdLoc::locate(PickList &pickList, double initLat, double initLon,
                       double initDepth, const Core::Time &initTime) {

	if ( isInitialLocationIgnored() ) {
		return locate(pickList);
	}

	loadTTT();

	SEISCOMP_DEBUG("Locating Origin using PickList and an initial location using "
	               "profile '%s'",
	               _currentProfile.name.c_str());

	vector<double> weights, sensorLat, sensorLon, sensorElev;
	computeAdditionlPickInfo(pickList, weights, sensorLat, sensorLon, sensorElev);

	double originLat, originLon, originDepth;
	Core::Time originTime;
	vector<double> travelTimes;
	CovMtrx covm;

	if ( _currentProfile.method == Profile::Method::GridSearch ||
	     _currentProfile.method == Profile::Method::GridAndLsqr ) {
		locateGridSearch(pickList, weights, sensorLat, sensorLon, sensorElev,
		                 originLat, originLon, originDepth, originTime, travelTimes,
		                 covm, _currentProfile.enableConfidenceEllipsoid);
	}
	else if (_currentProfile.method == Profile::Method::LeastSquares) {
		locateLeastSquares(pickList, weights, sensorLat, sensorLon, sensorElev,
		                   initLat, initLon, initDepth, initTime, originLat,
		                   originLon, originDepth, originTime, travelTimes,
		                   covm, _currentProfile.enableConfidenceEllipsoid);
	}

	return createOrigin(pickList, weights, sensorLat, sensorLon, sensorElev,
	                    originLat, originLon, originDepth, originTime,
	                    travelTimes, covm);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin *StdLoc::relocate(const Origin *origin) {
	SEISCOMP_DEBUG("Relocating origin (%s) with profile '%s'",
	               origin->publicID().c_str(), _currentProfile.name.c_str());

	if ( !origin ) {
		return nullptr;
	}

	double initLat, initLon, initDepth;
	Core::Time initTime;

	try {
		initLat = origin->latitude().value();
	}
	catch ( ... ) {
		throw LocatorException("incomplete origin, latitude is not set");
	}

	try {
		initLon = origin->longitude().value();
	}
	catch ( ... ) {
		throw LocatorException("incomplete origin, longitude is not set");
	}

	try {
		initDepth = origin->depth().value();
	}
	catch ( ... ) {
		throw LocatorException("incomplete origin, depth is not set");
	}

	try {
		initTime = origin->time().value();
	}
	catch ( ... ) {
		throw LocatorException("incomplete origin, depth is not set");
	}

	PickList picks;

	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		PickPtr pick = getPick(origin->arrival(i));
		if ( !pick ) {
			throw PickNotFoundException("pick '" + origin->arrival(i)->pickID() +
			                            "' not found");
		}

		try {
			// Phase definition of arrival and pick different?
			if ( pick->phaseHint().code() != origin->arrival(i)->phase().code() ) {
				PickPtr np = new Pick(*pick);
				np->setPhaseHint(origin->arrival(i)->phase());
				pick = np;
			}
		}
		catch ( ... ) {
			// Pick has no phase hint?
			PickPtr np = new Pick(*pick);
			np->setPhaseHint(origin->arrival(i)->phase());
			pick = np;
		}

		int flags = Seismology::arrivalToFlags(origin->arrival(i));
		picks.push_back(PickItem(pick.get(), flags));
	}

	return locate(picks, initLat, initLon, initDepth, initTime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::computeAdditionlPickInfo(const PickList &pickList,
                                      vector<double> &weights,
                                      vector<double> &sensorLat,
                                      vector<double> &sensorLon,
                                      vector<double> &sensorElev) const {
	weights.resize(pickList.size());
	sensorLat.resize(pickList.size());
	sensorLon.resize(pickList.size());
	sensorElev.resize(pickList.size());

	int activeArrivals = 0;
	for ( size_t i = 0; i < pickList.size(); ++i ) {
		const PickItem &pi = pickList[i];
		const PickPtr pick = pi.pick;

		SensorLocation *sloc = getSensorLocation(pick.get());
		if ( !sloc ) {
			throw StationNotFoundException(
			            "sensor location '" + pick->waveformID().networkCode() + "." +
			            pick->waveformID().stationCode() + "." +
			            pick->waveformID().locationCode() + "' not found");
		}

		try {
			sensorLat[i] = sloc->latitude();
			sensorLon[i] = sloc->longitude();
			sensorElev[i] = sloc->elevation();
		}
		catch ( ... ) {
			throw LocatorException(
				"sensor location '" + pick->waveformID().networkCode() + "." +
				pick->waveformID().stationCode() + "." +
				pick->waveformID().locationCode() + "' is incomplete w.r.t. lat/lon"
			);
		}

		if ( pi.flags == LocatorInterface::F_NONE ) {
			SEISCOMP_DEBUG("Omitting disabled pick %s@%s.%s.%s",
			               pick->phaseHint().code().c_str(),
			               pick->waveformID().networkCode().c_str(),
			               pick->waveformID().stationCode().c_str(),
			               pick->waveformID().locationCode().c_str());
			weights[i] = 0; //  mark the pick as unused
			continue;
		}

		weights[i] = 1.0; // marks the pick as used

		if (_currentProfile.usePickUncertainties) {
			weights[i] = computePickWeight(pick.get(), _currentProfile.pickUncertaintyClasses);
		}
		++activeArrivals;
	}

	if ( activeArrivals <= 0 ) {
		throw LocatorException("Empty set of active arrivals");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::locateGridSearch(
	const PickList &pickList, const vector<double> &weights,
	const vector<double> &sensorLat, const vector<double> &sensorLon,
	const vector<double> &sensorElev, double &newLat, double &newLon,
	double &newDepth, Core::Time &newTime, vector<double> &travelTimes,
	CovMtrx& covm, bool computeCovMtrx
) const {
	SEISCOMP_DEBUG("Start Grid Search");

	if ( !_ttt ) {
		throw LocatorException("Travel time table has not been loaded, check logs");
	}

	if ( pickList.empty() ) {
		throw LocatorException("Empty observation set");
	}

	if ( weights.size() != pickList.size() ||
	     sensorLat.size() != pickList.size() ||
	     sensorLon.size() != pickList.size() ||
	     sensorElev.size() != pickList.size()) {
		throw LocatorException("Interna logic error");
	}

	covm.valid = false;

	travelTimes.resize(pickList.size());

	double gridOriginLat = _currentProfile.gridSearch.originLat;
	double gridOriginLon = _currentProfile.gridSearch.originLon;
	if (_currentProfile.gridSearch.autoLatLon) {
		gridOriginLat = computeMean(sensorLat);
		gridOriginLon = Geo::GeoCoordinate::normalizeLon(computeCircularMean(sensorLon, false));
		SEISCOMP_DEBUG("GridSearch.center latitude %f longitude %f", gridOriginLat,
		               gridOriginLon);
	}

	// 
	// Build the list of cells withing the grid
	// 
	vector<Cell> cells;
	for (double x = -_currentProfile.gridSearch.xExtent / 2. +
	           _currentProfile.gridSearch.cellXExtent / 2.;
	      x < _currentProfile.gridSearch.xExtent / 2.;
	      x += _currentProfile.gridSearch.cellXExtent ) {
		for (double y = -_currentProfile.gridSearch.yExtent / 2. +
		           _currentProfile.gridSearch.cellYExtent / 2;
		      y < _currentProfile.gridSearch.yExtent / 2.;
		      y += _currentProfile.gridSearch.cellYExtent ) {
			for (double z = -_currentProfile.gridSearch.zExtent / 2. +
			           _currentProfile.gridSearch.cellZExtent / 2;
			      z < _currentProfile.gridSearch.zExtent / 2.;
			      z += _currentProfile.gridSearch.cellZExtent ) {

				Cell cell;
				cell.valid = false;
				cell.x = x;
				cell.y = y;
				cell.z = z;
				cell.org.depth = _currentProfile.gridSearch.originDepth + z;

				// compute distance and azimuth of the cell centroid to the grid origin
				double distance = sqrt(y * y + x * x); // km
				double azimuth = rad2deg(atan2(x, y));

				// Computes the coordinates (lat, lon) of the point which is at a degree
				// azimuth and km distance as seen from the other point location
				computeCoordinates(distance, azimuth,
				                   gridOriginLat, gridOriginLon,
				                   cell.org.lat, cell.org.lon);

				cells.push_back(cell);
			}
		}
	}

	//
	// Process each cell now
	//
	vector<double> originTimes;
	vector<double> timeWeights;
	Cell bestCell;
	bestCell.valid = false;
	vector<double> bestTravelTimes;
	CovMtrx bestCovM;

	for ( Cell& cell : cells ) {

		SEISCOMP_DEBUG("Processing cell x %g y %g z %g -> lon %g lat %g depth %g",
		               cell.x, cell.y, cell.z,
		               cell.org.lon, cell.org.lat, cell.org.depth);

		originTimes.clear();
		timeWeights.clear();

		bool tttError = false;
		for ( size_t i = 0; i < pickList.size(); ++i ) {
			const PickItem &pi = pickList[i];
			const PickPtr pick = pi.pick;

			if ( weights[i] <= 0 ) {
				continue;
			}

			TravelTime tt;

			try {
				const char * phaseName = pick->phaseHint().code().c_str();
				if ( _currentProfile.PSTableOnly ) {
					if (*pick->phaseHint().code().begin() == 'P') {
						phaseName = "P";
					}
					else if (*pick->phaseHint().code().begin() == 'S') {
						phaseName = "S";
					}
				}
				tt = _ttt->compute(phaseName, cell.org.lat, cell.org.lon, cell.org.depth,
													 sensorLat[i], sensorLon[i], sensorElev[i]);
			}
			catch ( exception &e ) {
				SEISCOMP_WARNING("Travel Time Table error for %s@%s.%s.%s and lat "
												 "%.6f lon %.6f depth %.3f: %s",
												 pick->phaseHint().code().c_str(),
												 pick->waveformID().networkCode().c_str(),
												 pick->waveformID().stationCode().c_str(),
												 pick->waveformID().locationCode().c_str(),
												 cell.org.lat, cell.org.lon, cell.org.depth, e.what());
				tttError = true;
				break;
			}

			if ( tt.time < 0 ) {
				SEISCOMP_WARNING("Travel Time Table error: data not returned for "
												 "%s@%s.%s.%s and lat %.6f lon %.6f depth %.3f",
												 pick->phaseHint().code().c_str(),
												 pick->waveformID().networkCode().c_str(),
												 pick->waveformID().stationCode().c_str(),
												 pick->waveformID().locationCode().c_str(),
												 cell.org.lat, cell.org.lon, cell.org.depth);
				tttError = true;
				break;
			}

			travelTimes[i] = tt.time;
			double pickTime = double(pick->time().value());
			originTimes.push_back(pickTime - travelTimes[i]);
			timeWeights.push_back(weights[i]);
		}

		if ( tttError ) {
			SEISCOMP_DEBUG("Could not compute travel times of active arrivals: "
										 "skip cell");
			continue;
		}

		// Compute origin time for the Cell
		double originTime, originTimeError;
		Math::Statistics::average(originTimes, timeWeights, originTime,
		                          originTimeError);
		cell.org.time = Core::Time(originTime);

		//
		// Optionally run Least Squares from cell center
		//
		if ( _currentProfile.method == Profile::Method::GridAndLsqr ) {
			try {
				// note: cell data will be updated
				locateLeastSquares(pickList, weights, sensorLat, sensorLon, sensorElev, 
				                   cell.org.lat, cell.org.lon, cell.org.depth, cell.org.time,
				                   cell.org.lat, cell.org.lon, cell.org.depth, cell.org.time,
				                   travelTimes, covm, computeCovMtrx);
			}
			catch ( exception &e ) {
				SEISCOMP_DEBUG(
					"Could not get a Least Square solution (%s): skip cell",
					e.what()
				);
				continue;
			}
		}

		//
		// Compute error for this location
		//
		double l1SumWeightedResiduals = 0.0;
		double l2SumWeightedResiduals = 0.0;
		double rms = 0.0;
		int numResiduals = 0;

		for ( size_t i = 0; i < pickList.size(); ++i ) {
			const PickItem &pi = pickList[i];
			const PickPtr pick = pi.pick;

			if ( weights[i] <= 0 ) {
				continue;
			}

			Core::Time pickTime = pick->time().value();
			double residual =
				(pickTime - (cell.org.time + Core::TimeSpan(travelTimes[i]))).length();
			l1SumWeightedResiduals += abs(residual * weights[i]);
			l2SumWeightedResiduals += (residual * weights[i]) * (residual * weights[i]);
			rms = residual * residual;
			numResiduals++;
		}

		rms = sqrt(rms) / numResiduals;

		if ( _currentProfile.gridSearch.maxRms > 0 &&
				 rms > _currentProfile.gridSearch.maxRms ) {
			SEISCOMP_DEBUG(
				"Unweighted rms %f is above GridSearch.maxRms %f -> reject cell",
				rms, _currentProfile.gridSearch.maxRms
			);
			continue;
		}

		if ( _currentProfile.gridSearch.errorType == "L1" ) {
			cell.org.error = l1SumWeightedResiduals;
		}
		else if ( _currentProfile.gridSearch.errorType == "L2" ) {
			cell.org.error = l2SumWeightedResiduals;
		}
		else {
			throw LocatorException("The GridSearch error type can only be L1 or "
														 "L2, but it is set to" +
														 _currentProfile.gridSearch.errorType);
		}

		cell.valid = true;

		SEISCOMP_DEBUG("%s error %g unweighted rms %f",
									 _currentProfile.gridSearch.errorType.c_str(),
									 cell.org.error, rms);

		if ( !bestCell.valid || bestCell.org.error > cell.org.error ) {
			bestCell = cell;
			bestTravelTimes = travelTimes;
			bestCovM = covm;
			SEISCOMP_DEBUG("Preferring this cell with error %g", bestCell.org.error);
		}
	}

	if ( !bestCell.valid ) {
		throw LocatorException("Couldn't find a solution");
	}

	// return the travel times for the solution
	travelTimes = bestTravelTimes;

	// compute covariance matrix if that is not already computed by LeastSquares
	if ( computeCovMtrx && 
      _currentProfile.method != Profile::Method::GridAndLsqr) {
		computeCovarianceMatrix(cells, bestCell, covm);
	} else {
		covm = bestCovM;
	}

	newLat = bestCell.org.lat;
	newLon = bestCell.org.lon;
	newDepth = bestCell.org.depth;
	newTime = bestCell.org.time;

	SEISCOMP_DEBUG(
		"Grid Search lowest %s error %g at lon %g lat %g depth %g time %s",
		_currentProfile.gridSearch.errorType.c_str(), bestCell.org.error,
		newLon, newLat, newDepth, newTime.iso().c_str()
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::locateLeastSquares(
	const PickList &pickList, const vector<double> &weights,
	const vector<double> &sensorLat, const vector<double> &sensorLon,
	const vector<double> &sensorElev, double initLat, double initLon,
	double initDepth, Core::Time initTime, double &newLat, double &newLon,
	double &newDepth, Core::Time &newTime, vector<double> &travelTimes,
	CovMtrx& covm, bool computeCovMtrx
) const {

	SEISCOMP_DEBUG(
		"Start Least Square with initial lat %.6f lon %.6f depth %.3f time %s",
		initLat, initLon, initDepth, initTime.iso().c_str()
	);

	if ( !_ttt ) {
		throw LocatorException("Travel time table has not been loaded, check logs");
	}

	if ( pickList.empty() ) {
		throw LocatorException("Empty observation set");
	}

	if ( weights.size() != pickList.size() ||
	     sensorLat.size() != pickList.size() ||
	     sensorLon.size() != pickList.size() ||
	     sensorElev.size() != pickList.size() ) {
		throw LocatorException("Interna logic error");
	}

	covm.valid = false;

	travelTimes.resize(pickList.size());

	vector<double> backazis(pickList.size());
	vector<double> dtdds(pickList.size());
	vector<double> dtdhs(pickList.size());

	for ( int iteration = 0; iteration <= _currentProfile.leastSquare.iterations;
	      ++iteration ) {
		
		// the last additional iteration is for final stats (no inversion)
		bool lastIteration = (iteration == _currentProfile.leastSquare.iterations);

		//
		// Load the information we need to build the Equation System
		//
		for ( size_t i = 0; i < pickList.size(); ++i ) {
			const PickItem &pi = pickList[i];
			const PickPtr pick = pi.pick;

			if ( weights[i] <= 0 ) {
				continue;
			}

			// get back azimuth, we don't need the distance, which will 
			// be discarded (it's a waste of computation)
			computeDistance(initLat, initLon, sensorLat[i], sensorLon[i],
			                nullptr, &backazis[i]);

			TravelTime tt;

			try {
				const char * phaseName = pick->phaseHint().code().c_str();
				if ( _currentProfile.PSTableOnly ) {
					if ( *pick->phaseHint().code().begin() == 'P' ) {
						phaseName = "P";
					}
					else if ( *pick->phaseHint().code().begin() == 'S' ) {
						phaseName = "S";
					}
				}

				tt = _ttt->compute(phaseName, initLat, initLon, initDepth,
				                   sensorLat[i], sensorLon[i], sensorElev[i]);

			}
			catch ( exception &e ) {
				SEISCOMP_WARNING("Travel Time Table error for %s@%s.%s.%s and lat %.6f "
				                 "lon %.6f depth %.3f: %s",
				                 pick->phaseHint().code().c_str(),
				                 pick->waveformID().networkCode().c_str(),
				                 pick->waveformID().stationCode().c_str(),
				                 pick->waveformID().locationCode().c_str(), initLat,
				                 initLon, initDepth, e.what());
				throw LocatorException("Travel Time Table error");
			}

			if ( tt.time < 0 || (tt.time > 0 && tt.dtdd == 0 && tt.dtdh == 0) ) {
				SEISCOMP_WARNING("Travel Time Table error: data not returned for "
				                 "%s@%s.%s.%s and lat %.6f lon %.6f depth %.3f",
				                 pick->phaseHint().code().c_str(),
				                 pick->waveformID().networkCode().c_str(),
				                 pick->waveformID().stationCode().c_str(),
				                 pick->waveformID().locationCode().c_str(), initLat,
				                 initLon, initDepth);
				throw LocatorException("Travel Time Table error");
			}

			travelTimes[i] = tt.time;
			dtdds[i] = tt.dtdd;
			dtdhs[i] = tt.dtdh;
		}

		// the last iteration is use for computing the travelTimes on the final
		// location and eventually the covariance matrix
		if ( lastIteration && !computeCovMtrx) {
			break;
		}

		//
		// Prepare the Equation System
		//
		System eq(pickList.size());

		for ( size_t i = 0; i < pickList.size(); ++i ) {
			const PickItem &pi = pickList[i];
			const PickPtr pick = pi.pick;

			eq.W[i] = weights[i];

			if ( weights[i] <= 0 ) {
				eq.W[i] = 0;
				continue;
			}

			Core::Time pickTime = pick->time().value();
			double residual = (pickTime - (initTime + Core::TimeSpan(travelTimes[i]))).length();
			eq.r[i] = residual;

			const double bazi = deg2rad(backazis[i]);
			eq.G[i][0] = dtdds[i] * sin(bazi); // dx [sec/deg]
			eq.G[i][1] = dtdds[i] * cos(bazi); // dy [sec/deg]
			eq.G[i][2] = dtdhs[i];             // dz [sec/km]
			eq.G[i][3] = 1.;                   // dtime [sec]
		}

		if ( lastIteration && computeCovMtrx) {
			computeCovarianceMatrix(eq, covm);
			break;
		}
 
		//
		// Solve the system
		//
		try {
			ostringstream solverLogs;
			if ( _currentProfile.leastSquare.solverType == "LSMR" ) {
				// solve
				Adapter<lsmrBase> solver = solve<lsmrBase>(
					eq, &solverLogs, _currentProfile.leastSquare.dampingFactor
				);
				// print some information
				SEISCOMP_DEBUG("Solver stopped because %u : %s (used %u iterations)",
				               solver.GetStoppingReason(),
				               solver.GetStoppingReasonMessage().c_str(),
				               solver.GetNumberOfIterationsPerformed());
			}
			else if ( _currentProfile.leastSquare.solverType == "LSQR" ) {
				// solve
				Adapter<lsqrBase> solver = solve<lsqrBase>(
					eq, &solverLogs, _currentProfile.leastSquare.dampingFactor
				);
				// print some information
				SEISCOMP_DEBUG("Solver stopped because %u : %s (used %u iterations)",
				               solver.GetStoppingReason(),
				               solver.GetStoppingReasonMessage().c_str(),
				               solver.GetNumberOfIterationsPerformed());
			}
			else {
				throw LocatorException(
					"Solver type can only be LSMR or LSQR, but it is set to" +
					_currentProfile.leastSquare.solverType
				);
			}

			SEISCOMP_DEBUG("Solver logs:\n%s", solverLogs.str().c_str());

		}
		catch ( exception &e ) {
			throw LocatorException(e.what());
		}

		//
		// Load the solution
		//
		double lonCorrection = eq.m[0];   // deg
		double latCorrection = eq.m[1];   // deg
		double depthCorrection = eq.m[2]; // km
		double timeCorrection = eq.m[3];  // sec

		if ( !isfinite(lonCorrection) || !isfinite(latCorrection) ||
		     !isfinite(depthCorrection) || !isfinite(timeCorrection) ) {
			throw LocatorException("Couldn't find a solution to the equation system");
		}

		newLat = initLat + latCorrection;
		newLon = initLon + lonCorrection;
		newTime = initTime + Core::TimeSpan(timeCorrection);
		newDepth = initDepth + depthCorrection;

		SEISCOMP_DEBUG("Least Square iteration %d: corrections lat %f [km] lon %f [km] "
		               "depth %f [km] time %f [sec]. New source parameters lat %.6f "
		               "lon %.6f depth %.3f time %s",
		               iteration, latCorrection, lonCorrection, depthCorrection,
		               timeCorrection, newLat, newLon, newDepth,
		               newTime.iso().c_str());

		//
		// set the initial values for the next iteration
		//
		initLat = newLat;
		initLon = newLon;
		initDepth = newDepth;
		initTime = newTime;
	}

	SEISCOMP_DEBUG("Least Square final solution lat %.6f lon %.6f "
	               "depth %.3f time %s",
	               newLat, newLon, newDepth, newTime.iso().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::computeCovarianceMatrix(const vector<Cell>& cells,
                                     const Cell& bestCell,
                                     CovMtrx& covm) const {
	covm.valid = false;

	auto toWeight = [&bestCell](double error) {
		return std::exp(-0.5*error/bestCell.org.error);
	};

	double wmeanLat = 0, wmeanLon = 0, wmeanDepth = 0, wmeanTime = 0;
	//vector<double> longitudes;
	//vector<double> weights;

	//for ( const Cell& cell : cells ) {
	//	if ( !cell.valid ) {
	//		continue;
	//	}
	//	double weight = toWeight(cell.org.error);
	//	weightSum += weight;
	//	wmeanLat += cell.org.lat * weight;
	//	wmeanDepth += cell.org.depth * weight;
	//	wmeanTime += static_cast<double>(cell.org.time) * weight;
	//	longitudes.push_back(cell.org.lon);
	//	weights.push_back(weight);
	//}
	//wmeanLat /= weightSum;
	//wmeanDepth /= weightSum;
	//wmeanTime /= weightSum;
	//wmeanLon = Geo::GeoCoordinate::normalizeLon(
	//	 computedWeightedCircularMean(longitudes, weights, false)
	//);

	//
	// The covaraiance should be computed using the expected values
	// that is the weigheted mean of lat, lon, depth and time
	// However the confidence ellipsoid is considered w.r.t. the
	// origin location, so we compute the covariance matrix
	// w.r.t. to the origin location (bestCell)
	//
	wmeanLat = bestCell.org.lat;
	wmeanLon = bestCell.org.lon;
	wmeanDepth = bestCell.org.depth;
	wmeanTime = static_cast<double>(bestCell.org.time);

	double weightSum = 0.0;
	std::array<std::array<double,4>,4> m = {};
	for ( const Cell& cell : cells ) {
		if ( !cell.valid ) {
			continue;
		}
		double weight = toWeight(cell.org.error);
		weightSum += weight;
		double rLon = computeDistance(cell.org.lat, cell.org.lon, 
		                              cell.org.lat, wmeanLon);
		if ( cell.org.lon > wmeanLon) {
			rLon = Math::Geo::deg2km(rLon);
		} else {
			rLon = -Math::Geo::deg2km(rLon);
		}
		double rLat = computeDistance(cell.org.lat, cell.org.lon,
		                              wmeanLat, cell.org.lon);
		if ( cell.org.lat > wmeanLat) {
			rLat = Math::Geo::deg2km(rLat);
		} else {
			rLat = -Math::Geo::deg2km(rLat);
		}
		double rDepth = cell.org.depth - wmeanDepth;
		double rTime = (cell.org.time - Core::Time(wmeanTime)).length();
		m[0][0] += weight * rLon   * rLon;
		m[0][1] += weight * rLon   * rLat;
		m[0][2] += weight * rLon   * rDepth;
		m[0][3] += weight * rLon   * rTime;
		m[1][0] += weight * rLat   * rLon;
		m[1][1] += weight * rLat   * rLat;
		m[1][2] += weight * rLat   * rDepth;
		m[1][3] += weight * rLat   * rTime;
		m[2][0] += weight * rDepth * rLon;
		m[2][1] += weight * rDepth * rLat;
		m[2][2] += weight * rDepth * rDepth;
		m[2][3] += weight * rDepth * rTime;
		m[3][0] += weight * rTime  * rLon;
		m[3][1] += weight * rTime  * rLat;
		m[3][2] += weight * rTime  * rDepth;
		m[3][3] += weight * rTime  * rTime;
	}

	m[0][0] /= weightSum;
	m[0][1] /= weightSum;
	m[0][2] /= weightSum;
	m[0][3] /= weightSum;
	m[1][0] /= weightSum;
	m[1][1] /= weightSum;
	m[1][2] /= weightSum;
	m[1][3] /= weightSum;
	m[2][0] /= weightSum;
	m[2][1] /= weightSum;
	m[2][2] /= weightSum;
	m[2][3] /= weightSum;
	m[3][0] /= weightSum;
	m[3][1] /= weightSum;
	m[3][2] /= weightSum;
	m[3][3] /= weightSum;

	covm.sxx = m[0][0];
	covm.sxy = m[0][1];
	covm.sxz = m[0][2];
	covm.sxt = m[0][3];
	covm.syy = m[1][1];
	covm.syz = m[1][2];
	covm.syt = m[1][3];
	covm.szz = m[2][2];
	covm.szt = m[2][3];
	covm.stt = m[3][3];
	covm.valid = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::computeCovarianceMatrix(const System& eq, CovMtrx& covm) const {
	//
	// Accordingly to "Routine Data Processing in Earthquake Seismology" 
	// by Jens Havskov and Lars Ottemoller, the covariance matrix can be
	// computed as:
	//
	//  covm = sigma^2 * inverse_matrix(G.T * G)
	//
	// sigma^2 is the variance of the arrival times multiplied by the
	// identity matrix defined as:
	//
	//   sigma^2 = 1/nfd * sum(residual^2)
	//
	// nfd is the degrees of freedom (numer of phases - 4) and the
	// residuals are computed on the best fitting hypocenter
	//
	// G is the matrix of the partial derivatives of the slowness vector
	// with respect to event/station location in the 3 directions and
	// 1 in the last column corresponding to the source time correction
	// term and G.T is G transposed
	//
	// Note: the resulting confidence ellipsoid seems in the same order
	// of magnitude of NonLinLoc. LOCSAT seems to be using sigma^2 = 1,
	// so the resulting confidence ellipsoid is bigger
	//
	covm.valid = false;

	if ( eq.numRowsG <= 4 ) {
		SEISCOMP_DEBUG("Cannot compute covariance matrix: less than 5 arrivals");
		return;
	}

	double sigma2 = 0;
	for ( unsigned int ob = 0; ob < eq.numRowsG; ob++ ) {
		sigma2 += eq.r[ob] * eq.r[ob];
	}
	sigma2 /= eq.numRowsG-4;

	std::array<std::array<double,4>,4> GtG = {}; // G.T * G
	for ( unsigned int ob = 0; ob < eq.numRowsG; ob++ ) {
		double gLon = eq.G[ob][0] / KM_OF_DEGREE;
		double gLat = eq.G[ob][1] / KM_OF_DEGREE;
		double gDepth = eq.G[ob][2];
		double gTime = eq.G[ob][3];
		GtG[0][0] += gLon   * gLon;
		GtG[0][1] += gLon   * gLat;
		GtG[0][2] += gLon   * gDepth;
		GtG[0][3] += gLon   * gTime;
		GtG[1][0] += gLat   * gLon;
		GtG[1][1] += gLat   * gLat;
		GtG[1][2] += gLat   * gDepth;
		GtG[1][3] += gLat   * gTime;
		GtG[2][0] += gDepth * gLon;
		GtG[2][1] += gDepth * gLat;
		GtG[2][2] += gDepth * gDepth;
		GtG[2][3] += gDepth * gTime;
		GtG[3][0] += gTime  * gLon;
		GtG[3][1] += gTime  * gLat;
		GtG[3][2] += gTime  * gDepth;
		GtG[3][3] += gTime  * gTime;
	}

	std::array<std::array<double,4>,4> inverseGtG;
	if ( ! invertMatrix4x4(GtG, inverseGtG) ) {
		SEISCOMP_DEBUG("Cannot compute covariance matrix: G.T*G not invertible");
		return;
	}

	covm.sxx = inverseGtG[0][0] * sigma2;
	covm.sxy = inverseGtG[0][1] * sigma2;
	covm.sxz = inverseGtG[0][2] * sigma2;
	covm.sxt = inverseGtG[0][3] * sigma2;
	covm.syy = inverseGtG[1][1] * sigma2;
	covm.syz = inverseGtG[1][2] * sigma2;
	covm.syt = inverseGtG[1][3] * sigma2;
	covm.szz = inverseGtG[2][2] * sigma2;
	covm.szt = inverseGtG[2][3] * sigma2;
	covm.stt = inverseGtG[3][3] * sigma2;
	covm.valid = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin *StdLoc::createOrigin(
	const PickList &pickList, const vector<double> &weights,
	const vector<double> &sensorLat, const vector<double> &sensorLon,
	const vector<double> &sensorElev, double originLat, double originLon,
	double originDepth, const Core::Time &originTime,
	const vector<double> &travelTimes, const CovMtrx& covm
) const {
	if ( weights.size() != pickList.size() ) {
		throw LocatorException("Internal logic error");
	}

	DataModel::CreationInfo ci;
	ci.setCreationTime(Core::Time::GMT());

	Origin *origin = Origin::Create();
	SEISCOMP_DEBUG("New origin publicID: %s", origin->publicID().c_str());

	origin->setCreationInfo(ci);
	origin->setEarthModelID(_tttType + ":" + _tttModel);
	origin->setMethodID("StdLoc");
	origin->setTime(DataModel::TimeQuantity(originTime));
	origin->setLatitude(DataModel::RealQuantity(originLat));
	origin->setLongitude(DataModel::RealQuantity(originLon));
	origin->setDepth(DataModel::RealQuantity(originDepth));

	set<string> associatedStations;
	set<string> usedStations;
	int associatedPhaseCount = 0;
	int usedPhaseCount = 0;
	vector<double> distances;
	vector<double> azi;
	double sumSquaredResiduals = 0.0;
	double sumSquaredWeights = 0.0;

	for ( size_t i = 0; i < pickList.size(); ++i ) {
		const PickItem &pi = pickList[i];
		const PickPtr pick = pi.pick;

		Core::Time pickTime = pick->time().value();

		++associatedPhaseCount;
		associatedStations.insert(pick->waveformID().networkCode() + "." +
		                          pick->waveformID().stationCode() + "." +
		                          pick->waveformID().locationCode());

		double azimuth = 0;
		double Hdist = computeDistance(originLat, originLon,
		                               sensorLat[i], sensorLon[i],
		                               &azimuth);
		Hdist = Math::Geo::deg2km(Hdist);
		double Vdist = abs(originDepth + sensorElev[i] / 1000);
		double distance = Math::Geo::km2deg(sqrt(Hdist * Hdist + Vdist * Vdist));

		// prepare the new arrival
		DataModel::Arrival *newArr = new DataModel::Arrival();
		newArr->setCreationInfo(ci);
		newArr->setPickID(pick->publicID());
		newArr->setPhase(pick->phaseHint().code());
		newArr->setHorizontalSlownessUsed(false);
		newArr->setBackazimuthUsed(false);
		newArr->setAzimuth(azimuth);
		newArr->setDistance(distance);

		if ( weights[i] <= 0 ) {
			// Not used
			newArr->setTimeUsed(false);
			newArr->setWeight(0.0);
			newArr->setTimeResidual(0.0);
		}
		else {
			newArr->setTimeUsed(true);
			newArr->setWeight(weights[i]);

			double residual =
			    (pickTime - (originTime + Core::TimeSpan(travelTimes[i]))).length();
			newArr->setTimeResidual(residual);

			sumSquaredResiduals += (residual * weights[i]) * (residual * weights[i]);
			sumSquaredWeights += weights[i] * weights[i];

			usedPhaseCount++;
			distances.push_back(distance);
			azi.push_back(azimuth);
			usedStations.insert(pick->waveformID().networkCode() + "." +
			                    pick->waveformID().stationCode() + "." +
			                    pick->waveformID().locationCode());
		}

		origin->add(newArr);
	}

	//
	// finish computing stats
	//
	double rms = sqrt(sumSquaredResiduals / sumSquaredWeights);

	double primaryAz = 360., secondaryAz = 360.;
	if ( azi.size() >= 2 ) {
		primaryAz = secondaryAz = 0.;
		sort(azi.begin(), azi.end());
		vector<double>::size_type aziCount = azi.size();
		azi.push_back(azi[0] + 360.);
		azi.push_back(azi[1] + 360.);
		for ( vector<double>::size_type i = 0; i < aziCount; ++i ) {
			double gap = azi[i + 1] - azi[i];
			if ( gap > primaryAz ) {
				primaryAz = gap;
			}
			gap = azi[i + 2] - azi[i];
			if ( gap > secondaryAz ) {
				secondaryAz = gap;
			}
		}
	}

	// add quality
	DataModel::OriginQuality oq;
	oq.setAssociatedPhaseCount(origin->arrivalCount());
	oq.setUsedPhaseCount(usedPhaseCount);
	oq.setAssociatedPhaseCount(associatedPhaseCount);
	oq.setAssociatedStationCount(associatedStations.size());
	oq.setUsedStationCount(usedStations.size());
	oq.setStandardError(rms);
	oq.setMedianDistance(computeMedian(distances));
	oq.setMinimumDistance(*min_element(distances.begin(), distances.end()));
	oq.setMaximumDistance(*max_element(distances.begin(), distances.end()));
	oq.setAzimuthalGap(primaryAz);
	oq.setSecondaryAzimuthalGap(secondaryAz);
	origin->setQuality(oq);

	//
	// The Confidence Ellipsoid code has been adapted from LOCSAT locator
	//
	if ( _currentProfile.enableConfidenceEllipsoid ) {
	
		if ( ! covm.valid ) {
			SEISCOMP_DEBUG("No valid covariance matrix. No Confidence ellipsoid will be computed");
		} else {

		// M4d is the 4D matrix with X axis as S-N and Y axis as W-E
		double M4d[16] = {
			covm.syy, covm.sxy, covm.syz, covm.syt,
			covm.sxy, covm.sxx, covm.sxz, covm.sxt,
			covm.syz, covm.sxz, covm.szz, covm.szt,
			covm.syt, covm.sxt, covm.szt, covm.stt
		};

		// M3d is the matrix in space
		double M3d[9] = {
			M4d[0], M4d[1], M4d[2],
			M4d[4], M4d[5], M4d[6],
			M4d[8], M4d[9], M4d[10]
		};

		// M2d is the matrix in the XY plane
		double M2d[4] = {
			M4d[0], M4d[1],
			M4d[4], M4d[5]
		};

		// Diagonalize 3D and 2D matrixes
		// We use EISPACK code

		// compute 3D and 2D eigenvalues, eigenvectors
		// EISPACK sort eigenvalues from min to max
		int ierr3;
		double eigvec3d[9];
		double eigval3d[3];
		ierr3 = rs(3, M3d, eigval3d, eigvec3d);

		int ierr2;
		double eigvec2d[4];
		double eigval2d[2];
		ierr2 = rs(2, M2d, eigval2d, eigvec2d);

		/*
		* Confidence coefficients for 1D, 2D and 3D
		* 
		* We use ASA091 code
		*
		* The following table summarizes confidence coefficients for 0.90 (LocSAT) and 0.68 (NonLinLoc) confidence levels
		*
		* confidenceLevel      1D             2D              3D
		*      0.90        1.6448536270    2.1459660263    2.5002777108
		*      0.68        0.9944578832    1.5095921855    1.8724001591
		*
		*
		*/

		if ( ierr3 == 0 && ierr2 == 0 ) {
			double kppf[3];
			double g;
			int ifault;
			double dof;

			for ( int i = 0; i < 3; ++i ) {
				dof = i + 1;
				g = alngam(dof/2.0, &ifault);
				kppf[i] = pow(ppchi2(_currentProfile.confLevel, dof, g, &ifault), 0.5);
			}

			double sx, sy, smajax, sminax, strike;

			// 1D confidence intervals
			sx = kppf[0] * pow(M4d[0], 0.5); // sxx
			sy = kppf[0] * pow(M4d[5], 0.5); // syy

			// 1D confidence intervals
			origin->setTime(DataModel::TimeQuantity(originTime, sqrt(covm.stt) * kppf[0], Core::None, Core::None, _currentProfile.confLevel * 100.0));
			origin->setLatitude(DataModel::RealQuantity(originLat, sqrt(covm.syy) * kppf[0], Core::None, Core::None, _currentProfile.confLevel * 100.0));
			origin->setLongitude(DataModel::RealQuantity(originLon, sqrt(covm.sxx) * kppf[0], Core::None, Core::None, _currentProfile.confLevel * 100.0));
			origin->setDepth(DataModel::RealQuantity(originDepth, sqrt(covm.szz) * kppf[0], Core::None, Core::None, _currentProfile.confLevel * 100.0));

			// 2D confidence intervals
			sminax = kppf[1] * pow(eigval2d[0], 0.5);
			smajax = kppf[1] * pow(eigval2d[1], 0.5);
			strike = rad2deg(atan(eigvec2d[3] / eigvec2d[2]));
			// give the strike in the [0.0, 180.0] interval
			if ( strike < 0.0 )
				strike += 180.0;

			if ( strike > 180.0 )
				strike -= 180.0;

			// 3D confidence intervals
			double s3dMajAxis, s3dMinAxis, s3dIntAxis, MajAxisPlunge, MajAxisAzimuth, MajAxisRotation;

			s3dMinAxis = kppf[2] * pow(eigval3d[0], 0.5);
			s3dIntAxis = kppf[2] * pow(eigval3d[1], 0.5);
			s3dMajAxis = kppf[2] * pow(eigval3d[2], 0.5);

			MajAxisPlunge   = rad2deg(atan(eigvec3d[8] / pow(pow(eigvec3d[6], 2.0) + pow(eigvec3d[7], 2.0), 0.5)));
			if ( MajAxisPlunge < 0.0 )
				MajAxisPlunge += 180.0;

			if ( MajAxisPlunge > 180.0 )
				MajAxisPlunge -= 180.0;

			MajAxisAzimuth  = rad2deg(atan(eigvec3d[7] / eigvec3d[6]));
			if ( MajAxisAzimuth < 0.0 )
				MajAxisAzimuth += 180.0;

			if ( MajAxisAzimuth > 180.0 )
				MajAxisAzimuth -= 180.0;

			MajAxisRotation = rad2deg(atan(eigvec3d[2] / pow(pow(eigvec3d[0], 2.0) + pow(eigvec3d[1], 2.0), 0.5)));
			if ( covm.szz == 0.0 )
				MajAxisRotation = 0.0;

			if ( MajAxisRotation < 0.0 )
				MajAxisRotation += 180.0;

			if ( MajAxisRotation > 180.0 )
				MajAxisRotation -= 180.0;

			DataModel::ConfidenceEllipsoid confidenceEllipsoid;
			DataModel::OriginUncertainty originUncertainty;

			confidenceEllipsoid.setSemiMinorAxisLength(s3dMinAxis*1000.0);
			confidenceEllipsoid.setSemiIntermediateAxisLength(s3dIntAxis*1000.0);
			confidenceEllipsoid.setSemiMajorAxisLength(s3dMajAxis*1000.0);
			confidenceEllipsoid.setMajorAxisPlunge(MajAxisPlunge);
			confidenceEllipsoid.setMajorAxisAzimuth(MajAxisAzimuth);
			confidenceEllipsoid.setMajorAxisRotation(MajAxisRotation);

			// QuakeML, horizontalUncertainty: Circular confidence region, given by single value of horizontal uncertainty.
			// Acordingly, 1D horizontal errors quadratic mean is given
			originUncertainty.setHorizontalUncertainty(sqrt(pow(sx, 2) + pow(sy, 2)));
			originUncertainty.setMinHorizontalUncertainty(sminax);
			originUncertainty.setMaxHorizontalUncertainty(smajax);
			originUncertainty.setAzimuthMaxHorizontalUncertainty(strike);
			originUncertainty.setConfidenceEllipsoid(confidenceEllipsoid);
			originUncertainty.setPreferredDescription(Seiscomp::DataModel::OriginUncertaintyDescription(Seiscomp::DataModel::ELLIPSOID));

			origin->setUncertainty(originUncertainty);

		}
		else {
			SEISCOMP_DEBUG("Unable to calculate eigenvalues/eigenvectors. No Confidence ellipsoid will be computed");
		}
		}
	}

	return origin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace
