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



#include <seiscomp/system/application.h>
#include <seiscomp/seismology/ttt.h>
#include <seiscomp/core/strings.h>
#include <seiscomp/geo/coordinate.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/datamodel/config.h>
#include <seiscomp/system/environment.h>


#include <string>
#include <cmath>
#include <map>
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

		double
		computeTime(const char *phase,
		            double lat1, double lon1, double dep1,
		            double lat2, double lon2, double alt2=0.,
		            int ellc = 1) override;

		bool isInside(double lat, double lon, double dep);

	private:
		std::string _model;
		std::map<string, double> _velocities;
		double _centerLat{0}; // [degree]
		double _centerLon{0}; // [degree]
		double _radius{0}; // [km]
		double _minDepth{0}; // [km]
		double _maxDepth{0}; // [km]
};


double radToDeg(double r) { return 180.0 * r / M_PI; }


//double degToRad(double d) { return M_PI * d / 180.0; }


double computeDistance(double lat1, double lon1,
                       double lat2, double lon2,
                       double *azimuth = nullptr,
                       double *backAzimuth = nullptr) {
	double dist;
	Math::Geo::delazi(lat1, lon1, lat2, lon2, &dist, azimuth, backAzimuth);
	return dist;
}


bool Homogeneous::setModel(const string &model) {
	_velocities.clear();
	std::map<string, double> velocitiesModel;
	// load global configuration
	auto app = Seiscomp::System::Application::Instance();
	const Config::Config *cfg;
	Config::Config tmp;

	if ( app ) {
		cfg = &app->configuration();
	}
	else {
		if ( !Environment::Instance()->initConfig(&tmp, "") ) {
			return false;
		}
		else {
			cfg = &tmp;
		}
	}

	string base = "ttt.homogeneous." + model + ".";
	vector<string> origin;
	try {
		vector<string> velocities = cfg->getStrings(base + "velocities");
		for ( const auto &velocity : velocities ) {
			vector<string> toks;
			if ( Core::split(toks, velocity.c_str(), ":") != 2 ) {
				SEISCOMP_ERROR("Invalid configuration of '%svelocities'", base);
				return false;
			}

			double vel;
			if ( !Core::fromString(vel, toks[1]) ) {
				SEISCOMP_ERROR("Invalid configuration of '%svelocities': '%s'",
				               base, velocity);
			    return false;
			}
			velocitiesModel[toks[0]] = vel;
		}
		SEISCOMP_DEBUG("Found configuration of '%svelocities': Ignoring "
		               "'P-velocity', 'S-velocity'", base);
	}
	catch ( ... ) {
		try {
			velocitiesModel["P"] = cfg->getDouble(base + "P-velocity");
			velocitiesModel["S"] = cfg->getDouble(base + "S-velocity");
		}
		catch ( ... ) {
			return false;
		}
	}

	for ( const auto &velocity : velocitiesModel ) {
		if ( velocity.second <= 0 ) {
			SEISCOMP_ERROR("%svelocities: Found invalid %s velocity = %.3f",
			               base, velocity.first, velocity.second);
			return false;
		}
	}

	double radius;
	double minDepth;
	double maxDepth;
	try {
		radius    = cfg->getDouble(base + "radius");
		minDepth  = cfg->getDouble(base + "minDepth");
		maxDepth  = cfg->getDouble(base + "maxDepth");
		origin = cfg->getStrings(base + "origin");
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Incomplete configuration of source region for homogeneous model %s",
		               model);
		return false;
	}

	if ( (radius <= 0) || minDepth > maxDepth ) {
		SEISCOMP_ERROR("Inconsistent configuration of source region for homogeneous model %s",
		               model);
		return false;
	}

	if ( origin.size() != 2                         ||
	   ! Core::fromString(_centerLat, origin.at(0)) ||
	   ! Core::fromString(_centerLon, origin.at(1)) ) {
		SEISCOMP_ERROR("Incomplete configuration of %sorigin for homogeneous model",
		               base);
		return false;
	}
	Seiscomp::Geo::GeoCoordinate::normalizeLatLon(_centerLat, _centerLon);

	_radius = radius;
	_minDepth = minDepth;
	_maxDepth = maxDepth;
	_model = model;
	_velocities = velocitiesModel;
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

	// tolerate missing travel time and except whatever is available
	for ( const auto &velocityMap : _velocities ) {
			try {
				ttlist->push_back(compute(velocityMap.first.c_str(), lat1, lon1, dep1,
				                          lat2, lon2, alt2, ellc));
			}
			catch ( ... ) {}
		}

	if ( ttlist->empty() ) {
		throw NoPhaseError();
	}

	ttlist->sortByTime();
	return ttlist;
}


