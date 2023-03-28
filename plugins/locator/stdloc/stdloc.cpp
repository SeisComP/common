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

#include "solver.h"
#include "stdloc.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;
using namespace LeastSquares;

using IDList = Seiscomp::Seismology::LocatorInterface::IDList;
using LocatorException = Seiscomp::Seismology::LocatorException;
using StationNotFoundException = Seiscomp::Seismology::StationNotFoundException;
using PickNotFoundException = Seiscomp::Seismology::PickNotFoundException;


namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ADD_SC_PLUGIN(
	"Standard Method Locator",
	"Luca Scarabello <luca.scarabello@erdw.ethz.ch>",
	1, 0, 0
)
REGISTER_LOCATOR(StdLoc, "StdLoc");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// Utility functions

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
	double mean = atan2(y / angles.size(), x / angles.size());
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

double computePickWeight(double uncertainty /* secs */) {
  unsigned uncertaintyClass;

  if (uncertainty >= 0.000 && uncertainty <= 0.025)
    uncertaintyClass = 0;
  else if (uncertainty > 0.025 && uncertainty <= 0.050)
    uncertaintyClass = 1;
  else if (uncertainty > 0.050 && uncertainty <= 0.100)
    uncertaintyClass = 2;
  else if (uncertainty > 0.100 && uncertainty <= 0.200)
    uncertaintyClass = 3;
  else if (uncertainty > 0.200 && uncertainty <= 0.400)
    uncertaintyClass = 4;
  else
    uncertaintyClass = 5;

  return 1 / pow(2, uncertaintyClass);
}

