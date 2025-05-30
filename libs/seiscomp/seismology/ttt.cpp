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
#include <seiscomp/math/geo.h>
#include <seiscomp/core/interfacefactory.ipp>

extern "C" {
#include "loc.h"
}


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::TravelTimeTableInterface, SC_SYSTEM_CORE_API);


namespace Seiscomp {


double ellipticityCorrection(const std::string &phase,
                             double lat1, double lon1, double depth,
                             double lat2, double lon2) {
	double delta, azi1, azi2;
	Seiscomp::Math::Geo::delazi(lat1, lon1, lat2, lon2, &delta, &azi1, &azi2);
	return sc_locsat_elpcor(phase.c_str(), delta, depth, azi1, 90. - lat1);
}



TravelTime::TravelTime() {}

TravelTime::TravelTime(const std::string &_phase,
                       double _time, double _dtdd, double _dtdh, double _dddp,
                       double _takeoff) {
	phase   = _phase;
	time    = _time;
	dtdd    = _dtdd;
	dtdh    = _dtdh;
	dddp    = _dddp;
	takeoff = _takeoff;
}

bool TravelTime::operator==(const TravelTime &other) const {
	return phase == other.phase &&
	       time == other.time &&
	       dtdd == other.dtdd &&
	       dtdh == other.dtdh &&
	       dddp == other.dddp &&
	       takeoff == other.takeoff;
}

bool TravelTime::operator<(const TravelTime &other) const {
	return phase < other.phase &&
	       time < other.time &&
	       dtdd < other.dtdd &&
	       dtdh < other.dtdh &&
	       dddp < other.dddp &&
	       takeoff < other.takeoff;
}


namespace {

struct TTpred {
	bool operator()(const TravelTime &t1, const TravelTime &t2)
	{
		return t1.time < t2.time;
	}
};

}


void TravelTimeList::sortByTime() {
	sort(TTpred());
}


const TravelTime *getPhase(const TravelTimeList *list, const std::string &phase) {
	TravelTimeList::const_iterator it;

	for ( it = list->begin(); it != list->end(); ++it ) {
		// direct match
		if ( (*it).phase == phase ) break;

		// no match for 1st character -> don't keep trying
		if ( (*it).phase[0] != phase[0] )
			continue;

		if ( phase == "P" ) {
			if ( list->delta < 120 ) {
				if ( (*it).phase == "Pn"    ) break;
				if ( (*it).phase == "Pb"    ) break;
				if ( (*it).phase == "Pg"    ) break;
				if ( (*it).phase == "Pdiff" ) break;
			}
			else
				if ( (*it).phase.substr(0,3) == "PKP" ) break;
		}
		else if ( phase == "pP" ) {
			if ( list->delta < 120 ) {
				if ( (*it).phase == "pPn"   ) break;
				if ( (*it).phase == "pPb"   ) break;
				if ( (*it).phase == "pPg"   ) break;
				if ( (*it).phase == "pPdiff") break;
			}
			else {
				if ( (*it).phase.substr(0,4) == "pPKP" ) break;
			}
		}
		else if ( phase == "PKP" ) {
			if ( list->delta > 100 ) {
				if ( (*it).phase == "PKPab" ) break;
				if ( (*it).phase == "PKPbc" ) break;
				if ( (*it).phase == "PKPdf" ) break;
			}
		}
		else if ( phase == "PKKP" ) {
			if ( list->delta > 100 && list->delta < 130 ) {
				if ( (*it).phase == "PKKPab" ) break;
				if ( (*it).phase == "PKKPbc" ) break;
				if ( (*it).phase == "PKKPdf" ) break;
			}
		}
		else if ( phase == "SKP" ) {
			if ( list->delta > 115 && list->delta < 145 ) {
				if ( (*it).phase == "SKPab" ) break;
				if ( (*it).phase == "SKPbc" ) break;
				if ( (*it).phase == "SKPdf" ) break;
			}
		}
		else if ( phase == "PP" ) {
			if ( (*it).phase == "PnPn" ) break;
		}
		else if ( phase == "sP" ) {
			if ( list->delta < 120 ) {
				if ( (*it).phase == "sPn"   ) break;
				if ( (*it).phase == "sPb"   ) break;
				if ( (*it).phase == "sPg"   ) break;
				if ( (*it).phase == "sPdiff") break;
			}
			else {
				if ( (*it).phase.substr(0,4)=="sPKP" ) break;
			}
		}
		else if ( phase == "S" ) {
			if ( (*it).phase == "Sn"   ) break;
			if ( (*it).phase == "Sb"   ) break;
			if ( (*it).phase == "Sg"   ) break;
			if ( (*it).phase == "S"    ) break;
			if ( (*it).phase == "Sdiff") break;
			if ( (*it).phase.substr(0,3) == "SKS" ) break;
		}
	}

	if ( it == list->end() )
		return nullptr;

	return &(*it);
}


const TravelTime* firstArrivalP(const TravelTimeList *ttlist)
{
	TravelTimeList::const_iterator it;
	for (it = ttlist->begin(); it != ttlist->end(); ++it) {
		// direct match
		if ((*it).phase == "P")
			break;

		// no match for 1st character -> don't keep trying
		if ( (*it).phase[0] != 'P' ) continue;

		if ( ttlist->delta < 120 ) {
			if ( (*it).phase == "Pn"   ) break;
			if ( (*it).phase == "Pb"   ) break;
			if ( (*it).phase == "Pg"   ) break;
			if ( (*it).phase == "Pdiff") break;
		}
		else {
			if ( (*it).phase.substr(0,3)=="PKP" ) break;
		}
	}

	if ( it == ttlist->end() )
		return nullptr;

	return &(*it);
}


TravelTimeTableInterface::TravelTimeTableInterface() {}
TravelTimeTableInterface::~TravelTimeTableInterface() {}


TravelTimeTableInterface *TravelTimeTableInterface::Create(const char *name) {
	return TravelTimeTableInterfaceFactory::Create(name);
}


TravelTime TravelTimeTableInterface::compute(const char *phase,
                                             double lat1, double lon1, double dep1,
                                             double lat2, double lon2, double alt2,
                                             int ellc) {
	TravelTimeList *ttlist = compute(lat1, lon1, dep1, lat2, lon2, alt2, ellc);
	if ( ttlist == nullptr )
		throw NoPhaseError();

	TravelTime ret;
	const TravelTime *tt = getPhase(ttlist, phase);

	if ( tt == nullptr ) {
		delete ttlist;
		throw NoPhaseError();
	}

	ret = *tt;
	delete ttlist;
	return ret;
}


double TravelTimeTableInterface::computeTime(const char *phase,
                                         double lat1, double lon1, double dep1,
                                         double lat2, double lon2, double alt2,
                                         int ellc) {
  return compute(phase, lat1, lon1, dep1, lat2, lon2, alt2, ellc).time;
}


TravelTimeTableInterfacePtr TravelTimeTable::_interface;


TravelTimeTable::TravelTimeTable() {
	if ( !_interface )
		_interface = TravelTimeTableInterfaceFactory::Create("libtau");
}


bool TravelTimeTable::setModel(const std::string &model) {
	if ( _interface )
		return _interface->setModel(model);
	return false;
}


const std::string &TravelTimeTable::model() const {
	static std::string empty;
	if ( _interface )
		return _interface->model();
	return empty;
}


TravelTimeList *
TravelTimeTable::compute(double lat1, double lon1, double dep1,
                         double lat2, double lon2, double alt2,
                         int ellc) {
	if ( _interface )
		return _interface->compute(lat1, lon1, dep1, lat2, lon2, alt2, ellc);
	return nullptr;
}


TravelTime
TravelTimeTable::compute(const char *phase,
                         double lat1, double lon1, double dep1,
                         double lat2, double lon2, double alt2,
                         int ellc) {
	if ( _interface )
		return _interface->compute(phase, lat1, lon1, dep1, lat2, lon2, alt2, ellc);
	throw NoPhaseError();
}


TravelTime
TravelTimeTable::computeFirst(double lat1, double lon1, double dep1,
                              double lat2, double lon2, double alt2,
                              int ellc) {
	if ( _interface )
		return _interface->computeFirst(lat1, lon1, dep1, lat2, lon2, alt2, ellc);
	throw NoPhaseError();
}


}
