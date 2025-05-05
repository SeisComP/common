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
#include <iostream>
#include <stdexcept>
#include <string.h>

#include <seiscomp/core/strings.h>
#include <seiscomp/system/environment.h>
#include <seiscomp/math/geo.h>
#include <seiscomp/seismology/ttt/locsat.h>


#define EXTRAPOLATE 0

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

	double takeoff = std::atan2(dtdh, dtdd2) * 180.0 / M_PI; // degress
	takeoff += 90; // -90(down):+90(up) -> 0(down):180(up)

	return takeoff;
}


inline void checkDepth(double depth) {
	if ( (depth < 0) || (depth > 800) ) {
		throw std::out_of_range(
			Seiscomp::Core::stringify(
				"Source depth of %f km is out of range of 0 <= z <= 800",
				depth
			)
		);
	}
}


}


namespace Seiscomp {
namespace TTT {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
extern "C" {

void distaz2_(double *lat1, double *lon1, double *lat2, double *lon2,
              double *delta, double *azi1, double *azi2);
int setup_tttables_dir(const char *new_dir);
double compute_ttime(double distance, double depth, const char *phase,
                     int extrapolate, double *rdtdd,  double *rdtdh,
                     int *errorflag);
int num_phases();
char **phase_types();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Locsat::Locsat() : _Pindex(-1) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Locsat::Locsat(const Locsat &other) {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Locsat &Locsat::operator=(const Locsat &other) {
	_Pindex = other._Pindex;
	_tablePrefix = other._tablePrefix;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Locsat::~Locsat() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Locsat::setModel(const std::string &model) {
	if ( _model == model ) {
		return true;
	}

	_model = model;

	if ( _model.empty() ) {
		_tablePrefix.clear();
		return true;
	}

	std::string tablePrefix = Environment::Instance()->shareDir() + "/locsat/tables/" + model;
	if ( _tablePrefix == tablePrefix ) return true;

	_tablePrefix = tablePrefix;
	return initTables();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Locsat::model() const {
	return _model;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Locsat::initTables() {
	if ( _tablePrefix.empty()
	  || (setup_tttables_dir(_tablePrefix.c_str()) != 0) )
		return false;

	int nphases = num_phases();
	char **phases = phase_types();

	_Pindex = -1;
	if ( phases != nullptr ) {
		for ( int i = 0; i < nphases; ++i ) {
			if ( !strcmp(phases[i], "P") ) {
				_Pindex = i;
				break;
			}
		}
	}

	return _Pindex != -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTimeList *Locsat::compute(double delta, double depth) {
	int nphases = num_phases();
	char **phases = phase_types();

	checkDepth(depth);

	TravelTimeList *ttlist = new TravelTimeList;
	ttlist->delta = delta;
	ttlist->depth = depth;

	//bool has_vel = get_vel(depth, &vp, &vs);

	for ( int i = 0; i < nphases; ++i ) {
		char *phase = phases[i];
		int errorflag = 0;
		double dtdd, dtdh;
		double ttime = compute_ttime(delta, depth, phase, EXTRAPOLATE,
		                             &dtdd, &dtdh, &errorflag);
		if (errorflag != 0) {
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
	int errorflag=0;
	double dtdd, dtdh;

	checkDepth(depth);

	double ttime = compute_ttime(delta, depth, phase, EXTRAPOLATE,
	                             &dtdd, &dtdh, &errorflag);
	if ( errorflag!=0 ) throw NoPhaseError();
	if ( !(ttime > 0) ) throw NoPhaseError();
 	double takeoff = takeoff_angle(dtdd, dtdh, depth);
	return TravelTime(phase, ttime, dtdd, dtdh, 0, takeoff);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double
Locsat::computeTime(const char *phase, double delta, double depth) {
	int errorflag=0;
	double dtdd, dtdh;

	checkDepth(depth);

	double ttime = compute_ttime(delta, depth, phase, EXTRAPOLATE,
	                             &dtdd, &dtdh, &errorflag);
	if ( errorflag!=0 ) throw NoPhaseError();
	if ( !(ttime > 0) ) throw NoPhaseError();
	return ttime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTimeList *Locsat::compute(double lat1, double lon1, double dep1,
                                double lat2, double lon2, double alt2,
                                int ellc) {
	checkDepth(dep1);

	if ( !initTables() ) {
		return nullptr;
	}

	double delta, azi1, azi2;

	distaz2_(&lat1, &lon1, &lat2, &lon2, &delta, &azi1, &azi2);

	/* TODO apply ellipticity correction */
	TravelTimeList *ttlist = compute(delta, dep1);
	ttlist->delta = delta;
	ttlist->depth = dep1;

	if ( ellc ) {
		for ( auto &tt : *ttlist ) {
			double ecorr = 0.;
			if ( ellipcorr(tt.phase, lat1, lon1, lat2, lon2, dep1, ecorr) )
				tt.time += ecorr;
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

	if ( !initTables() ) {
		throw NoPhaseError();
	}

	double delta, azi1, azi2;
	distaz2_(&lat1, &lon1, &lat2, &lon2, &delta, &azi1, &azi2);

	TravelTime tt = compute(phase, delta, dep1);

	if ( ellc ) {
		double ecorr = 0.;
		if ( ellipcorr(tt.phase, lat1, lon1, lat2, lon2, dep1, ecorr) )
			tt.time += ecorr;
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

	if ( !initTables() ) {
		throw NoPhaseError();
	}

	double delta, azi1, azi2;
	distaz2_(&lat1, &lon1, &lat2, &lon2, &delta, &azi1, &azi2);

	double ttime = computeTime(phase, delta, dep1);

	if ( ellc ) {
		double ecorr = 0.;
		if ( ellipcorr(phase, lat1, lon1, lat2, lon2, dep1, ecorr) )
			ttime += ecorr;
	}

	return ttime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TravelTime Locsat::computeFirst(double delta, double depth) {
	checkDepth(depth);

	char **phases = phase_types();
	char *phase = phases[_Pindex];
	int errorflag=0;
	double dtdd, dtdh;
	double ttime = compute_ttime(delta, depth, phase, EXTRAPOLATE,
	                             &dtdd, &dtdh, &errorflag);
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

	if ( !initTables() ) {
		throw NoPhaseError();
	}

	double delta, azi1, azi2;
	distaz2_(&lat1, &lon1, &lat2, &lon2, &delta, &azi1, &azi2);

	TravelTime tt = computeFirst(delta, dep1);
	if ( ellc ) {
		double ecorr = 0.;
		if ( ellipcorr(tt.phase, lat1, lon1, lat2, lon2, dep1, ecorr) )
			tt.time += ecorr;
	}

	return tt;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_TRAVELTIMETABLE(Locsat, "LOCSAT");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