double getPickUncertainty(DataModel::Pick *pick, double defaultUncertainty) {
  double uncertainty = -1; // secs
  try {
    // symmetric uncertainty
    uncertainty = pick->time().uncertainty();
  } catch (Core::ValueException &) {
    // asymmetric uncertainty
    try {
      uncertainty =
          (pick->time().lowerUncertainty() + pick->time().upperUncertainty()) /
          2.0;
    } catch (Core::ValueException &) {
    }
  }

	if (uncertainty < 0 || !isfinite(uncertainty)) {
		uncertainty = defaultUncertainty;
	}

	return uncertainty;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// StdLoc implementation

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const IDList StdLoc::_allowedParameters = {
	"method",
	"tttType",
	"tttModel",
	"PSTableOnly",
	"GridSearch.center",
	"GridSearch.autoLatLon",
	"GridSearch.size",
	"GridSearch.cellSize",
	"GridSearch.errorType",
	"GridSearch.maxRms",
	"LeastSquares.iterations",
	"LeastSquares.dampingFactor",
	"LeastSquares.solverType",
	"usePickUncertainties",
	"defaultTimeError",
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StdLoc::init(const Config::Config &config) {
	vector<string> profileNames;
	try {
		profileNames = config.getStrings("StdLoc.profiles");
	}
	catch (...) {}

	Profile defaultProf;
	defaultProf.name = "default";
	defaultProf.method = Profile::Method::GridAndLsqr;
	defaultProf.tttType = "libtau";
	defaultProf.tttModel = "iasp91";
	defaultProf.PSTableOnly = true;
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
	defaultProf.usePickUncertainties = false;
	defaultProf.defaultTimeError = 1.0;

	_currentProfile = defaultProf;

	_profiles.clear();

	for ( const string &profileName : profileNames) {
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
		catch (...) {}

		try { prof.tttType = config.getString(prefix + "tableType"); }
		catch (...) {}

		try { prof.tttModel = config.getString(prefix + "tableModel"); }
		catch (...) {}

		try { prof.PSTableOnly = config.getBool(prefix + "PSTableOnly"); }
		catch (...) {}

		try {
			prof.gridSearch.autoLatLon = config.getBool(prefix + "GridSearch.autoLatLon");
		}
		catch (...) {}

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
		catch (...) {}

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
		catch (...) {}

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
		catch (...) {}

		try {
			prof.gridSearch.errorType = config.getString(prefix + "GridSearch.errorType");
			if ( prof.gridSearch.errorType != "L1" &&
			     prof.gridSearch.errorType != "L2") {
				SEISCOMP_ERROR("Profile %s: GridSearch.errorType is invalid",
				               prof.name.c_str());
				return false;
			}
		}
		catch (...) {}

		try {
			prof.gridSearch.maxRms = config.getDouble(prefix + "GridSearch.maxRms");
		}
		catch (...) {}

		try {
			prof.leastSquare.iterations = config.getInt(prefix + "LeastSquares.iterations");
		}
		catch (...) {}

		try {
			prof.leastSquare.dampingFactor = config.getDouble(prefix + "LeastSquares.dampingFactor");
		}
		catch (...) {}

		try {
			prof.leastSquare.solverType = config.getString(prefix + "LeastSquares.solverType");
			if ( prof.leastSquare.solverType != "LSMR" &&
			     prof.leastSquare.solverType != "LSQR") {
				SEISCOMP_ERROR("Profile %s: leastSquare.solverType is invalid",
				               prof.name.c_str());
				return false;
			}
		}
		catch (...) {}

		try {
			prof.usePickUncertainties = config.getBool(prefix + "usePickUncertainties");
		}
		catch (...) {}

		try {
			prof.defaultTimeError = config.getDouble(prefix + "defaultTimeError");
		}
		catch (...) {}

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
	else if ( name == "usePickUncertainties" ) {
		return _currentProfile.usePickUncertainties ? "y" : "n";
	}
	else if ( name == "defaultTimeError" ) {
		return Core::toString(_currentProfile.defaultTimeError);
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
	else if ( name == "usePickUncertainties" ) {
		_currentProfile.usePickUncertainties = (value == "y");
		return true;
	}
	else if ( name == "defaultTimeError" ) {
		double tmp;
		if ( !Core::fromString(tmp, value) ) {
			return false;
		}
		_currentProfile.defaultTimeError = tmp;
		return true;
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

	locateGridSearch(pickList, weights, sensorLat, sensorLon, sensorElev,
	                 originLat, originLon, originDepth, originTime, travelTimes);

	return createOrigin(pickList, weights, sensorLat, sensorLon, sensorElev,
	                    travelTimes, originLat, originLon, originDepth,
	                    originTime);
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

	if ( _currentProfile.method == Profile::Method::GridSearch ||
	     _currentProfile.method == Profile::Method::GridAndLsqr ) {
		locateGridSearch(pickList, weights, sensorLat, sensorLon, sensorElev,
		                 originLat, originLon, originDepth, originTime,
		                 travelTimes);
	}
	else if (_currentProfile.method == Profile::Method::LeastSquares) {
		locateLeastSquares(pickList, weights, sensorLat, sensorLon, sensorElev,
		                   initLat, initLon, initDepth, initTime, originLat,
		                   originLon, originDepth, originTime, travelTimes);
	}

	return createOrigin(pickList, weights, sensorLat, sensorLon, sensorElev,
	                    travelTimes, originLat, originLon, originDepth,
	                    originTime);
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
	catch (...) {
		throw LocatorException("incomplete origin, latitude is not set");
	}

	try {
		initLon = origin->longitude().value();
	}
	catch (...) {
		throw LocatorException("incomplete origin, longitude is not set");
	}

	try {
		initDepth = origin->depth().value();
	}
	catch (...) {
		throw LocatorException("incomplete origin, depth is not set");
	}

	try {
		initTime = origin->time().value();
	}
	catch (...) {
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
		catch (...) {
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
		catch (...) {
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
			double uncertainty = getPickUncertainty(pick.get(), _currentProfile.defaultTimeError);
			weights[i] = computePickWeight(uncertainty);
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
	double &newDepth, Core::Time &newTime, vector<double> &travelTimes
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

	double gridOriginLat = _currentProfile.gridSearch.originLat;
	double gridOriginLon = _currentProfile.gridSearch.originLon;
	if (_currentProfile.gridSearch.autoLatLon) {
		gridOriginLat = computeMean(sensorLat);
		gridOriginLon = Geo::GeoCoordinate::normalizeLon(computeCircularMean(sensorLon, false));
		SEISCOMP_DEBUG("GridSearch.center latitude %f longitude %f", gridOriginLat,
		               gridOriginLon);
	}

	travelTimes.resize(pickList.size());

	vector<double> originTimes;
	vector<double> timeWeights;
	double lowestError = nan("");
	vector<double> lowestErrorTravelTimes;
	double x, y, z;

	for ( x = -_currentProfile.gridSearch.xExtent / 2. +
	           _currentProfile.gridSearch.cellXExtent / 2.;
	      x < _currentProfile.gridSearch.xExtent / 2.;
	      x += _currentProfile.gridSearch.cellXExtent ) {
		for ( y = -_currentProfile.gridSearch.yExtent / 2. +
		           _currentProfile.gridSearch.cellYExtent / 2;
		      y < _currentProfile.gridSearch.yExtent / 2.;
		      y += _currentProfile.gridSearch.cellYExtent ) {
			for ( z = -_currentProfile.gridSearch.zExtent / 2. +
			           _currentProfile.gridSearch.cellZExtent / 2;
			      z < _currentProfile.gridSearch.zExtent / 2.;
			      z += _currentProfile.gridSearch.cellZExtent ) {
				double cellDepth = _currentProfile.gridSearch.originDepth + z;
				double cellLat, cellLon;
				// compute distance and azimuth of the cell centroid to the grid origin
				double distance = sqrt(y * y + x * x); // km
				double azimuth = rad2deg(atan2(x, y));
				// Computes the coordinates (lat, lon) of the point which is at a degree
				// azimuth and km distance as seen from the original event location
				computeCoordinates(distance, azimuth,
				                   gridOriginLat, gridOriginLon,
				                   cellLat, cellLon);

				SEISCOMP_DEBUG("Processing cell x=%f y=%f z=%f - lat %.6f lon %.6f "
				               "depth %.3f", x, y, z, cellLat, cellLon, cellDepth);

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
						tt = _ttt->compute(phaseName, cellLat, cellLon, cellDepth,
						                   sensorLat[i], sensorLon[i], sensorElev[i]);
					}
					catch ( exception &e ) {
						SEISCOMP_WARNING("Travel Time Table error for %s@%s.%s.%s and lat "
						                 "%.6f lon %.6f depth %.3f: %s",
						                 pick->phaseHint().code().c_str(),
						                 pick->waveformID().networkCode().c_str(),
						                 pick->waveformID().stationCode().c_str(),
						                 pick->waveformID().locationCode().c_str(), cellLat,
						                 cellLon, cellDepth, e.what());
						tttError = true;
						break;
					}

					if ( tt.time < 0 ) {
						SEISCOMP_WARNING("Travel Time Table error: data not returned for "
						                 "%s@%s.%s.%s and lat %.6f lon %.6f depth %.3f",
						                 pick->phaseHint().code().c_str(),
						                 pick->waveformID().networkCode().c_str(),
						                 pick->waveformID().stationCode().c_str(),
						                 pick->waveformID().locationCode().c_str(), cellLat,
						                 cellLon, cellDepth);
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
				double __originTime, __originTimeError;
				Math::Statistics::average(originTimes, timeWeights, __originTime,
				                          __originTimeError);
				Core::Time originTime(__originTime);

				//
				// Optionally run Least Squares
				//
				if ( _currentProfile.method == Profile::Method::GridAndLsqr ) {
					try {
						locateLeastSquares(pickList, weights, sensorLat, sensorLon,
						                   sensorElev, cellLat, cellLon, cellDepth,
						                   originTime, cellLat, cellLon, cellDepth,
						                   originTime, travelTimes);
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
						(pickTime - (originTime + Core::TimeSpan(travelTimes[i]))).length();
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

				double error;
				if ( _currentProfile.gridSearch.errorType == "L1" ) {
					error = l1SumWeightedResiduals;
				}
				else if ( _currentProfile.gridSearch.errorType == "L2" ) {
					error = l2SumWeightedResiduals;
				}
				else {
					throw LocatorException("The GridSearch error type can only be L1 or "
					                       "L2, but it is set to" +
					                       _currentProfile.gridSearch.errorType);
				}

				SEISCOMP_DEBUG("%s error %f (lowest error %f) unweighted rms %f",
				               _currentProfile.gridSearch.errorType.c_str(), error,
				               lowestError, rms);

				if ( !isfinite(lowestError) || lowestError > error ) {
					lowestError = error;
					newLat = cellLat;
					newLon = cellLon;
					newDepth = cellDepth;
					newTime = originTime;
					lowestErrorTravelTimes = travelTimes;
					SEISCOMP_DEBUG("Preferring this cell with error %f lat %.6f lon %.6f "
					               "depth %.3f time %s",
					               error, newLat, newLon, newDepth,
					               newTime.iso().c_str());
				}
			}
		}
	}

	if ( !isfinite(lowestError) ) {
		throw LocatorException("Couldn't find a solution");
	}

	// return the travel times for the solution
	travelTimes = lowestErrorTravelTimes;

	SEISCOMP_DEBUG(
		"Grid Search lowest error %f for lat %.6f lon %.6f depth %.3f time %s",
		lowestError, newLat, newLon, newDepth, newTime.iso().c_str()
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StdLoc::locateLeastSquares(
	const PickList &pickList, const vector<double> &weights,
	const vector<double> &sensorLat, const vector<double> &sensorLon,
	const vector<double> &sensorElev, double initLat, double initLon,
	double initDepth, Core::Time initTime, double &newLat, double &newLon,
	double &newDepth, Core::Time &newTime, vector<double> &travelTimes
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

	travelTimes.resize(pickList.size());

	vector<double> backazis(pickList.size());
	vector<double> dtdds(pickList.size());
	vector<double> dtdhs(pickList.size());

	for ( int iteration = 0; iteration < _currentProfile.leastSquare.iterations;
	      ++iteration ) {
		//
		// Load the information we need to build the Equation System
		//
		for ( size_t i = 0; i < pickList.size(); ++i ) {
			const PickItem &pi = pickList[i];
			const PickPtr pick = pi.pick;

			if ( weights[i] <= 0 ) {
				continue;
			}

			// get back azimuth, we don't need the distance
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
			eq.G[i][0] = Math::Geo::km2deg(dtdds[i]) * cos(bazi); // dy [sec/km]
			eq.G[i][1] = Math::Geo::km2deg(dtdds[i]) * sin(bazi); // dx [sec/km]
			eq.G[i][2] = dtdhs[i];                                // dz [sec/km]
			eq.G[i][3] = 1.;                                      // dtime [sec]
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
		catch (exception &e) {
			throw LocatorException(e.what());
		}

		//
		// Load the solution
		//
		double yCorrection = eq.m[0];    // km
		double xCorrection = eq.m[1];    // km
		double zCorrection = eq.m[2];    // km
		double timeCorrection = eq.m[3]; // sec

		if ( !isfinite(xCorrection) || !isfinite(yCorrection) ||
		     !isfinite(zCorrection) || !isfinite(timeCorrection) ) {
			throw LocatorException("Couldn't find a solution to the equation system");
		}

		newTime = initTime + Core::TimeSpan(timeCorrection);
		newDepth = initDepth + zCorrection;

		// compute distance and azimuth of the event to the new location
		double distance = sqrt(xCorrection * xCorrection + yCorrection * yCorrection); // km
		double azimuth = rad2deg(atan2(xCorrection, yCorrection));

		// Computes the coordinates (lat, lon) of the point which is at a degree
		// azimuth and km distance as seen from the original event location
		computeCoordinates(distance, azimuth, initLat, initLon, newLat, newLon);

		SEISCOMP_DEBUG("Least Square iteration %d: corrections x %f [km] y %f [km] "
		               "z %f [km] time %f [sec]. New source parameters lat %.6f "
		               "lon %.6f depth %.3f time %s",
		               iteration, xCorrection, yCorrection, zCorrection,
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
Origin *StdLoc::createOrigin(
	const PickList &pickList, const vector<double> &weights,
	const vector<double> &sensorLat, const vector<double> &sensorLon,
	const vector<double> &sensorElev, const vector<double> &travelTimes,
	double originLat, double originLon, double originDepth,
	const Core::Time &originTime
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

	return origin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace
