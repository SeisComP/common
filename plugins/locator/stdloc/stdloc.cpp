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
#include <tuple>
#include <set>

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
	if ( tmp.size() % 2 == 0 ) {
		const auto leftMiddleItr = max_element(tmp.begin(), middleItr);
		median = (*leftMiddleItr + *middleItr) / 2;
	}
	return median;
}


void computeCoordinates(double distance, double azimuth, double clat,
                        double clon, double &lat, double &lon) {
	Math::Geo::delandaz2coord(Math::Geo::km2deg(distance), azimuth, clat, clon,
	                          &lat, &lon);
	lon = Geo::GeoCoordinate::normalizeLon(lon);
}


double computeDistance(double lat1, double lon1, double lat2, double lon2,
                       double *azimuth = nullptr,
                       double *backAzimuth = nullptr) {
	double dist;
	Math::Geo::delazi(lat1, lon1, lat2, lon2, &dist, azimuth, backAzimuth);
	return dist;
}


double computePickWeight(DataModel::Pick *pick,
                         const vector<double> &uncertaintyClasses) {

	double uncertainty = -1; // secs
	try {
		// symmetric uncertainty
		uncertainty = pick->time().uncertainty();
	}
	catch ( Core::ValueException & ) {
		// asymmetric uncertainty
		try {
			uncertainty = (pick->time().lowerUncertainty() +
			               pick->time().upperUncertainty()) /
			              2.0;
		}
		catch ( Core::ValueException & ) {
		}
	}

	// set lowest class as default
	unsigned uncertaintyClass = uncertaintyClasses.size() - 1;

	if ( uncertainty >= 0 && isfinite(uncertainty) &&
	     uncertaintyClasses.size() > 1 &&
	     uncertainty < uncertaintyClasses.back() ) {

		for ( unsigned curr = 0, next = 1; next < uncertaintyClasses.size();
		      curr++, next++ ) {
			if ( uncertainty >= uncertaintyClasses.at(curr) &&
			     uncertainty <= uncertaintyClasses.at(next) ) {
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

	if ( det == 0 ) {
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

} // namespace
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


namespace { // StdLoc implementation 


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ADD_SC_PLUGIN(
	"Standard Locator",
	"Luca Scarabello, ETH Zurich",
	1, 0, 0
)
REGISTER_LOCATOR(StdLoc, "StdLoc");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StdLoc::capabilities() const {
	return InitialLocation | FixedDepth;
}
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
    "GridSearch.size",
    "GridSearch.numPoints",
    "GridSearch.misfitType",
    "GridSearch.travelTimeError",
    "OctTree.maxIterations",
    "OctTree.minCellSize",
    "LeastSquares.depthInit",
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
	defaultProf.method = Profile::Method::LeastSquares;
	defaultProf.tttType = "LOCSAT";
	defaultProf.tttModel = "iasp91";
	defaultProf.PSTableOnly = true;
	defaultProf.usePickUncertainties = false;
	defaultProf.pickUncertaintyClasses = {0.000, 0.025, 0.050,
	                                      0.100, 0.200, 0.400};
	defaultProf.enableConfidenceEllipsoid = true;
	defaultProf.confLevel = 0.9;
	defaultProf.gridSearch.originLat = 0.;
	defaultProf.gridSearch.originLon = 0.;
	defaultProf.gridSearch.originDepth = 20.;
	defaultProf.gridSearch.autoOriginLon   = true;
	defaultProf.gridSearch.autoOriginLat   = true;
	defaultProf.gridSearch.autoOriginDepth = false;
	defaultProf.gridSearch.xExtent = 40.;
	defaultProf.gridSearch.yExtent = 40.;
	defaultProf.gridSearch.zExtent = 30.;
	defaultProf.gridSearch.numXPoints = 0;
	defaultProf.gridSearch.numYPoints = 0;
	defaultProf.gridSearch.numZPoints = 0;
	defaultProf.gridSearch.misfitType = "L1";
	defaultProf.gridSearch.travelTimeError = 0.25;
	defaultProf.octTree.maxIterations = 50000;
	defaultProf.octTree.minCellSize = 0.1;
	defaultProf.leastSquares.depthInit = 20.;
	defaultProf.leastSquares.iterations = 20;
	defaultProf.leastSquares.dampingFactor = 0;
	defaultProf.leastSquares.solverType = "LSMR";

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
			else if ( method == "OctTree" ) {
				prof.method = Profile::Method::OctTree;
			}
			else if ( method == "GridSearch+LeastSquares" ) {
				prof.method = Profile::Method::GridAndLsqr;
			}
			else if ( method == "OctTree+LeastSquares" ) {
				prof.method = Profile::Method::OctTreeAndLsqr;
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
			prof.usePickUncertainties =
			    config.getBool(prefix + "usePickUncertainties");
		}
		catch ( ... ) {}

		try {
			vector<string> tokens =
			    config.getStrings(prefix + "pickUncertaintyClasses");
			if ( tokens.size() < 2 ) {
				SEISCOMP_ERROR("Profile %s: pickUncertaintyClasses should "
				               "contain at least "
				               "2 values",
				               prof.name.c_str());
				return false;
			}
			prof.pickUncertaintyClasses.clear();
			for ( const string &tok : tokens ) {
				double time;
				if ( !Core::fromString(time, tok) ) {
					SEISCOMP_ERROR(
					    "Profile %s: pickUncertaintyClasses is invalid",
					    prof.name.c_str());
					return false;
				}
				prof.pickUncertaintyClasses.push_back(time);
			}
		}
		catch ( ... ) {}

		try {
			prof.enableConfidenceEllipsoid =
			    config.getBool(prefix + "enableConfidenceEllipsoid");
		}
		catch ( ... ) {}

		try {
			prof.confLevel = config.getDouble(prefix + "confLevel");
		}
		catch ( ... ) {}

		try {
			vector<string> tokens =
			    config.getStrings(prefix + "GridSearch.center");
			if ( tokens.size() != 3 ) {
				SEISCOMP_ERROR("Profile %s: GridSearch.center is invalid",
				               prof.name.c_str());
				return false;
			}

			if ( tokens.at(0) == "auto" ) {
				prof.gridSearch.originLat = 0.0;
				prof.gridSearch.autoOriginLat = true;
			}
			else if ( Core::fromString(prof.gridSearch.originLat, tokens.at(0)) ) {
				prof.gridSearch.autoOriginLat = false;
			}
			else {
				SEISCOMP_ERROR("Profile %s: GridSearch.center lat is invalid",
				               prof.name.c_str());
				return false;
			}

			if ( tokens.at(1) == "auto" ) {
				prof.gridSearch.originLon = 0.0;
				prof.gridSearch.autoOriginLon = true;
			}
			else if ( Core::fromString(prof.gridSearch.originLon, tokens.at(1)) ) {
				prof.gridSearch.autoOriginLon = false;
			}
			else {
				SEISCOMP_ERROR("Profile %s: GridSearch.center lon is invalid",
				               prof.name.c_str());
				return false;
			}

			if ( tokens.at(2) == "auto" ) {
				prof.gridSearch.originDepth = 0.0;
				prof.gridSearch.autoOriginDepth = true;
			}
			else if ( Core::fromString(prof.gridSearch.originDepth, tokens.at(2)) ) {
				prof.gridSearch.autoOriginDepth = false;
			}
			else {
				SEISCOMP_ERROR("Profile %s: GridSearch.center depth is invalid",
				               prof.name.c_str());
				return false;
			}

		}
		catch ( ... ) {}

		try {
			vector<string> tokens =
			    config.getStrings(prefix + "GridSearch.size");
			if ( tokens.size() != 3 ||
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
			vector<string> tokens =
			    config.getStrings(prefix + "GridSearch.numPoints");
			if ( tokens.size() != 3 ||
			     !Core::fromString(prof.gridSearch.numXPoints, tokens.at(0)) ||
			     !Core::fromString(prof.gridSearch.numYPoints, tokens.at(1)) ||
			     !Core::fromString(prof.gridSearch.numZPoints,
			                       tokens.at(2)) ) {
				SEISCOMP_ERROR("Profile %s: GridSearch.numPoints is invalid",
				               prof.name.c_str());
				return false;
			}
		}
		catch ( ... ) {}

		try {
			prof.gridSearch.misfitType =
			    config.getString(prefix + "GridSearch.misfitType");
			if ( prof.gridSearch.misfitType != "L1" &&
			     prof.gridSearch.misfitType != "L2" ) {
				SEISCOMP_ERROR("Profile %s: GridSearch.misfitType is invalid",
				               prof.name.c_str());
				return false;
			}
		}
		catch ( ... ) {}

		try {
			prof.gridSearch.travelTimeError =
			    config.getDouble(prefix + "GridSearch.travelTimeError");
		}
		catch ( ... ) {}

		try {
			prof.octTree.maxIterations =
			    config.getInt(prefix + "OctTree.maxIterations");
		}
		catch ( ... ) {}

		try {
			prof.octTree.minCellSize =
			    config.getDouble(prefix + "OctTree.minCellSize");
		}
		catch ( ... ) {}

		try {
			prof.leastSquares.depthInit =
			    config.getDouble(prefix + "LeastSquares.depthInit");
		}
		catch ( ... ) {}

		try {
			prof.leastSquares.iterations =
			    config.getInt(prefix + "LeastSquares.iterations");
		}
		catch ( ... ) {}

		try {
			prof.leastSquares.dampingFactor =
			    config.getDouble(prefix + "LeastSquares.dampingFactor");
		}
		catch ( ... ) {}

		try {
			prof.leastSquares.solverType =
			    config.getString(prefix + "LeastSquares.solverType");
			if ( prof.leastSquares.solverType != "LSMR" &&
			     prof.leastSquares.solverType != "LSQR" ) {
				SEISCOMP_ERROR("Profile %s: leastSquares.solverType is invalid",
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
	transform(begin(_profiles), end(_profiles), back_inserter(keys),
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
		else if ( _currentProfile.method == Profile::Method::GridSearch ) {
			return "GridSearch";
		}
		else if ( _currentProfile.method == Profile::Method::OctTree ) {
			return "OctTree";
		}
		else if ( _currentProfile.method == Profile::Method::GridAndLsqr ) {
			return "GridSearch+LeastSquares";
		}
		else if ( _currentProfile.method == Profile::Method::OctTreeAndLsqr ) {
			return "OctTree+LeastSquares";
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
		for ( double time : _currentProfile.pickUncertaintyClasses ) {
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
	else if ( name == "LeastSquares.depthInit" ) {
		return Core::toString(_currentProfile.leastSquares.depthInit);
	}
	else if ( name == "LeastSquares.iterations" ) {
		return Core::toString(_currentProfile.leastSquares.iterations);
	}
	else if ( name == "LeastSquares.dampingFactor" ) {
		return Core::toString(_currentProfile.leastSquares.dampingFactor);
	}
	else if ( name == "LeastSquares.solverType" ) {
		return _currentProfile.leastSquares.solverType;
	}
	else if ( name == "GridSearch.center" ) {
		string lat = _currentProfile.gridSearch.autoOriginLat ? "auto"
		           : Core::toString(_currentProfile.gridSearch.originLat);
		string lon = _currentProfile.gridSearch.autoOriginLon ? "auto"
		           : Core::toString(_currentProfile.gridSearch.originLon);
		string dep = _currentProfile.gridSearch.autoOriginDepth ? "auto"
		           : Core::toString(_currentProfile.gridSearch.originDepth);
		return lat + "," + lon + "," + dep;
	}
	else if ( name == "GridSearch.size" ) {
		return Core::toString(_currentProfile.gridSearch.xExtent) + "," +
		       Core::toString(_currentProfile.gridSearch.yExtent) + "," +
		       Core::toString(_currentProfile.gridSearch.zExtent);
	}
	else if ( name == "GridSearch.numPoints" ) {
		return Core::toString(_currentProfile.gridSearch.numXPoints) + "," +
		       Core::toString(_currentProfile.gridSearch.numYPoints) + "," +
		       Core::toString(_currentProfile.gridSearch.numZPoints);
	}
	else if ( name == "GridSearch.misfitType" ) {
		return _currentProfile.gridSearch.misfitType;
	}
	else if ( name == "GridSearch.travelTimeError" ) {
		return Core::toString(_currentProfile.gridSearch.travelTimeError);
	}
	else if ( name == "OctTree.maxIterations" ) {
		return Core::toString(_currentProfile.octTree.maxIterations);
	}
	else if ( name == "OctTree.minCellSize" ) {
		return Core::toString(_currentProfile.octTree.minCellSize);
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
		else if ( value == "OctTree" ) {
			_currentProfile.method = Profile::Method::OctTree;
			return true;
		}
		else if ( value == "GridSearch+LeastSquares" ) {
			_currentProfile.method = Profile::Method::GridAndLsqr;
			return true;
		}
		else if ( value == "OctTree+LeastSquares" ) {
			_currentProfile.method = Profile::Method::OctTreeAndLsqr;
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
			SEISCOMP_ERROR(
			    "Profile %s: pickUncertaintyClasses should contain at least "
			    "2 values",
			    _currentProfile.name.c_str());
			return false;
		}
		_currentProfile.pickUncertaintyClasses.clear();
		for ( const string &tok : tokens ) {
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
	else if ( name == "LeastSquares.depthInit" ) {
		double tmp;
		if ( !Core::fromString(tmp, value) ) {
			return false;
		}
		_currentProfile.leastSquares.depthInit = tmp;
		return true;
	}
	else if ( name == "LeastSquares.iterations" ) {
		int tmp;
		if ( !Core::fromString(tmp, value) ) {
			return false;
		}
		_currentProfile.leastSquares.iterations = tmp;
		return true;
	}
	else if ( name == "LeastSquares.dampingFactor" ) {
		double tmp;
		if ( !Core::fromString(tmp, value) ) {
			return false;
		}
		_currentProfile.leastSquares.dampingFactor = tmp;
		return true;
	}
	else if ( name == "LeastSquares.solverType" ) {
		if ( value != "LSMR" && value != "LSQR" ) {
			return false;
		}
		_currentProfile.leastSquares.solverType = value;
		return true;
	}
	else if ( name == "GridSearch.center" ) {
		vector<string> tokens = splitString(value);
		if ( tokens.size() != 3 ) {
			return false;
		}

		if ( tokens.at(0) == "auto" ) {
			_currentProfile.gridSearch.originLat = 0.0;
			_currentProfile.gridSearch.autoOriginLat = true;
		}
		else if ( Core::fromString(_currentProfile.gridSearch.originLat, tokens.at(0)) ) {
			_currentProfile.gridSearch.autoOriginLat = false;
		}
		else {
			return false;
		}

		if ( tokens.at(1) == "auto" ) {
			_currentProfile.gridSearch.originLon = 0.0;
			_currentProfile.gridSearch.autoOriginLon = true;
		}
		else if ( Core::fromString(_currentProfile.gridSearch.originLon, tokens.at(1)) ) {
			_currentProfile.gridSearch.autoOriginLon = false;
		}
		else {
			return false;
		}

		if ( tokens.at(2) == "auto" ) {
			_currentProfile.gridSearch.originDepth = 0.0;
			_currentProfile.gridSearch.autoOriginDepth = true;
		}
		else if ( Core::fromString(_currentProfile.gridSearch.originDepth, tokens.at(2)) ) {
			_currentProfile.gridSearch.autoOriginDepth = false;
		}
		else {
			return false;
		}

		return true;
	}
	else if ( name == "GridSearch.size" ) {
		vector<string> tokens = splitString(value);
		if ( tokens.size() != 3 ||
		     !Core::fromString(_currentProfile.gridSearch.xExtent,
		                       tokens.at(0)) ||
		     !Core::fromString(_currentProfile.gridSearch.yExtent,
		                       tokens.at(1)) ||
		     !Core::fromString(_currentProfile.gridSearch.zExtent,
		                       tokens.at(2)) ) {
			return false;
		}
		return true;
	}
	else if ( name == "GridSearch.numPoints" ) {
		vector<string> tokens = splitString(value);
		if ( tokens.size() != 3 ||
		     !Core::fromString(_currentProfile.gridSearch.numXPoints,
		                       tokens.at(0)) ||
		     !Core::fromString(_currentProfile.gridSearch.numYPoints,
		                       tokens.at(1)) ||
		     !Core::fromString(_currentProfile.gridSearch.numZPoints,
		                       tokens.at(2)) ) {
			return false;
		}
		return true;
	}
	else if ( name == "GridSearch.misfitType" ) {
		if ( value != "L1" && value != "L2" ) {
			return false;
		}
		_currentProfile.gridSearch.misfitType = value;
		return true;
	}
	else if ( name == "GridSearch.travelTimeError" ) {
		double tmp;
		if ( !Core::fromString(tmp, value) ) {
			return false;
		}
		_currentProfile.gridSearch.travelTimeError = tmp;
	}
	else if ( name == "OctTree.maxIterations" ) {
		int tmp;
		if ( !Core::fromString(tmp, value) ) {
			return false;
		}
		_currentProfile.octTree.maxIterations = tmp;
	}
	else if ( name == "OctTree.minCellSize" ) {
		double tmp;
		if ( !Core::fromString(tmp, value) ) {
			return false;
		}
		_currentProfile.octTree.minCellSize = tmp;
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
std::string StdLoc::lastMessage(MessageType type) const {
	if ( type == Warning )
		return _rejectionMsg;

	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin *StdLoc::locate(PickList &pickList) {
	SEISCOMP_DEBUG("Locating Origin using PickList with profile '%s'",
	               _currentProfile.name.c_str());

	_rejectLocation = false;
	_rejectionMsg = "";

	loadTTT();

	vector<double> weights, sensorLat, sensorLon, sensorElev;
	computeAdditionlPickInfo(pickList, weights, sensorLat, sensorLon,
	                         sensorElev);

	// these are the output of the location
	double originLat, originLon, originDepth;
	Core::Time originTime;
	vector<double> travelTimes;
	CovMtrx covm;

	bool computeCovMtrx = _currentProfile.enableConfidenceEllipsoid;

	if ( _currentProfile.method == Profile::Method::GridSearch ||
	     _currentProfile.method == Profile::Method::GridAndLsqr ) {
		bool enablePerCellLeastSquares =
		    _currentProfile.method == Profile::Method::GridAndLsqr;
		locateGridSearch(pickList, weights, sensorLat, sensorLon, sensorElev,
		                 originLat, originLon, originDepth, originTime,
		                 travelTimes, covm, computeCovMtrx,
		                 enablePerCellLeastSquares);
	}
	else if ( _currentProfile.method == Profile::Method::OctTree ||
	          _currentProfile.method == Profile::Method::OctTreeAndLsqr ) {
		locateOctTree(pickList, weights, sensorLat, sensorLon, sensorElev,
		              originLat, originLon, originDepth, originTime,
		              travelTimes, covm,
		              (computeCovMtrx &&
		               _currentProfile.method == Profile::Method::OctTree));
		if ( _currentProfile.method == Profile::Method::OctTreeAndLsqr ) {
			locateLeastSquares(pickList, weights, sensorLat, sensorLon,
			                   sensorElev, originLat, originLon, originDepth,
			                   originTime, originLat, originLon, originDepth,
			                   originTime, travelTimes, covm, computeCovMtrx);
		}
	}
	else if ( _currentProfile.method == Profile::Method::LeastSquares ) {
		locateLeastSquares(pickList, weights, sensorLat, sensorLon, sensorElev,
		                   originLat, originLon, originDepth, originTime,
		                   travelTimes, covm, computeCovMtrx);
	}

	return createOrigin(pickList, weights, sensorLat, sensorLon, sensorElev,
	                    travelTimes, originLat, originLon, originDepth,
	                    originTime, covm);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Origin *StdLoc::locate(PickList &pickList, double initLat, double initLon,
                       double initDepth, const Core::Time &initTime) {

	_rejectLocation = false;
	_rejectionMsg = "";

	loadTTT();

	SEISCOMP_DEBUG(
	    "Locating Origin using PickList and an initial location using "
	    "profile '%s'",
	    _currentProfile.name.c_str());

	vector<double> weights, sensorLat, sensorLon, sensorElev;
	computeAdditionlPickInfo(pickList, weights, sensorLat, sensorLon,
	                         sensorElev);

	// these are the output of the location
	double originLat, originLon, originDepth;
	Core::Time originTime;
	vector<double> travelTimes;
	CovMtrx covm;

	bool computeCovMtrx = _currentProfile.enableConfidenceEllipsoid;

	if ( _currentProfile.method == Profile::Method::GridSearch ||
	     _currentProfile.method == Profile::Method::GridAndLsqr ) {
		bool enablePerCellLeastSquares =
		    _currentProfile.method == Profile::Method::GridAndLsqr;
		locateGridSearch(pickList, weights, sensorLat, sensorLon, sensorElev,
		                 originLat, originLon, originDepth, originTime,
		                 travelTimes, covm, computeCovMtrx,
		                 enablePerCellLeastSquares);
	}
	else if ( _currentProfile.method == Profile::Method::OctTree ||
	          _currentProfile.method == Profile::Method::OctTreeAndLsqr ) {
		locateOctTree(pickList, weights, sensorLat, sensorLon, sensorElev,
		              originLat, originLon, originDepth, originTime,
		              travelTimes, covm,
		              (computeCovMtrx &&
		               _currentProfile.method == Profile::Method::OctTree));
		if ( _currentProfile.method == Profile::Method::OctTreeAndLsqr ) {
			locateLeastSquares(pickList, weights, sensorLat, sensorLon,
			                   sensorElev, originLat, originLon, originDepth,
			                   originTime, originLat, originLon, originDepth,
			                   originTime, travelTimes, covm, computeCovMtrx);
		}
	}
	else if ( _currentProfile.method == Profile::Method::LeastSquares ) {
		locateLeastSquares(pickList, weights, sensorLat, sensorLon, sensorElev,
		                   initLat, initLon, initDepth, initTime, originLat,
		                   originLon, originDepth, originTime, travelTimes,
		                   covm, computeCovMtrx);
	}

	return createOrigin(pickList, weights, sensorLat, sensorLon, sensorElev,
	                    travelTimes, originLat, originLon, originDepth,
	                    originTime, covm);
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
			throw PickNotFoundException(
			    "pick '" + origin->arrival(i)->pickID() + "' not found");
		}

		try {
			// Phase definition of arrival and pick different?
			if ( pick->phaseHint().code() !=
			     origin->arrival(i)->phase().code() ) {
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
			throw LocatorException("sensor location '" +
			                       pick->waveformID().networkCode() + "." +
			                       pick->waveformID().stationCode() + "." +
			                       pick->waveformID().locationCode() +
			                       "' is incomplete w.r.t. lat/lon");
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

		if ( _currentProfile.usePickUncertainties ) {
			weights[i] = computePickWeight(
			    pick.get(), _currentProfile.pickUncertaintyClasses);
		}
		++activeArrivals;
	}

	if ( activeArrivals <= 0 ) {
		throw LocatorException("Empty set of active arrivals");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::computeProbDensity(const PickList &pickList,
                                const vector<double> &weights,
                                const vector<double> &travelTimes,
                                const Core::Time &originTime,
                                double &probDensity, double &rms) const {

	if ( _currentProfile.gridSearch.misfitType != "L1" &&
	     _currentProfile.gridSearch.misfitType != "L2" ) {
		throw LocatorException("The error type can only be L1 or "
		                       "L2, but it is set to " +
		                       _currentProfile.gridSearch.misfitType);
	}

	if ( weights.size() != pickList.size() ||
	     travelTimes.size() != pickList.size() ) {
		throw LocatorException("Interna logic error");
	}

	rms = 0.0;
	double sigma = _currentProfile.gridSearch.travelTimeError;

	double l1SumWeightedResiduals = 0.0;
	double l2SumWeightedResiduals = 0.0;
	double weightedSigma = 0.0;
	double weightedSquaredSigma = 0.0;
	double sumWeights = 0.0;
	double sumSquaredWeights = 0.0;

	for ( size_t i = 0; i < pickList.size(); ++i ) {
		const PickItem &pi = pickList[i];
		const PickPtr pick = pi.pick;

		if ( weights[i] <= 0 || travelTimes[i] < 0 ) {
			continue;
		}

		Core::Time pickTime = pick->time().value();
		double residual =
		    (pickTime - (originTime + Core::TimeSpan(travelTimes[i]))).length();
		l1SumWeightedResiduals += abs(residual * weights[i]);
		l2SumWeightedResiduals +=
		    (residual * weights[i]) * (residual * weights[i]);
		weightedSigma += sigma * weights[i];
		weightedSquaredSigma += (sigma * weights[i]) * (sigma * weights[i]);
		sumWeights += weights[i];
		sumSquaredWeights += weights[i] * weights[i];
	}

	if ( sumSquaredWeights == 0 ) {
		throw LocatorException("Cannot compute probability density without "
		                       "valid picks and/or travel times");
	}

	weightedSigma /= sumWeights;
	weightedSquaredSigma /= sumSquaredWeights;
	rms = sqrt(l2SumWeightedResiduals / sumSquaredWeights);

	//
	// Compute the non-normalized probability density (likelihood function)
	//
	if ( _currentProfile.gridSearch.misfitType == "L1" ) {
		probDensity = -1.0 * l1SumWeightedResiduals / weightedSigma;
	}
	else if ( _currentProfile.gridSearch.misfitType == "L2" ) {
		probDensity = -0.5 * l2SumWeightedResiduals / weightedSquaredSigma;
	}
	//
	// Note that we actually return the natural log of the likelihood function
	// to avoid severe precision loss (that is we do not compute std::exp).
	// std::exp would returns 0 for the vast majoriy of cells, making the
	// comparison of the likelihood between cells impossible
	//
	// probDensity = std::exp(probDensity);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StdLoc::computeOriginTime(const PickList &pickList,
                               const vector<double> &weights,
                               const vector<double> &sensorLat,
                               const vector<double> &sensorLon,
                               const vector<double> &sensorElev, double lat,
                               double lon, double depth, Core::Time &originTime,
                               vector<double> &travelTimes) const {

	if ( weights.size() != pickList.size() ||
	     sensorLat.size() != pickList.size() ||
	     sensorLon.size() != pickList.size() ||
	     sensorElev.size() != pickList.size() ) {
		throw LocatorException("Interna logic error");
	}

	travelTimes.resize(pickList.size());

	vector<double> originTimes;
	vector<double> timeWeights;

	for ( size_t i = 0; i < pickList.size(); ++i ) {
		const PickItem &pi = pickList[i];
		const PickPtr pick = pi.pick;

		travelTimes[i] = -1.0;

		if ( weights[i] <= 0 ) {
			continue;
		}

		double ttime;

		try {
			const char *phaseName = pick->phaseHint().code().c_str();
			if ( _currentProfile.PSTableOnly ) {
				if ( *pick->phaseHint().code().begin() == 'P' ) {
					phaseName = "P";
				}
				else if ( *pick->phaseHint().code().begin() == 'S' ) {
					phaseName = "S";
				}
			}
			ttime = _ttt->computeTime(phaseName, lat, lon, depth, sensorLat[i],
			                          sensorLon[i], sensorElev[i]);
		}
		catch ( exception &e ) {
			SEISCOMP_WARNING("Travel Time Table error for %s@%s.%s.%s and lat "
			                 "%g lon %g depth %g: %s",
			                 pick->phaseHint().code().c_str(),
			                 pick->waveformID().networkCode().c_str(),
			                 pick->waveformID().stationCode().c_str(),
			                 pick->waveformID().locationCode().c_str(), lat,
			                 lon, depth, e.what());
			continue;
		}

		if ( ttime < 0 ) {
			SEISCOMP_WARNING("Travel Time Table error: data not returned for "
			                 "%s@%s.%s.%s and lat %g lon %g depth %g",
			                 pick->phaseHint().code().c_str(),
			                 pick->waveformID().networkCode().c_str(),
			                 pick->waveformID().stationCode().c_str(),
			                 pick->waveformID().locationCode().c_str(), lat,
			                 lon, depth);
			continue;
		}

		travelTimes[i] = ttime;
		double pickTime = double(pick->time().value());
		originTimes.push_back(pickTime - travelTimes[i]);
		timeWeights.push_back(weights[i]);
	}

	if ( originTimes.size() == 0 ) {
		SEISCOMP_DEBUG("Unable to compute origin time: no valid picks and/or "
		               "travel times");
		return false;
	}

	// Compute origin time
	double orgTime, orgTimeError;
	Math::Statistics::average(originTimes, timeWeights, orgTime, orgTimeError);
	originTime = Core::Time(orgTime);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::locateOctTree(const PickList &pickList,
                           const vector<double> &weights,
                           const vector<double> &sensorLat,
                           const vector<double> &sensorLon,
                           const vector<double> &sensorElev,
                           double &newLat,double &newLon, double &newDepth,
                           Core::Time &newTime, vector<double> &travelTimes,
                           CovMtrx &covm, bool computeCovMtrx) {
	SEISCOMP_DEBUG("Start OctTree Search: maxIterations %d minCellSize %g [km]",
	               _currentProfile.octTree.maxIterations,
	               _currentProfile.octTree.minCellSize);

	if ( !_ttt ) {
		throw LocatorException(
		    "Travel time table has not been loaded, check logs");
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

	if ( _currentProfile.octTree.maxIterations <= 0 &&
	     _currentProfile.octTree.minCellSize <= 0 ) {
		throw LocatorException(
		    "Either octTree.maxIterations or octTree.minCellSize must be used");
	}
	
	if ( _currentProfile.gridSearch.numXPoints < 2 ||
	     _currentProfile.gridSearch.numYPoints < 2 ||
	     _currentProfile.gridSearch.numZPoints < 2 ) {
		throw LocatorException("At least 2 points per dimension should be given");
	}

	covm.valid = false;

	double xExtent = _currentProfile.gridSearch.xExtent;
	double yExtent = _currentProfile.gridSearch.yExtent;
	double zExtent = _currentProfile.gridSearch.zExtent;
	double gridOriginLat = _currentProfile.gridSearch.originLat;
	double gridOriginLon = _currentProfile.gridSearch.originLon;
	double gridOriginDepth = _currentProfile.gridSearch.originDepth;

	double cellXExtent = xExtent / (_currentProfile.gridSearch.numXPoints -1);
	double cellYExtent = yExtent / (_currentProfile.gridSearch.numYPoints -1);
	double cellZExtent = zExtent / (_currentProfile.gridSearch.numZPoints -1);

	SEISCOMP_DEBUG("OctTree cell size X x Y x Z = %gx%gx%g [km]",
	               cellXExtent, cellYExtent, cellZExtent);

	//
	// Auto-position the grid center
	//
	if ( _currentProfile.gridSearch.autoOriginLat ) {
		gridOriginLat = computeMean(sensorLat);
		SEISCOMP_DEBUG("GridSearch.center auto latitude %g", gridOriginLat);
	}

	if ( _currentProfile.gridSearch.autoOriginLon ) {
		gridOriginLon = Geo::GeoCoordinate::normalizeLon(
		    computeCircularMean(sensorLon, false));
		SEISCOMP_DEBUG("GridSearch.center auto longitude %g", gridOriginLon);
	}

	if ( _currentProfile.gridSearch.autoOriginDepth ) {
		gridOriginDepth = -computeMean(sensorElev)/1000.;
		SEISCOMP_DEBUG("GridSearch.center auto depth %g", gridOriginDepth);
	}

	// fix depth
	if ( usingFixedDepth() ) {
		gridOriginDepth = fixedDepth();
		zExtent = cellZExtent;
		SEISCOMP_DEBUG("Using fixed depth %g [km]", gridOriginDepth);
	}

	vector<Cell> unknownPriorityList;

	//
	// Start off with the initial grid cells
	//
	for ( double x = -xExtent / 2. + cellXExtent / 2.;
	             x < xExtent / 2.;
	             x += cellXExtent ) {
		for ( double y = -yExtent / 2. + cellYExtent / 2;
		             y < yExtent / 2.;
		             y += cellYExtent ) {
			for ( double z = -zExtent / 2. + cellZExtent / 2;
			             z < zExtent / 2.;
			             z += cellZExtent ) {
				Cell cell;
				cell.valid = false;
				cell.x = x;
				cell.y = y;
				cell.z = z;
				cell.size.x = cellXExtent;
				cell.size.y = cellYExtent;
				cell.size.z = cellZExtent;
				unknownPriorityList.push_back(cell);
			}
		}
	}

	multimap<double, Cell> priorityList;
	vector<double> cellTravelTimes(pickList.size());
	set<tuple<float, float, float>> processedCells;
	Cell bestCell;
	bestCell.valid = false;

	//
	// Process each cell by its priority
	//
	while ( !priorityList.empty() || !unknownPriorityList.empty() ) {

		// before fetching the next cell with the highest priority make
		// sure to have processed all the cells in the unknownPriorityList
		// and put them in the priority list, ordered by their priority
		for ( Cell &cell : unknownPriorityList ) {

			//
			// Avoid processing the same cell twice
			//
			tuple<float, float, float> toProcess =
			    std::make_tuple(cell.x, cell.y, cell.z);

			if ( processedCells.count(toProcess) > 0 ) {
				continue;
			}
			processedCells.insert(toProcess);

			cell.org.depth = gridOriginDepth + cell.z;

			// compute distance and azimuth of the cell centroid to the grid
			// origin
			double distance = sqrt(cell.y * cell.y + cell.x * cell.x); // km
			double azimuth = rad2deg(atan2(cell.x, cell.y));

			// Computes the coordinates (lat, lon) of the point which is at
			// a degree azimuth and km distance as seen from the other point
			// location
			computeCoordinates(distance, azimuth, gridOriginLat, gridOriginLon,
			                   cell.org.lat, cell.org.lon);

			// Compute origin time
			bool ok = computeOriginTime(pickList, weights, sensorLat, sensorLon,
			                            sensorElev, cell.org.lat, cell.org.lon,
			                            cell.org.depth, cell.org.time,
			                            cellTravelTimes);

			if ( !ok ) {
				continue;
			}

			// Compute the prob density (log) and from there the cell
			// probability considering its volume
			computeProbDensity(pickList, weights, cellTravelTimes,
			                   cell.org.time, cell.org.probDensity,
			                   cell.org.rms);

			// add cell to the priority list
			double volume = cell.size.x * cell.size.y * cell.size.z;
			double logProb = std::log(volume) + cell.org.probDensity;

			if ( !isfinite(logProb) ) {
				continue;
			}

			cell.valid = true;
			priorityList.emplace(logProb, cell);
		}
		// all done
		unknownPriorityList.clear();

		//
		// Fetch and split the highest priority cell
		//
		if ( priorityList.empty() ) {
			break;
		}
		const Cell &topCell = priorityList.crbegin()->second;

		//
		// Keep track of the best solution, which is not the highest
		// probability cell but the one with highest probability density
		//
		if ( !bestCell.valid ||
		     bestCell.org.probDensity < topCell.org.probDensity ) {
			bestCell = topCell;
		}

		//
		// Check for completion
		//
		if ( _currentProfile.octTree.maxIterations > 0 &&
		     processedCells.size() >= _currentProfile.octTree.maxIterations ) {
			SEISCOMP_DEBUG("Maximum number of iteration reached");
			break;
		}

		if ( _currentProfile.octTree.minCellSize > 0 &&
		     (bestCell.size.x <= _currentProfile.octTree.minCellSize ||
		      bestCell.size.y <= _currentProfile.octTree.minCellSize ||
		      bestCell.size.z <= _currentProfile.octTree.minCellSize) ) {
			SEISCOMP_DEBUG("Minimum cell size reached");
			break;
		}

		//
		// Split current cell in 8 and add them to the unknownPriorityList,
		// but also, as in NonLinLoc, search the direct neighbouring cells
		// too, the ones adjiacent to the 6 faces of the current cell cube
		//
		auto newCellToProcess = [&unknownPriorityList, &topCell, &xExtent,
		                         &yExtent, &zExtent](int xOffset, int yOffset,
		                                             int zOffset) {
			Cell cell;
			cell.valid = false;
			cell.size.x = topCell.size.x / 2.;
			cell.size.y = topCell.size.y / 2.;
			cell.size.z = topCell.size.z / 2.;
			//
			// Careful: since we use the 'processed' std::set to keep track of
			// the already processed cells, we need to make sure the hash
			// function of two cell locations (x,y,z) is the same. So we must be
			// build the new x,y,z using values that end up with the x,y,z
			// independetly of the cell father x,y,z
			//
			cell.x = round(topCell.x / cell.size.x) * cell.size.x +
			         xOffset * cell.size.x / 2;
			cell.y = round(topCell.y / cell.size.y) * cell.size.y +
			         yOffset * cell.size.y / 2;
			cell.z = round(topCell.z / cell.size.z) * cell.size.z +
			         zOffset * cell.size.z / 2;

			if ( (abs(cell.x) + cell.size.x / 2.) <= (xExtent / 2.) &&
			     (abs(cell.y) + cell.size.y / 2.) <= (yExtent / 2.) &&
			     (abs(cell.z) + cell.size.z / 2.) <= (zExtent / 2.) ) {
				unknownPriorityList.push_back(cell);
			}
		};
		// offsets -1 and 1 -> 8 sub cells of current cell
		// offsets -3 and 3 -> neighbouring cells
		for ( int xOffset : std::array<int, 4>{-3, -1, 1, 3} ) {
			for ( int yOffset : std::array<int, 4>{-3, -1, 1, 3} ) {
				if ( usingFixedDepth() ) {
					newCellToProcess(xOffset, yOffset, 0);
				}
				else {
					for ( int zOffset : std::array<int, 4>{-3, -1, 1, 3} ) {
						newCellToProcess(xOffset, yOffset, zOffset);
					}
				}
			}
		}

		// Remove the current cell from priorityList since its
		// children will be added at the next loop
		priorityList.erase(std::prev(priorityList.end()));
	}

	if ( !bestCell.valid ) {
		throw LocatorException("Couldn't find a solution");
	}

	// check the solutiond doesn't lie on the grid boundary
	if ( (xExtent / 2. - std::abs(bestCell.x)) < bestCell.size.x ||
	     (yExtent / 2. - std::abs(bestCell.y)) < bestCell.size.y ||
	     ((zExtent / 2. - std::abs(bestCell.z)) < bestCell.size.z &&
	      !usingFixedDepth()) ) {
		_rejectLocation = true;
		_rejectionMsg = "The location lies on the grid boundary: rejecting it";
	}

	SEISCOMP_DEBUG("Iterations %zu min cell size %g %g %g x %g y %g z %g",
	               processedCells.size(), bestCell.size.x, bestCell.size.y,
	               bestCell.size.z, bestCell.x, bestCell.y, bestCell.z);

	newLat = bestCell.org.lat;
	newLon = bestCell.org.lon;
	newDepth = bestCell.org.depth;
	newTime = bestCell.org.time;

	//
	// To return the travelTimes we need to recompute them again (ugly)
	//
	Core::Time dummy;
	if ( !computeOriginTime(pickList, weights, sensorLat, sensorLon, sensorElev,
	                        newLat, newLon, newDepth, dummy, travelTimes) ) {
		throw LocatorException("Couldn't find a solution");
	}

	// compute covariance matrix
	if ( computeCovMtrx ) {
		// convert priorityList: map<double,Cell> -> vector<Cell>
		vector<Cell> cells;
		std::transform(begin(priorityList), end(priorityList),
		               back_inserter(cells),
		               [](decltype(priorityList)::value_type const &pair) {
			               return pair.second;
		               });
		computeCovarianceMatrix(cells, bestCell, false, covm);
	}
	SEISCOMP_DEBUG("OctTree solution RMS %g lat %g lon %g depth %g time %s ",
	               bestCell.org.rms, newLat, newLon, newDepth,
	               newTime.iso().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::locateGridSearch(const PickList &pickList,
                              const vector<double> &weights,
                              const vector<double> &sensorLat,
                              const vector<double> &sensorLon,
                              const vector<double> &sensorElev,
                              double &newLat, double &newLon, double &newDepth,
                              Core::Time &newTime, vector<double> &travelTimes,
                              CovMtrx &covm, bool computeCovMtrx,
                              bool enablePerCellLeastSquares) {
	SEISCOMP_DEBUG("Start Grid Search");

	if ( !_ttt ) {
		throw LocatorException(
		    "Travel time table has not been loaded, check logs");
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

	if ( _currentProfile.gridSearch.numXPoints < 1 ||
	     _currentProfile.gridSearch.numYPoints < 1 ||
	     _currentProfile.gridSearch.numZPoints < 1 ) {
		throw LocatorException("At least 1 point per dimension should be given");
	}

	covm.valid = false;

	double xExtent = _currentProfile.gridSearch.xExtent;
	double yExtent = _currentProfile.gridSearch.yExtent;
	double zExtent = _currentProfile.gridSearch.zExtent;

	double numXPoints = _currentProfile.gridSearch.numXPoints;
	double numYPoints = _currentProfile.gridSearch.numYPoints;
	double numZPoints = _currentProfile.gridSearch.numZPoints;

	double gridOriginLat = _currentProfile.gridSearch.originLat;
	double gridOriginLon = _currentProfile.gridSearch.originLon;
	double gridOriginDepth = _currentProfile.gridSearch.originDepth;

	//
	// Auto-position the grid center
	//
	if ( _currentProfile.gridSearch.autoOriginLat ) {
		gridOriginLat = computeMean(sensorLat);
		SEISCOMP_DEBUG("GridSearch.center auto latitude %g", gridOriginLat);
	}

	if ( _currentProfile.gridSearch.autoOriginLon ) {
		gridOriginLon = Geo::GeoCoordinate::normalizeLon(
		    computeCircularMean(sensorLon, false));
		SEISCOMP_DEBUG("GridSearch.center auto longitude %g", gridOriginLon);
	}

	if ( _currentProfile.gridSearch.autoOriginDepth ) {
		gridOriginDepth = -computeMean(sensorElev)/1000.;
		SEISCOMP_DEBUG("GridSearch.center auto depth %g", gridOriginDepth);
	}

	// fix depth
	if ( usingFixedDepth() ) {
		gridOriginDepth = fixedDepth();
		numZPoints = 1;
		SEISCOMP_DEBUG("Using fixed depth %g [km]", gridOriginDepth);
	}

	double cellXExtent;
	double cellYExtent;
	double cellZExtent;

	if ( numXPoints > 1 ) {
		cellXExtent = xExtent / (numXPoints -1);
	}
	else { // force the single point to the grid center
		cellXExtent = xExtent;
		xExtent = 0;
	}

	if ( numYPoints > 1 ) {
		cellYExtent = yExtent / (numYPoints -1);
	}
	else { // force the single point to the grid center
		cellYExtent = yExtent;
		yExtent = 0;
	}

	if ( numZPoints > 1 ) {
		cellZExtent = zExtent / (numZPoints -1);
	}
	else { // force the single point to the grid center
		cellZExtent = zExtent;
		zExtent = 0;
	}

	vector<Cell> cells;

	//
	// Build the list of cells withing the grid
	//
	for ( int ix = 0; ix < numXPoints; ix++ ) {
		for ( int iy = 0; iy < numYPoints; iy++ ) {
			for ( int iz = 0; iz < numZPoints; iz++ ) {
				Cell cell;
				cell.valid = false;
				cell.x = -xExtent/2 + ix * cellXExtent;
				cell.y = -yExtent/2 + iy * cellYExtent;
				cell.z = -zExtent/2 + iz * cellZExtent;
				cell.size.x = cellXExtent;
				cell.size.y = cellYExtent;
				cell.size.z = cellZExtent;

				cell.org.depth = gridOriginDepth + cell.z;

				// compute distance and azimuth of the cell centroid to the grid
				// origin
				double distance = sqrt(cell.y * cell.y + cell.x * cell.x); // km
				double azimuth = rad2deg(atan2(cell.x, cell.y));

				// Computes the coordinates (lat, lon) of the point which is at
				// a degree azimuth and km distance as seen from the other point
				// location
				computeCoordinates(distance, azimuth, gridOriginLat,
				                   gridOriginLon, cell.org.lat, cell.org.lon);

				cells.push_back(cell);
			}
		}
	}

	vector<double> cellTravelTimes(pickList.size());
	struct {
			Cell cell;
			vector<double> travelTimes;
			CovMtrx covm;
	} best;
	best.cell.valid = false;

	//
	// Process each cell now
	//
	for ( Cell &cell : cells ) {

		//
		// Compute origin time
		//
		bool ok = computeOriginTime(
		    pickList, weights, sensorLat, sensorLon, sensorElev, cell.org.lat,
		    cell.org.lon, cell.org.depth, cell.org.time, cellTravelTimes);

		if ( !ok ) {
			continue;
		}

		//
		// Optionally run Least Squares from cell center:
		// note that the cell position will be updated
		//
		if ( enablePerCellLeastSquares ) {
			try {
				locateLeastSquares(pickList, weights, sensorLat, sensorLon,
				                   sensorElev, cell.org.lat, cell.org.lon,
				                   cell.org.depth, cell.org.time, cell.org.lat,
				                   cell.org.lon, cell.org.depth, cell.org.time,
				                   cellTravelTimes, covm, computeCovMtrx);
			}
			catch ( exception &e ) {
				SEISCOMP_DEBUG(
				    "Could not get a Least Square solution (%s): skip cell",
				    e.what());
				continue;
			}
		}

		//
		// Compute cell probability density (log)
		//
		computeProbDensity(pickList, weights, cellTravelTimes, cell.org.time,
		                   cell.org.probDensity, cell.org.rms);

		cell.valid = true;

		//
		// Keep track of the best solution
		//
		if ( !best.cell.valid ||
		     best.cell.org.probDensity < cell.org.probDensity ) {
			best.cell = cell;
			best.travelTimes = cellTravelTimes;
			best.covm = covm;
		}
	}

	if ( !best.cell.valid ) {
		throw LocatorException("Couldn't find a solution");
	}

	if ( !enablePerCellLeastSquares ) {
		// check the solutiond doesn't lie on the grid boundary
		if ( (xExtent / 2. - std::abs(best.cell.x)) < best.cell.size.x ||
		     (yExtent / 2. - std::abs(best.cell.y)) < best.cell.size.y ||
		     ((zExtent / 2. - std::abs(best.cell.z)) < best.cell.size.z &&
		      !usingFixedDepth()) ) {
			_rejectLocation = true;
			_rejectionMsg =
			    "The location lies on the grid boundary: rejecting it";
		}
	}

	newLat = best.cell.org.lat;
	newLon = best.cell.org.lon;
	newDepth = best.cell.org.depth;
	newTime = best.cell.org.time;

	// return the travel times for the solution
	travelTimes = best.travelTimes;

	// compute covariance matrix if that is not already computed by LeastSquares
	if ( computeCovMtrx && !enablePerCellLeastSquares ) {
		computeCovarianceMatrix(cells, best.cell, false, covm);
	}
	else {
		covm = best.covm;
	}

	SEISCOMP_DEBUG("Grid Search solution RMS %g lat %g lon %g depth %g time %s",
	               best.cell.org.rms, newLat, newLon, newDepth,
	               newTime.iso().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::locateLeastSquares(
    const PickList &pickList, const vector<double> &weights,
    const vector<double> &sensorLat, const vector<double> &sensorLon,
    const vector<double> &sensorElev, double &newLat, double &newLon,
    double &newDepth, Core::Time &newTime, vector<double> &travelTimes,
    CovMtrx &covm, bool computeCovMtrx) const {

	double initDepth = _currentProfile.leastSquares.depthInit;
	double initLat = computeMean(sensorLat);
	double initLon = Geo::GeoCoordinate::normalizeLon(
	                      computeCircularMean(sensorLon, false));
	Core::Time initTime;
	bool ok = computeOriginTime(pickList, weights, sensorLat, sensorLon, sensorElev,
	                            initLat, initLon, initDepth, initTime, travelTimes);
	if ( !ok ) {
		throw LocatorException("Couldn't find a solution");
	}

	locateLeastSquares(pickList, weights, sensorLat, sensorLon, sensorElev,
	                   initLat, initLon, initDepth, initTime,
	                   newLat, newLon, newDepth, newTime,
	                   travelTimes, covm, computeCovMtrx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::locateLeastSquares(
    const PickList &pickList, const vector<double> &weights,
    const vector<double> &sensorLat, const vector<double> &sensorLon,
    const vector<double> &sensorElev, double initLat, double initLon,
    double initDepth, Core::Time initTime, double &newLat, double &newLon,
    double &newDepth, Core::Time &newTime, vector<double> &travelTimes,
    CovMtrx &covm, bool computeCovMtrx) const {

	SEISCOMP_DEBUG("Start Least Square with initial lat %g lon %g depth %g "
	               "time %s. Num iterations %d",
	               initLat, initLon, initDepth, initTime.iso().c_str(),
	               _currentProfile.leastSquares.iterations);

	if ( !_ttt ) {
		throw LocatorException(
		    "Travel time table has not been loaded, check logs");
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

	if ( usingFixedDepth() ) {
		initDepth = fixedDepth();
	}

	covm.valid = false;

	travelTimes.resize(pickList.size());

	vector<double> backazis(pickList.size());
	vector<double> dtdds(pickList.size());
	vector<double> dtdhs(pickList.size());

	double prevLat, prevLon, prevDepth;
	Core::Time prevTime;

	prevLat = newLat = initLat;
	prevLon = newLon = initLon;
	prevDepth = newDepth = initDepth;
	prevTime = newTime = initTime;

	bool revertToPrevIteration = false;

	//
	// Solve the system via least squares multiple times. Each time
	// improve the previous solution
	//
	for ( int iteration = 0;
	      iteration <= _currentProfile.leastSquares.iterations;
	      ++iteration ) {

		// the last additional iteration is for final stats (no inversion)
		bool lastIteration =
		    (iteration == _currentProfile.leastSquares.iterations);

		// recover an error by reverting to the last valid solution
		// E.g. the location moves outside the ttt boundary but we keep
		// the last solution that was withing the limits
		if ( revertToPrevIteration ) {
			if ( iteration <= 1 ) {
				throw LocatorException("Unable to find a location");
			}
			lastIteration = true;
			newLat = prevLat;
			newLon = prevLon;
			newDepth = prevDepth;
			newTime = prevTime;
			SEISCOMP_DEBUG("Locator stopped early, at iteration %d", iteration);
		}

		//
		// Load the information we need to build the Equation System
		//
		bool unableToComputeTT = true;
		for ( size_t i = 0; i < pickList.size(); ++i ) {
			const PickItem &pi = pickList[i];
			const PickPtr pick = pi.pick;

			travelTimes[i] = -1.0;

			if ( weights[i] <= 0 ) {
				continue;
			}

			TravelTime tt;

			try {
				const char *phaseName = pick->phaseHint().code().c_str();
				if ( _currentProfile.PSTableOnly ) {
					if ( *pick->phaseHint().code().begin() == 'P' ) {
						phaseName = "P";
					}
					else if ( *pick->phaseHint().code().begin() == 'S' ) {
						phaseName = "S";
					}
				}

				tt = _ttt->compute(phaseName, newLat, newLon, newDepth,
				                   sensorLat[i], sensorLon[i], sensorElev[i]);
			}
			catch ( exception &e ) {
				SEISCOMP_WARNING(
				    "Travel Time Table error for %s@%s.%s.%s and lat %g "
				    "lon %g depth %g: %s",
				    pick->phaseHint().code().c_str(),
				    pick->waveformID().networkCode().c_str(),
				    pick->waveformID().stationCode().c_str(),
				    pick->waveformID().locationCode().c_str(),
				    newLat, newLon, newDepth, e.what());
				continue;
			}

			if ( tt.time < 0 ||
			     (tt.time > 0 && tt.dtdd == 0 && tt.dtdh == 0) ) {
				SEISCOMP_WARNING(
				    "Travel Time Table error: data not returned for "
				    "%s@%s.%s.%s and lat %g lon %g depth %g",
				    pick->phaseHint().code().c_str(),
				    pick->waveformID().networkCode().c_str(),
				    pick->waveformID().stationCode().c_str(),
				    pick->waveformID().locationCode().c_str(),
				    newLat, newLon, newDepth);
				continue;
			}

			travelTimes[i] = tt.time;
			dtdds[i] = tt.dtdd;
			dtdhs[i] = tt.dtdh;
			if ( tt.azi ) { // 3D model
				backazis[i] = *tt.azi;
			}
			else {
				computeDistance(newLat, newLon, sensorLat[i], sensorLon[i],
				                nullptr, &backazis[i]);
			}

			unableToComputeTT = false;
		}

		if ( unableToComputeTT ) {
			SEISCOMP_WARNING("No travel times available: stop here");
			revertToPrevIteration = true;
			continue;
		}

		//
		// Prepare the Equation System
		//
		System eq(pickList.size());

		for ( size_t i = 0; i < pickList.size(); ++i ) {
			const PickItem &pi = pickList[i];
			const PickPtr pick = pi.pick;

			eq.W[i] = weights[i];

			if ( weights[i] <= 0 || travelTimes[i] < 0 ) {
				eq.W[i] = 0;
				continue;
			}

			Core::Time pickTime = pick->time().value();
			double residual =
			    (pickTime - (newTime + Core::TimeSpan(travelTimes[i])))
			        .length();
			eq.r[i] = residual;

			const double bazi = deg2rad(backazis[i]);
			eq.G[i][0] = dtdds[i] * sin(bazi); // dx [sec/deg]
			eq.G[i][1] = dtdds[i] * cos(bazi); // dy [sec/deg]
			eq.G[i][2] = dtdhs[i];             // dz [sec/km]
			eq.G[i][3] = 1.;                   // dtime [sec]

			if ( usingFixedDepth() ) {
				eq.G[i][2] = 0;                  // dz [sec/km]
			}
		}

		// the last iteration is use for computing the travelTimes on the final
		// location and eventually the covariance matrix
		if ( lastIteration ) {
			if ( computeCovMtrx ) {
				computeCovarianceMatrix(eq, covm);
			}
			break;
		}

		//
		// Solve the system
		//
		try {
			ostringstream solverLogs;

			if ( _currentProfile.leastSquares.solverType == "LSMR" ) {
				Adapter<lsmrBase> solver =
				    solve<lsmrBase>(eq, &solverLogs,
				                    _currentProfile.leastSquares.dampingFactor);
				// SEISCOMP_DEBUG(
				//     "Solver stopped because %u : %s (used %u iterations)",
				//     solver.GetStoppingReason(),
				//     solver.GetStoppingReasonMessage().c_str(),
				//     solver.GetNumberOfIterationsPerformed());
			}
			else if ( _currentProfile.leastSquares.solverType == "LSQR" ) {
				Adapter<lsqrBase> solver =
				    solve<lsqrBase>(eq, &solverLogs,
				                    _currentProfile.leastSquares.dampingFactor);
				// SEISCOMP_DEBUG(
				//     "Solver stopped because %u : %s (used %u iterations)",
				//     solver.GetStoppingReason(),
				//     solver.GetStoppingReasonMessage().c_str(),
				//     solver.GetNumberOfIterationsPerformed());
			}
			else {
				throw LocatorException(
				    "Solver type can only be LSMR or LSQR, but it is set to" +
				    _currentProfile.leastSquares.solverType);
			}

			// SEISCOMP_DEBUG("Solver logs:\n%s", solverLogs.str().c_str());
		}
		catch ( exception &e ) {
			SEISCOMP_WARNING("%s", e.what());
			revertToPrevIteration = true;
			continue;
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
			SEISCOMP_WARNING("Couldn't find a solution to the equation system");
			revertToPrevIteration = true;
			continue;
		}

		// save old values in case the next iteration fails
		prevLat = newLat;
		prevLon = newLon;
		prevDepth = newDepth;
		prevTime = newTime;

		// prepare values for next iteration
		newLat += latCorrection;
		newLon += lonCorrection;
		newDepth += depthCorrection;
		newTime += Core::TimeSpan(timeCorrection);
	}

	SEISCOMP_DEBUG("Least Square final solution lat %g lon %g "
	               "depth %g time %s",
	               newLat, newLon, newDepth, newTime.iso().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::computeCovarianceMatrix(const vector<Cell> &cells,
                                     const Cell &bestCell,
                                     bool useExpectedHypocenter,
                                     CovMtrx &covm) const {
	covm.valid = false;

	if ( !isfinite(bestCell.org.probDensity) ) {
		return;
	}

	//
	// Remember that we stored the log of the probability density, so we need
	// to std:exp(logProb) to obtain the actual probability density.
	// Then to transform the probability density to probability we have to
	// multiply it by the cell volume.
	// This is a relative probability, to normalize it we need to divide it
	// by the integral over all the processed cell probabilities (sum of all
	// cell probabilities)
	//
	auto relativeProbability = [&bestCell](const Cell &cell) {
		double volume = cell.size.x * cell.size.y * cell.size.z;
		return std::exp(cell.org.probDensity - bestCell.org.probDensity) *
		       volume;
	};

	//
	// The covariance should be computed using the expected hypocenter
	// (that is the weigheted mean of lat, lon, depth and time)
	// However the confidence ellipsoid is considered w.r.t. the
	// maximum likelihood hypocenter (i.e. bestCell).
	// So the useExpectedHypocenter allow to control the reference hypocenter
	// to use for computing the covariance matrix
	//
	double wmeanLat = 0, wmeanLon = 0, wmeanDepth = 0, wmeanTime = 0;
	double weightSum = 0.0;
	if ( useExpectedHypocenter ) {
		vector<double> longitudes;
		vector<double> weights;

		for ( const Cell &cell : cells ) {
			if ( !cell.valid ) {
				continue;
			}
			double weight = relativeProbability(cell);
			weightSum += weight;
			wmeanLat += cell.org.lat * weight;
			wmeanDepth += cell.org.depth * weight;
			wmeanTime += static_cast<double>(cell.org.time) * weight;
			longitudes.push_back(cell.org.lon);
			weights.push_back(weight);
		}
		wmeanLat /= weightSum;
		wmeanDepth /= weightSum;
		wmeanTime /= weightSum;
		wmeanLon = Geo::GeoCoordinate::normalizeLon(
		    computedWeightedCircularMean(longitudes, weights, false));
	}
	else {
		wmeanLat = bestCell.org.lat;
		wmeanLon = bestCell.org.lon;
		wmeanDepth = bestCell.org.depth;
		wmeanTime = static_cast<double>(bestCell.org.time);
	}

	weightSum = 0.0;
	std::array<std::array<double, 4>, 4> m = {};
	for ( const Cell &cell : cells ) {
		if ( !cell.valid ) {
			continue;
		}
		double weight = relativeProbability(cell);
		weightSum += weight;
		double rLon =
		    computeDistance(cell.org.lat, cell.org.lon, cell.org.lat, wmeanLon);
		if ( cell.org.lon > wmeanLon ) {
			rLon = Math::Geo::deg2km(rLon);
		}
		else {
			rLon = -Math::Geo::deg2km(rLon);
		}
		double rLat =
		    computeDistance(cell.org.lat, cell.org.lon, wmeanLat, cell.org.lon);
		if ( cell.org.lat > wmeanLat ) {
			rLat = Math::Geo::deg2km(rLat);
		}
		else {
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
void StdLoc::computeCovarianceMatrix(const System &eq, CovMtrx &covm) const {
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
		SEISCOMP_DEBUG(
		    "Cannot compute covariance matrix: less than 5 arrivals");
		return;
	}

	double sigma2 = 0;
	for ( unsigned int ob = 0; ob < eq.numRowsG; ob++ ) {
		sigma2 += eq.r[ob] * eq.r[ob];
	}
	sigma2 /= eq.numRowsG - 4;

	std::array<std::array<double, 4>, 4> GtG = {}; // G.T * G
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

	std::array<std::array<double, 4>, 4> inverseGtG;
	if ( !invertMatrix4x4(GtG, inverseGtG) ) {
		SEISCOMP_DEBUG(
		    "Cannot compute covariance matrix: G.T*G not invertible");
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
    const vector<double> &sensorElev, const vector<double> &travelTimes,
    double originLat, double originLon, double originDepth,
    const Core::Time &originTime, const CovMtrx &covm) const {
	if ( weights.size() != pickList.size() ) {
		throw LocatorException("Internal logic error");
	}

	DataModel::CreationInfo ci;
	ci.setCreationTime(Core::Time::GMT());

	Origin *origin = Origin::Create();
	SEISCOMP_DEBUG("New origin publicID: %s", origin->publicID().c_str());

	if ( _currentProfile.method == Profile::Method::LeastSquares ) {
		origin->setMethodID("StdLoc:LeastSquares");
	}
	else if ( _currentProfile.method == Profile::Method::GridSearch ) {
		origin->setMethodID("StdLoc:GridSearch");
	}
	else if ( _currentProfile.method == Profile::Method::OctTree ) {
		origin->setMethodID("StdLoc:OctTree");
	}
	else if ( _currentProfile.method == Profile::Method::GridAndLsqr ) {
		origin->setMethodID("StdLoc:GridSearch+LeastSquares");
	}
	else if ( _currentProfile.method == Profile::Method::OctTreeAndLsqr ) {
		origin->setMethodID("StdLoc:OctTree+LeastSquares");
	}

	origin->setCreationInfo(ci);
	origin->setEarthModelID(_tttType + ":" + _tttModel);
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

		++associatedPhaseCount;
		associatedStations.insert(pick->waveformID().networkCode() + "." +
		                          pick->waveformID().stationCode() + "." +
		                          pick->waveformID().locationCode());

		double azimuth = 0;
		double distance = computeDistance(originLat, originLon, sensorLat[i],
		                                  sensorLon[i], &azimuth);

		// prepare the new arrival
		DataModel::Arrival *newArr = new DataModel::Arrival();
		newArr->setCreationInfo(ci);
		newArr->setPickID(pick->publicID());
		newArr->setPhase(pick->phaseHint().code());
		newArr->setHorizontalSlownessUsed(false);
		newArr->setBackazimuthUsed(false);
		newArr->setAzimuth(azimuth);
		newArr->setDistance(distance);

		if ( weights[i] <= 0 || travelTimes[i] < 0 ) {
			// Not used
			newArr->setTimeUsed(false);
			newArr->setWeight(0.0);
			newArr->setTimeResidual(0.0);
		}
		else {
			newArr->setTimeUsed(true);
			newArr->setWeight(weights[i]);

			Core::Time pickTime = pick->time().value();
			double residual =
			    (pickTime - (originTime + Core::TimeSpan(travelTimes[i])))
			        .length();
			newArr->setTimeResidual(residual);

			sumSquaredResiduals +=
			    (residual * weights[i]) * (residual * weights[i]);
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

		if ( !covm.valid ) {
			SEISCOMP_DEBUG(
			    "No valid covariance matrix. No Confidence ellipsoid will "
			    "be computed");
		}
		else {

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
			 * The following table summarizes confidence coefficients for 0.90
			 * (LocSAT) and 0.68 (NonLinLoc) confidence levels
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
					g = alngam(dof / 2.0, &ifault);
					kppf[i] =
					    pow(ppchi2(_currentProfile.confLevel, dof, g, &ifault),
					        0.5);
				}

				double sx, sy, smajax, sminax, strike;

				// 1D confidence intervals
				sx = kppf[0] * pow(M4d[0], 0.5); // sxx
				sy = kppf[0] * pow(M4d[5], 0.5); // syy

				// 1D confidence intervals
				origin->setTime(DataModel::TimeQuantity(
				    originTime, sqrt(covm.stt) * kppf[0], Core::None,
				    Core::None, _currentProfile.confLevel * 100.0));
				origin->setLatitude(DataModel::RealQuantity(
				    originLat, sqrt(covm.syy) * kppf[0], Core::None, Core::None,
				    _currentProfile.confLevel * 100.0));
				origin->setLongitude(DataModel::RealQuantity(
				    originLon, sqrt(covm.sxx) * kppf[0], Core::None, Core::None,
				    _currentProfile.confLevel * 100.0));
				origin->setDepth(DataModel::RealQuantity(
				    originDepth, sqrt(covm.szz) * kppf[0], Core::None,
				    Core::None, _currentProfile.confLevel * 100.0));

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
				double s3dMajAxis, s3dMinAxis, s3dIntAxis, MajAxisPlunge,
				    MajAxisAzimuth, MajAxisRotation;

				s3dMinAxis = kppf[2] * pow(eigval3d[0], 0.5);
				s3dIntAxis = kppf[2] * pow(eigval3d[1], 0.5);
				s3dMajAxis = kppf[2] * pow(eigval3d[2], 0.5);

				MajAxisPlunge = rad2deg(atan(
				    eigvec3d[8] /
				    pow(pow(eigvec3d[6], 2.0) + pow(eigvec3d[7], 2.0), 0.5)));
				if ( MajAxisPlunge < 0.0 )
					MajAxisPlunge += 180.0;

				if ( MajAxisPlunge > 180.0 )
					MajAxisPlunge -= 180.0;

				MajAxisAzimuth = rad2deg(atan(eigvec3d[7] / eigvec3d[6]));
				if ( MajAxisAzimuth < 0.0 )
					MajAxisAzimuth += 180.0;

				if ( MajAxisAzimuth > 180.0 )
					MajAxisAzimuth -= 180.0;

				MajAxisRotation = rad2deg(atan(
				    eigvec3d[2] /
				    pow(pow(eigvec3d[0], 2.0) + pow(eigvec3d[1], 2.0), 0.5)));
				if ( covm.szz == 0.0 )
					MajAxisRotation = 0.0;

				if ( MajAxisRotation < 0.0 )
					MajAxisRotation += 180.0;

				if ( MajAxisRotation > 180.0 )
					MajAxisRotation -= 180.0;

				DataModel::ConfidenceEllipsoid confidenceEllipsoid;
				DataModel::OriginUncertainty originUncertainty;

				confidenceEllipsoid.setSemiMinorAxisLength(s3dMinAxis * 1000.0);
				confidenceEllipsoid.setSemiIntermediateAxisLength(s3dIntAxis *
				                                                  1000.0);
				confidenceEllipsoid.setSemiMajorAxisLength(s3dMajAxis * 1000.0);
				confidenceEllipsoid.setMajorAxisPlunge(MajAxisPlunge);
				confidenceEllipsoid.setMajorAxisAzimuth(MajAxisAzimuth);
				confidenceEllipsoid.setMajorAxisRotation(MajAxisRotation);

				// QuakeML, horizontalUncertainty: Circular confidence region,
				// given by single value of horizontal uncertainty. Acordingly,
				// 1D horizontal errors quadratic mean is given
				originUncertainty.setHorizontalUncertainty(
				    sqrt(pow(sx, 2) + pow(sy, 2)));
				originUncertainty.setMinHorizontalUncertainty(sminax);
				originUncertainty.setMaxHorizontalUncertainty(smajax);
				originUncertainty.setAzimuthMaxHorizontalUncertainty(strike);
				originUncertainty.setConfidenceEllipsoid(confidenceEllipsoid);
				originUncertainty.setPreferredDescription(
				    Seiscomp::DataModel::OriginUncertaintyDescription(
				        Seiscomp::DataModel::ELLIPSOID));

				origin->setUncertainty(originUncertainty);
			}
			else {
				SEISCOMP_DEBUG(
				    "Unable to calculate eigenvalues/eigenvectors. No "
				    "Confidence ellipsoid will be computed");
			}
		}
	}

	if ( _rejectLocation ) {
		origin->setEvaluationStatus(EvaluationStatus(REJECTED));
	}

	return origin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace
