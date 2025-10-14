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


#include <math.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <cstring>

#include <seiscomp/core/strings.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/math/geo.h>

#include "locsat_private.h"


#define EXTRAPOLATE 0


using namespace std;

namespace fs = std::filesystem;


namespace {


// Compute the "takeoff angle" of a wave at the source.
// dtdd  [s/rad]
// dtdh  [s/km]
// depth [km]
double takeoff_angle(double dtdd, double dtdh, double depth) {

	// no slowness provided
	if ( dtdd == 0 && dtdh == 0 ) {
		return 0;
	}

	static constexpr double earthMeanRadius = Seiscomp::Math::Geo::WGS84_MEAN_RADIUS * 0.001; // km

	// We want dtdd and dtdh to use the same units:
	// So transform dtdd [s/rad] -> [s/km]
	double dtdd2 = dtdd / ((earthMeanRadius - depth) * M_PI / 180.);

	double takeoff = atan2(dtdh, dtdd2) * 180.0 / M_PI; // degress
	takeoff += 90; // -90(down):+90(up) -> 0(down):180(up)

	return takeoff;
}


inline void checkDepth(double depth) {
	if ( (depth < 0) || (depth > 800) ) {
		throw out_of_range(
			Seiscomp::Core::stringify(
				"Source depth of %f km is out of range of 0 <= z <= 800 by LOCSAT",
				depth
			)
		);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
extern "C" {

#include "geog.h"

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Locsat::Locsat() : _Pindex(-1) {
	sc_locsat_init_ttt(&_ttt);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Locsat::~Locsat() {
	sc_locsat_free_ttt(&_ttt);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Locsat::setModel(const string &model) {
	if ( _model == model ) {
		return true;
	}

	_tablePrefix = string();
	_model = model;
	sc_locsat_free_ttt(&_ttt);

	if ( _model.empty() ) {
		return true;
	}

	std::string prefix;

	const char *tablePath = getenv("SEISCOMP_LOCSAT_TABLE_DIR");
	if ( tablePath ) {
		fs::path path(tablePath);
		path /= model;
		prefix = path.string();
	}
	else {
		prefix = Environment::Instance()->shareDir() + "/locsat/tables/" + _model;
	}

	if ( _tablePrefix == prefix ) {
		return true;
	}

	_tablePrefix = std::move(prefix);

	if ( sc_locsat_setup_tttables(&_ttt, _tablePrefix.c_str(), 0) != 0 ) {
		return false;
	}

	_Pindex = sc_locsat_find_phase(&_ttt, "P");
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const string &Locsat::model() const {
	return _model;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTimeList *Locsat::compute(double delta, double depth) {
	auto nphases = _ttt.num_phases;
	auto phases = _ttt.phases;

	checkDepth(depth);

	TravelTimeList *ttlist = new TravelTimeList;
	ttlist->delta = delta;
	ttlist->depth = depth;

	//bool has_vel = get_vel(depth, &vp, &vs);

	for ( int i = 0; i < nphases; ++i ) {
		auto phase = phases[i];
		int errorflag = 0;
		double dtdd;
		double dtdh;
		double ttime = sc_locsat_compute_ttime(
			&_ttt, delta, depth, phase, EXTRAPOLATE,
			&dtdd, &dtdh, &errorflag
		);
		if ( errorflag ) {
			continue;
		}

		// This comparison is there to also skip NaN values
		if ( !(ttime > 0) ) {
			continue;
		}

		double takeoff = takeoff_angle(dtdd, dtdh, depth);
		ttlist->push_back(TravelTime(phase, ttime, dtdd, dtdh, 0, takeoff));
	}

	ttlist->sortByTime();

	return ttlist;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTime Locsat::compute(const char *phase, double delta, double depth) {
	int errorflag = 0;
	double dtdd;
	double dtdh;

	checkDepth(depth);

	double ttime = sc_locsat_compute_ttime(
		&_ttt, delta, depth, phase, EXTRAPOLATE,
		&dtdd, &dtdh, &errorflag
	);
	if ( errorflag ) {
		throw NoPhaseError();
	}
	if ( !(ttime > 0) ) {
		throw NoPhaseError();
	}
 	double takeoff = takeoff_angle(dtdd, dtdh, depth);
	return TravelTime(phase, ttime, dtdd, dtdh, 0, takeoff);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Locsat::computeTime(const char *phase, double delta, double depth) {
	int errorflag = 0;
	double dtdd;
	double dtdh;

	checkDepth(depth);

	double ttime = sc_locsat_compute_ttime(
		&_ttt, delta, depth, phase, EXTRAPOLATE,
		&dtdd, &dtdh, &errorflag
	);
	if ( errorflag ) {
		throw NoPhaseError();
	}
	if ( !(ttime > 0) ) {
		throw NoPhaseError();
	}
	return ttime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTimeList *Locsat::compute(double lat1, double lon1, double dep1,
                                double lat2, double lon2, double alt2,
                                int ellc) {
	checkDepth(dep1);
	if ( !_ttt.num_phases ) {
		return nullptr;
	}

	double delta;
	double azi1;
	double azi2;

	sc_locsat_distaz2(lat1, lon1, lat2, lon2, &delta, &azi1, &azi2);

	TravelTimeList *ttlist = compute(delta, dep1);
	ttlist->delta = delta;
	ttlist->depth = dep1;

	if ( ellc ) {
		for ( auto &tt : *ttlist ) {
			tt.time += ellipticityCorrection(tt.phase, lat1, lon1, dep1, lat2, lon2);
		}
	}

	return ttlist;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTime Locsat::compute(const char *phase,
                           double lat1, double lon1, double dep1,
                           double lat2, double lon2, double alt2,
                           int ellc) {
	checkDepth(dep1);

	if ( !_ttt.num_phases ) {
		throw NoPhaseError();
	}

	double delta;
	double azi1;
	double azi2;
	sc_locsat_distaz2(lat1, lon1, lat2, lon2, &delta, &azi1, &azi2);

	TravelTime tt = compute(phase, delta, dep1);

	if ( ellc ) {
		tt.time += ellipticityCorrection(tt.phase, lat1, lon1, dep1, lat2, lon2);
	}

	return tt;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Locsat::computeTime(const char *phase,
                           double lat1, double lon1, double dep1,
                           double lat2, double lon2, double alt2,
                           int ellc) {
	checkDepth(dep1);

	if ( !_ttt.num_phases ) {
		throw NoPhaseError();
	}

	double delta;
	double azi1;
	double azi2;
	sc_locsat_distaz2(lat1, lon1, lat2, lon2, &delta, &azi1, &azi2);

	double ttime = computeTime(phase, delta, dep1);

	if ( ellc ) {
		ttime += ellipticityCorrection(phase, lat1, lon1, dep1, lat2, lon2);
	}

	return ttime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTime Locsat::computeFirst(double delta, double depth) {
	if ( _Pindex < 0 ) {
		throw NoPhaseError();
	}

	checkDepth(depth);

	auto phase = _ttt.phases[_Pindex];
	int errorflag=0;
	double dtdd;
	double dtdh;
	double ttime = sc_locsat_compute_ttime(
		&_ttt, delta, depth, phase, EXTRAPOLATE,
		&dtdd, &dtdh, &errorflag
	);
	if ( errorflag ) {
		throw NoPhaseError();
	}

	if ( !(ttime > 0) ) {
		throw NoPhaseError();
	}

	double takeoff = takeoff_angle(dtdd, dtdh, depth);
	return TravelTime(phase, ttime, dtdd, dtdh, 0, takeoff);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTime Locsat::computeFirst(double lat1, double lon1, double dep1,
                                double lat2, double lon2, double alt2,
                                int ellc) {
	checkDepth(dep1);

	if ( !_ttt.num_phases ) {
		throw NoPhaseError();
	}

	double delta;
	double azi1;
	double azi2;
	sc_locsat_distaz2(lat1, lon1, lat2, lon2, &delta, &azi1, &azi2);

	TravelTime tt = computeFirst(delta, dep1);
	if ( ellc ) {
		tt.time += ellipticityCorrection(tt.phase, lat1, lon1, dep1, lat2, lon2);
	}

	return tt;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_TRAVELTIMETABLE(Locsat, "LOCSAT");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
