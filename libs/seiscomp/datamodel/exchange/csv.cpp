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

#include "csv.h"

#include <seiscomp/core/exceptions.h>
#include <seiscomp/datamodel/event.h>
#include <seiscomp/datamodel/eventdescription.h>
#include <seiscomp/datamodel/eventparameters.h>
#include <seiscomp/datamodel/magnitude.h>
#include <seiscomp/datamodel/origin.h>

#include <iostream>
#include <iomanip>

using namespace Seiscomp::DataModel;

namespace Seiscomp {
namespace DataModel {

REGISTER_EXPORTER_INTERFACE(ExporterCSV, "csv");

Origin *findOrigin(EventParameters *ep, const std::string& id) {
	for ( size_t i = 0; i < ep->originCount(); ++i )
		if ( ep->origin(i)->publicID() == id )
			return ep->origin(i);
	return nullptr;
}

Magnitude *findMagnitude(Origin *o, const std::string& id) {
	for ( size_t i = 0; i < o->magnitudeCount(); ++i )
		if ( o->magnitude(i)->publicID() == id )
			return o->magnitude(i);
	return nullptr;
}

ExporterCSV::ExporterCSV() {
	_delim = ",";
}

void ExporterCSV::setDelimiter(std::string &delim) {
	_delim = delim;
}

const std::string& ExporterCSV::getDelimiter() const {
	return _delim;
}

bool ExporterCSV::put(std::streambuf* buf, Core::BaseObject* obj) {
	if ( buf == nullptr ) return false;
	if ( obj == nullptr ) return false;
	EventParameters* ep = EventParameters::Cast(obj);
	if ( ep == nullptr ) return false;

	std::ostream os(buf);
	os  << std::setiosflags(std::ios_base::fixed);

	// prettyPrint flag enables header output
	if ( _prettyPrint )
		os << "eventID" << _delim << "originTime(UTC)" << _delim << "latitude"
		   << _delim << "longitude" << _delim << "depth" << _delim
		   << "magnitude" << _delim << "description" << std::endl;

	for ( size_t i = 0; i < ep->eventCount(); ++i ) {
		Event* e = ep->event(i);
		os << e->publicID() << _delim;

		Origin* o = findOrigin(ep, e->preferredOriginID());
		if ( !o ) {
			os << _delim << _delim << _delim << _delim;
		}
		else {
			os << o->time().value().iso() << _delim
			   << std::setprecision(8) << o->latitude().value() << _delim
			   << std::setprecision(8) << o->longitude().value() << _delim;
			try {
				os << std::setprecision(6) << o->depth().value();
			} catch (Core::ValueException&) {}
			os << _delim;
			Magnitude* m = findMagnitude(o, e->preferredMagnitudeID());
			if ( m )
				os << std::setprecision(4) << m->magnitude().value();
		}
		os << _delim;

		for ( size_t j = 0; j < e->eventDescriptionCount(); ++j ) {
			EventDescription* desc = e->eventDescription(j);
			if ( desc->type() == REGION_NAME ) {
				os << "\"" << desc->text() << "\"";
				break;
			}
		}

		os << std::endl;
	}
	return true;
}

} // namespace DataModel
} // namesapce Seiscomp
