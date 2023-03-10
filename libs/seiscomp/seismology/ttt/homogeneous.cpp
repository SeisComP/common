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

#include <seiscomp/seismology/ttt.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/datamodel/config.h>
#include <seiscomp/system/environment.h>

#include <string>
#include <cmath>
#include <vector>

using namespace std;
using namespace Seiscomp;

namespace {

/**
 * Homogeneous
 *
 * A class to compute seismic travel times for an homogeneous (constant)
 * velocity
 */
class Homogeneous : public TravelTimeTableInterface {
	public:
		/**
		 * Construct a TravelTimeTable object for the specified model.
		 */
		Homogeneous() = default;
		~Homogeneous() = default;

		Homogeneous(const Homogeneous &other) = default;
		Homogeneous &operator=(const Homogeneous &other) = default;

		Homogeneous(Homogeneous &&other) = default;
		Homogeneous &operator=(Homogeneous &&other) = default;

	public:
		bool setModel(const std::string &model) override;
		const std::string &model() const override;

		TravelTimeList *
		compute(double lat1, double lon1, double dep1,
		        double lat2, double lon2, double alt2 = 0.,
		        int ellc = 1) override;

		TravelTime
		compute(const char *phase,
		        double lat1, double lon1, double dep1,
		        double lat2, double lon2, double alt2 = 0.,
		        int ellc = 1) override;

		TravelTime
		computeFirst(double lat1, double lon1, double dep1,
		             double lat2, double lon2, double alt2 = 0.,
		             int ellc = 1) override;

		bool isInside(double lat, double lon, double dep);

	private:
		std::string _model;
		double _pVel = 1; // [km/s]
		double _sVel = 1; // [km/s]
		double _centerLat = 0; // [degree]
		double _centerLon = 0; // [degree]
		double _radius = 0; // [km]
		double _minDepth = 0; // [km]
		double _maxDepth = 0; // [km]
};

double radToDeg(double r) { return 180.0 * r / M_PI; }

double degToRad(double d) { return M_PI * d / 180.0; }

double computeDistance(double lat1, double lon1,
                       double lat2, double lon2,
                       double *azimuth = nullptr,
                       double *backAzimuth = nullptr) {
	double dist, az, baz;
	Math::Geo::delazi(lat1, lon1, lat2, lon2, &dist, &az, &baz);
	dist = Math::Geo::deg2km(dist);
	if ( azimuth ) {
		*azimuth = az;
	}
	if ( backAzimuth ) {
		*backAzimuth = baz;
	}
	return dist;
}

bool Homogeneous::setModel(const string &model) {

	// load global configuration
	Config::Config cfg;
	if ( ! Environment::Instance()->initConfig(&cfg, "") ) {
		return false;
	}

	string base = "ttt.homogeneous." + model + ".";
	vector<string> origin;
	try {
		_pVel      = cfg.getDouble(base + "P-velocity");
		_sVel      = cfg.getDouble(base + "S-velocity");
		_radius    = cfg.getDouble(base + "radius");
		_minDepth  = cfg.getDouble(base + "minDepth");
		_maxDepth  = cfg.getDouble(base + "maxDepth");
		origin = cfg.getStrings(base + "origin");
	}
	catch (...) {
		return false;
	}

	if ( origin.size() != 2                         ||
	   ! Core::fromString(_centerLat, origin.at(0)) ||
	   ! Core::fromString(_centerLon, origin.at(1)) ) {
		return false;
	}

	_model = model;
	return true;
}

const string &Homogeneous::model() const {
	return _model;
}

TravelTimeList *
Homogeneous::compute(double lat1, double lon1, double dep1,
                     double lat2, double lon2, double alt2, int ellc) {
	TravelTimeList *ttlist = new TravelTimeList;
	ttlist->delta = computeDistance(lat1, lon1, lat2, lon2);
	ttlist->depth = dep1;
	try {
		ttlist->push_back(compute("P", lat1, lon1, dep1, lat2, lon2,  alt2, ellc));
	}
	catch (const NoPhaseError& e ) { }
	try {
		ttlist->push_back(compute("S", lat1, lon1, dep1, lat2, lon2,  alt2, ellc));
	}
	catch (const NoPhaseError& e ) { }
	ttlist->sortByTime();
	return ttlist;
}

bool
Homogeneous::isInside(double lat, double lon, double dep)
{
	if ( dep < _minDepth || dep > _maxDepth ) {
		return false;
	}
	double dist = computeDistance(lat, lon, _centerLat, _centerLon);
	if ( dist > _radius ) {
		return false;
	}
	return true;
}

TravelTime
Homogeneous::compute(const char *phase,
                     double lat1, double lon1, double dep1,
                     double lat2, double lon2, double alt2, int ellc) {
	double velocity;
	if ( phase[0] == 'P' || phase[0] == 'p' ) {
		velocity = _pVel;
	}
	else if ( phase[0] == 'S' || phase[0] == 's' ) {
		velocity = _sVel;
	}
	else {
		throw NoPhaseError();
	}

	if ( !isInside(lat1, lon1, dep1) ) {
		throw NoPhaseError();
	}

	// straight ray path since we are in a homogeneous media
	double Hdist = computeDistance(lat1, lon1, lat2, lon2);
	double Vdist = dep1 + alt2/1000.;
	double distance = sqrt(Hdist*Hdist + Vdist*Vdist); // [km]

	double tt = distance / velocity; // [sec]
	double takeOffAngle = atan2(Vdist, Hdist); // [rad]

	double dtdd = std::cos(takeOffAngle)             // [sec/deg]
                / Math::Geo::km2deg(velocity);
	double dtdh = std::sin(takeOffAngle) / velocity; // [sec/km]

	takeOffAngle = radToDeg(takeOffAngle);
	takeOffAngle += 90; // -90(down):+90(up) -> 0(down):180(up)

	return TravelTime(phase, tt, dtdd, dtdh, 0, takeOffAngle);
}

TravelTime
Homogeneous::computeFirst(double lat1, double lon1, double dep1,
                          double lat2, double lon2, double alt2, int ellc) {
	if ( _pVel >= _sVel ) {
		return compute("P", lat1, lon1, dep1, lat2, lon2,  alt2, ellc);
	}
	else {
		return compute("S", lat1, lon1, dep1, lat2, lon2,  alt2, ellc);
	}
}

REGISTER_TRAVELTIMETABLE(Homogeneous, "homogeneous");

}