bool
Homogeneous::isInside(double lat, double lon, double dep)
{
	if ( dep < _minDepth || dep > _maxDepth ) {
		return false;
	}
	double dist = 
	    Math::Geo::deg2km(computeDistance(lat, lon, _centerLat, _centerLon));
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
	bool found = false;
	for ( const auto &velocityMap : _velocities ) {
		if ( phase == velocityMap.first ) {
			found = true;
			velocity = velocityMap.second;
			break;
		}
	}

	if ( !found ) {
		if ( phase[0] == 'P' || phase[0] == 'p' ) {
			auto it = _velocities.find("P");
			if ( it == _velocities.end() ) {
				throw NoPhaseError();
			}
			velocity = it->second;
			found = true;
		}
		else if ( phase[0] == 'S' || phase[0] == 's' ) {
			auto it = _velocities.find("S");
			if ( it == _velocities.end() ) {
				throw NoPhaseError();
			}
			velocity = it->second;
			found = true;
		}
	}

	if ( !found ) {
		throw NoPhaseError();
	}

	if ( !isInside(lat1, lon1, dep1) ) {
		throw std::out_of_range(
			Core::stringify("Source out of model %s range (lat %f lon %f depth %f)",
			                _model, lat1, lon1, dep1)
		);
	}

	// straight ray path since we are in a homogeneous media
	double Hdist = Math::Geo::deg2km(computeDistance(lat1, lon1, lat2, lon2));
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


double
Homogeneous::computeTime(const char *phase,
                         double lat1, double lon1, double dep1,
                         double lat2, double lon2, double alt2,
                         int ellc) {
	double velocity;
	bool found = false;
	for ( const auto &velocityMap : _velocities ) {
		if ( phase == velocityMap.first ) {
			found = true;
			velocity = velocityMap.second;
			break;
		}
	}

	if ( !found ) {
		if ( phase[0] == 'P' || phase[0] == 'p' ) {
			auto it = _velocities.find("P");
			if ( it == _velocities.end() ) {
				throw NoPhaseError();
			}
			velocity = it->second;
			found = true;
		}
		else if ( phase[0] == 'S' || phase[0] == 's' ) {
			auto it = _velocities.find("P");
			if ( it == _velocities.end() ) {
				throw NoPhaseError();
			}
			velocity = it->second;
			found = true;
		}
	}

	if ( !found ) {
		throw NoPhaseError();
	}

	if ( !isInside(lat1, lon1, dep1) ) {
		throw std::out_of_range(
			Core::stringify("Source out of model %s range (lat %f lon %f depth %f)",
			                _model, lat1, lon1, dep1)
		);
	}

	// straight ray path since we are in a homogeneous media
	double Hdist = Math::Geo::deg2km(computeDistance(lat1, lon1, lat2, lon2));
	double Vdist = dep1 + alt2/1000.;
	double distance = sqrt(Hdist*Hdist + Vdist*Vdist); // [km]

	double tt = distance / velocity; // [sec]

	return tt;
}


TravelTime
Homogeneous::computeFirst(double lat1, double lon1, double dep1,
                          double lat2, double lon2, double alt2, int ellc) {
	string firstPhase = "P";
	double firstPhaseVelocity = -1;
	for ( const auto &velocity : _velocities ) {
		if ( velocity.second > firstPhaseVelocity ) {
			firstPhaseVelocity = velocity.second;
			firstPhase = velocity.first;
		}
	}

	return compute(firstPhase.c_str(), lat1, lon1, dep1, lat2, lon2,  alt2, ellc);
}


REGISTER_TRAVELTIMETABLE(Homogeneous, "homogeneous");


}

