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



#define SEISCOMP_COMPONENT EventList
#include "eventlistview.h"
#include "eventlistview_p.h"

#include <seiscomp/gui/datamodel/ui_eventlistview.h>
#include <seiscomp/gui/datamodel/ui_eventlistviewregionfilterdialog.h>

#include <seiscomp/gui/core/application.h>
#include <seiscomp/gui/core/compat.h>
#include <seiscomp/gui/core/connectiondialog.h>
#include <seiscomp/gui/core/icon.h>
#include <seiscomp/gui/core/messages.h>
#include <seiscomp/gui/core/scheme.h>
#include <seiscomp/gui/datamodel/publicobjectevaluator.h>
#include <seiscomp/gui/datamodel/utils.h>
#include <seiscomp/gui/datamodel/ui_eventfilterwidget.h>
#include <seiscomp/logging/log.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/eventparameters.h>
#include <seiscomp/datamodel/event.h>
#include <seiscomp/datamodel/eventdescription.h>
#include <seiscomp/datamodel/magnitude.h>
#include <seiscomp/datamodel/origin.h>
#include <seiscomp/datamodel/originquality.h>
#include <seiscomp/datamodel/originreference.h>
#include <seiscomp/datamodel/momenttensor.h>
#include <seiscomp/datamodel/focalmechanism.h>
#include <seiscomp/datamodel/focalmechanismreference.h>
#include <seiscomp/datamodel/arrival.h>
#include <seiscomp/datamodel/station.h>
#include <seiscomp/datamodel/comment.h>
#include <seiscomp/datamodel/databasequery.h>
#include <seiscomp/datamodel/messages.h>
#include <seiscomp/datamodel/journalentry.h>
#include <seiscomp/io/archive/xmlarchive.h>
#include <seiscomp/io/archive/binarchive.h>
#include <seiscomp/geo/featureset.h>
#include <seiscomp/seismology/regions.h>

#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QTreeWidgetItem>

#include <algorithm>


#define SC_D (*_d_ptr)


using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::IO;


namespace Seiscomp::Gui {


namespace {


#define CMD_MERGE_EVENT  "EvMerge"
#define CMD_GRAB_ORIGIN  "EvGrabOrg"
#define CMD_NEW_EVENT    "EvNewEvent"
#define CMD_SPLIT_ORIGIN "EvSplitOrg"


MAKEENUM(
	EventListColumns,
	EVALUES(
		COL_OTIME,
		COL_TIME_AGO,
		COL_EVENTTYPE_CERTAINTY,
		COL_EVENTTYPE,
		COL_M,
		COL_MTYPE,
		COL_PHASES,
		COL_RMS,
		COL_AZIMUTHAL_GAP,
		COL_LAT,
		COL_LON,
		COL_DEPTH,
		COL_DEPTH_TYPE,
		COL_TYPE,
		COL_FM,
		COL_ORIGINS,
		COL_AGENCY,
		COL_AUTHOR,
		COL_REGION,
		COL_ID
	),
	ENAMES(
		"OT (%1)",
		"TimeAgo",
		"Certainty",
		"Type",
		"M",
		"MType",
		"Phases",
		"RMS",
		"AzGap",
		"Lat",
		"Lon",
		"Depth",
		"DType",
		"Stat",
		"FM",
		"Origins",
		"Agency",
		"Author",
		"Region",
		"ID"
	)
);


bool colVisibility[EventListColumns::Quantity] = {
	true,
	false,
	false,
	true,
	true,
	true,
	false,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	true,
	true,
	true,
	true
};


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define POPULATE_AGENCY(VALUE) \
do {\
	try {\
		setText(config.columnMap[COL_AGENCY], VALUE.c_str());\
		auto it = SCScheme.colors.agencies.find(VALUE);\
		if ( it != SCScheme.colors.agencies.end() )\
			setData(config.columnMap[COL_AGENCY], Qt::ForegroundRole, it.value());\
		else \
			setData(config.columnMap[COL_AGENCY], Qt::ForegroundRole, QVariant());\
	}\
	catch ( Seiscomp::Core::ValueException& ) {\
		setText(config.columnMap[COL_AGENCY], QString());\
		setData(config.columnMap[COL_AGENCY], Qt::ForegroundRole, QVariant());\
	}\
} while (0)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ByteArrayBuf : public std::streambuf {
	public:
		ByteArrayBuf(QByteArray &array) : _array(array) {}

	protected:
		int_type overflow (int_type c) override {
			_array.append((char)c);
			return c;
		}

		std::streamsize xsputn(const char* s, std::streamsize n) override {
			_array += QByteArray(s, n);
			return n;
		}

	private:
		QByteArray &_array;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define _T(name) ar->driver()->convertColumnName(name)


void addFilterConstraints(std::ostream &os, DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( filter.minLatitude ) {
		os << " and Origin." << _T("latitude_value") << " >= " << *filter.minLatitude;
	}
	if ( filter.maxLatitude ) {
		os << " and Origin." << _T("latitude_value") << " <= " << *filter.maxLatitude;
	}
	if ( filter.minLongitude ) {
		os << " and Origin." << _T("longitude_value") << " >= " << *filter.minLongitude;
	}
	if ( filter.maxLongitude ) {
		os << " and Origin." << _T("longitude_value") << " <= " << *filter.maxLongitude;
	}
	if ( filter.minDepth ) {
		os << " and Origin." << _T("depth_value") << " >= " << *filter.minDepth;
	}
	if ( filter.maxDepth ) {
		os << " and Origin." << _T("depth_value") << " <= " << *filter.maxDepth;
	}
	if ( filter.minMagnitude ) {
		os << " and Magnitude." << _T("magnitude_value") << " >= " << *filter.minMagnitude;
	}
	if ( filter.maxMagnitude ) {
		os << " and Magnitude." << _T("magnitude_value") << " <= " << *filter.maxMagnitude;
	}
	if ( filter.minPhaseCount ) {
		os << " and Origin." << _T("quality_usedPhaseCount") << " >= " << *filter.minPhaseCount;
	}
	if ( filter.maxPhaseCount ) {
		os << " and Origin." << _T("quality_usedPhaseCount") << " <= " << *filter.maxPhaseCount;
	}

	if ( !filter.eventID.empty() ) {
		// Convert to most common SQL LIKE format
		std::string pattern = filter.eventID;
		for ( auto &c : pattern ) {
			if ( c == '?' ) {
				c = '_';
			}
			else if ( c == '*' ) {
				c = '%';
			}
		}

		std::string escapedPattern;
		ar->driver()->escape(escapedPattern, pattern);

		os << " and PEvent." << _T("publicID") << " like '" << escapedPattern << "'";
	}
}


DatabaseIterator getEvents(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) {
		return {};
	}

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;

	oss << "select PEvent." + _T("publicID") + ",Event.* "
	    << "from Origin, PublicObject as POrigin, Event, PublicObject as PEvent ";

	if ( filterMagnitude ) {
		oss << ", PublicObject as PMagnitude,  Magnitude ";
	}

	oss << "where POrigin." + _T("publicID") + "=Event." + _T("preferredOriginID");

	if ( filterMagnitude ) {
		oss << " and "
		    << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID");
	}

	oss <<       " and "
	    <<       "Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "'";

	addFilterConstraints(oss, ar, filter);

	oss <<       " and "
	    <<       "Origin._oid=POrigin._oid and Event._oid=PEvent._oid";

	return ar->getObjectIterator(oss.str(), Event::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getEventOriginReferences(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) {
		return {};
	}

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select OriginReference.* "
	    << "from PublicObject as POrigin, Origin, "
	    << "OriginReference, Event ";

	if ( filterMagnitude ) {
		oss << ", PublicObject as PMagnitude,  Magnitude ";
	}

	oss << "where POrigin._oid = Origin._oid";

	if ( filterMagnitude ) {
		oss << " and "
		    << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID");
	}

	oss <<       " and "
	    <<       "Event." << _T("preferredOriginID") << " = POrigin." << _T("publicID") << " and "
	    <<       "Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "'";

	addFilterConstraints(oss, ar, filter);

	oss <<       " and "
	    <<       "OriginReference._parent_oid = Event._oid";

	return ar->getObjectIterator(oss.str(), OriginReference::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getEventFocalMechanismReferences(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) {
		return {};
	}

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select FocalMechanismReference.* "
	    << "from PublicObject as POrigin, Origin, "
	    << "FocalMechanismReference, Event ";

	if ( filterMagnitude ) {
		oss << ", PublicObject as PMagnitude,  Magnitude ";
	}

	oss << "where POrigin._oid = Origin._oid";

	if ( filterMagnitude ) {
		oss << " and "
		    << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID");
	}

	oss <<       " and "
	    <<       "Event." << _T("preferredOriginID") << " = POrigin." << _T("publicID") << " and "
	    <<       "Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "'";

	addFilterConstraints(oss, ar, filter);

	oss <<       " and "
	    <<       "FocalMechanismReference._parent_oid = Event._oid";

	return ar->getObjectIterator(oss.str(), FocalMechanismReference::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getEventOrigins(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) {
		return {};
	}

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select PAssocOrigin." << _T("publicID") << ", AssocOrigin.* "
	    << "from PublicObject as PAssocOrigin, Origin as AssocOrigin, "
	    <<      "Event, OriginReference, "
	    <<      "PublicObject as POrigin, Origin ";

	if ( filterMagnitude ) {
		oss << ", PublicObject as PMagnitude,  Magnitude ";
	}

	oss << "where PAssocOrigin._oid = AssocOrigin._oid and POrigin._oid = Origin._oid";

	if ( filterMagnitude ) {
		oss << " and "
		    << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID");
	}

	oss <<       " and "
	    <<       "Event." << _T("preferredOriginID") << " = POrigin." << _T("publicID") << " and "
	    <<       "Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "'";

	addFilterConstraints(oss, ar, filter);

	oss <<       " and "
	    <<       "OriginReference._parent_oid = Event._oid and "
	    <<       "OriginReference." << _T("originID") << " = PAssocOrigin." << _T("publicID");

	return ar->getObjectIterator(oss.str(), Origin::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getEventMagnitudes(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) {
		return {};
	}

	std::ostringstream oss;
	oss << "select PMagnitude." << _T("publicID") << ", Magnitude.* "
	    << "from PublicObject as PMagnitude, Magnitude, "
	    <<      "Event, PublicObject as POrigin, Origin "
	    << "where PMagnitude._oid = Magnitude._oid and POrigin._oid = Origin._oid and "
	    <<       "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID") << " and "
	    <<       "Event." << _T("preferredOriginID") << " = POrigin." << _T("publicID");

	addFilterConstraints(oss, ar, filter);

	oss <<       " and "
	    <<       "Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "'";

	return ar->getObjectIterator(oss.str(), Magnitude::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getEventPreferredOrigins(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) {
		return {};
	}

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select POrigin." << _T("publicID") << ", Origin.* "
	    << "from PublicObject as POrigin, Origin, Event";

	if ( filterMagnitude ) {
		oss << ", PublicObject as PMagnitude,  Magnitude";
	}

	oss << " where POrigin._oid = Origin._oid and "
	    <<       "Event." << _T("preferredOriginID") << " = POrigin." << _T("publicID");

	if ( filterMagnitude ) {
		oss << " and "
		       "PMagnitude._oid = Magnitude._oid and "
		       "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID");
	}

	addFilterConstraints(oss, ar, filter);

	oss <<       " and "
	    <<       "Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "'";

	return ar->getObjectIterator(oss.str(), Origin::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getEventFocalMechanisms(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) {
		return {};
	}

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select PFocalMechanism." << _T("publicID") << ", FocalMechanism.* "
	    << "from PublicObject as PFocalMechanism, FocalMechanism, "
	    <<      "Event, FocalMechanismReference, "
	    <<      "PublicObject as PPrefOrigin, Origin as PrefOrigin";

	if ( filterMagnitude ) {
		oss << ", PublicObject as PMagnitude,  Magnitude";
	}

	oss << " where PFocalMechanism._oid = FocalMechanism._oid and PPrefOrigin._oid = PrefOrigin._oid";


	if ( filterMagnitude ) {
		oss << " and "
		    << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID");
	}

	oss <<       " and "
	    <<       "Event." << _T("preferredOriginID") << " = PPrefOrigin." << _T("publicID") << " and "
	    <<       "PrefOrigin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "PrefOrigin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "'";

	addFilterConstraints(oss, ar, filter);

	oss <<       " and "
	    <<       "FocalMechanismReference._parent_oid = Event._oid and "
	    <<       "FocalMechanismReference." << _T("focalMechanismID") << " = PFocalMechanism." << _T("publicID");

	return ar->getObjectIterator(oss.str(), FocalMechanism::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getEventMomentTensors(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) {
		return {};
	}

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select PMomentTensor." << _T("publicID") << ", MomentTensor.* "
	    << "from PublicObject as PFocalMechanism, FocalMechanism, "
	    <<      "PublicObject as PMomentTensor, MomentTensor, "
	    <<      "Event, FocalMechanismReference, "
	    <<      "PublicObject as PPrefOrigin, Origin as PrefOrigin";

	if ( filterMagnitude ) {
		oss << ", PublicObject as PMagnitude,  Magnitude";
	}

	oss << " where PFocalMechanism._oid = FocalMechanism._oid and PMomentTensor._oid = MomentTensor._oid and "
	    <<       "PPrefOrigin._oid = PrefOrigin._oid";


	if ( filterMagnitude ) {
		oss << " and "
		    << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID");
	}

	oss <<       " and "
	    <<       "Event." << _T("preferredOriginID") << " = PPrefOrigin." << _T("publicID") << " and "
	    <<       "PrefOrigin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "PrefOrigin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "'";

	addFilterConstraints(oss, ar, filter);

	oss <<       " and "
	    <<       "FocalMechanismReference._parent_oid = Event._oid and "
	    <<       "FocalMechanismReference." << _T("focalMechanismID") << " = PFocalMechanism." << _T("publicID") << " and "
	    <<       "MomentTensor._parent_oid = FocalMechanism._oid";

	return ar->getObjectIterator(oss.str(), MomentTensor::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getUnassociatedOrigins(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) {
		return {};
	}

	std::ostringstream oss;

	oss << "select POrigin." << _T("publicID") << ", Origin.* "
	    << "from Origin, PublicObject as POrigin "
	    << "left join OriginReference on POrigin." << _T("publicID") << " = OriginReference." << _T("originID") << " "
	    << "where POrigin._oid = Origin._oid and "
	    <<       "Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "' and ";

	if ( filter.minLatitude ) {
		oss << "Origin." << _T("latitude_value") << " >= " << *filter.minLatitude << " and ";
	}
	if ( filter.maxLatitude ) {
		oss << "Origin." << _T("latitude_value") << " <= " << *filter.maxLatitude << " and ";
	}
	if ( filter.minLongitude ) {
		oss << "Origin." << _T("longitude_value") << " >= " << *filter.minLongitude << " and ";
	}
	if ( filter.maxLongitude ) {
		oss << "Origin." << _T("longitude_value") << " <= " << *filter.maxLongitude << " and ";
	}
	if ( filter.minDepth ) {
		oss << "Origin." << _T("depth_value") << " >= " << *filter.minDepth << " and ";
	}
	if ( filter.maxDepth ) {
		oss << "Origin." << _T("depth_value") << " <= " << *filter.maxDepth << " and ";
	}

	oss <<       "OriginReference." << _T("originID") << " is NULL";

	return ar->getObjectIterator(oss.str(), Origin::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getComments4Origins(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) {
		return {};
	}

	std::ostringstream oss;
	oss	<< "select Comment.* "
		<< "from Origin, "
		<<      "Comment "
		<< "where Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
		<<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime)   << "' and ";

	if ( filter.minLatitude ) {
		oss << "Origin." << _T("latitude_value") << " >= " << *filter.minLatitude << " and ";
	}
	if ( filter.maxLatitude ) {
		oss << "Origin." << _T("latitude_value") << " <= " << *filter.maxLatitude << " and ";
	}
	if ( filter.minLongitude ) {
		oss << "Origin." << _T("longitude_value") << " >= " << *filter.minLongitude << " and ";
	}
	if ( filter.maxLongitude ) {
		oss << "Origin." << _T("longitude_value") << " <= " << *filter.maxLongitude << " and ";
	}
	if ( filter.minDepth ) {
		oss << "Origin." << _T("depth_value") << " >= " << *filter.minDepth << " and ";
	}
	if ( filter.maxDepth ) {
		oss << "Origin." << _T("depth_value") << " <= " << *filter.maxDepth << " and ";
	}

	oss <<       "Comment._parent_oid = Origin._oid";

	return ar->getObjectIterator( oss.str(), Comment::TypeInfo() );
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getComments4Events(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if( !ar->driver() ) {
		return {};
	}

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select Comment.* "
		<< "from Event, "
		<<      "Origin, "
		<<      "PublicObject as POrigin, "
		<<      "Comment";

	if ( filterMagnitude ) {
		oss <<  ", PublicObject as PMagnitude,  Magnitude";
	}

	oss	<< " where Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
		<<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime)   << "'";

	if ( filterMagnitude ) {
		oss << " and "
		    << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID");
	}

	addFilterConstraints(oss, ar, filter);

	oss	<<       " and "
		<<       "Origin._oid = POrigin._oid and "
		<<       "POrigin." << _T("publicID") << " = Event." << _T("preferredOriginID") << " and "
		<<       "Comment._parent_oid = Event._oid";

	return ar->getObjectIterator( oss.str(), Comment::TypeInfo() );
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getComments4PrefOrigins(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if( !ar->driver() ) {
		return {};
	}

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select Comment.* "
		<< "from Event, "
		<<      "Origin, "
		<<      "PublicObject as POrigin, "
		<<      "Comment";

	if ( filterMagnitude ) {
		oss <<  ", PublicObject as PMagnitude,  Magnitude";
	}

	oss	<< " where Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
		<<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime)   << "'";

	if ( filterMagnitude ) {
		oss << " and "
		    << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID");
	}

	addFilterConstraints(oss, ar, filter);

	oss	<<       " and "
		<<       "Origin._oid = POrigin._oid and "
		<<       "POrigin." << _T("publicID") << " = Event." << _T("preferredOriginID") << " and "
		<<       "Comment._parent_oid = Origin._oid";

	return ar->getObjectIterator( oss.str(), Comment::TypeInfo() );
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator getDescriptions4Events(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if( !ar->driver() ) {
		return {};
	}

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select EventDescription.* "
		<< "from Event, "
		<<      "Origin, "
		<<      "PublicObject as POrigin, "
		<<      "EventDescription";

	if ( filterMagnitude ) {
		oss <<  ", PublicObject as PMagnitude,  Magnitude";
	}

	oss	<< " where Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
		<<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime)   << "'";

	if ( filterMagnitude ) {
		oss << " and "
		    << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID");
	}

	addFilterConstraints(oss, ar, filter);

	oss <<       " and "
		<<       "Origin._oid = POrigin._oid and "
		<<       "POrigin." << _T("publicID") << " = Event." << _T("preferredOriginID") << " and "
		<<       "EventDescription._parent_oid = Event._oid";

	 return ar->getObjectIterator( oss.str(), EventDescription::TypeInfo() );
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
using SortItem = QPair<QTreeWidgetItem*, int>;
using LessThan = bool(*)(const SortItem&, const SortItem&);

template <typename T>
bool itemLessThan(const SortItem& left, const SortItem& right) {
	return left.first->data(left.second, Qt::UserRole).value<T>() <
	       right.first->data(right.second, Qt::UserRole).value<T>();
}

template <typename T>
bool itemGreaterThan(const SortItem& left, const SortItem& right) {
	return left.first->data(left.second, Qt::UserRole).value<T>() >
	       right.first->data(right.second, Qt::UserRole).value<T>();
}

bool itemTextLessThan(const SortItem& left, const SortItem& right) {
	return left.first->text(left.second) < right.first->text(right.second);
}

bool itemTextGreaterThan(const SortItem& left, const SortItem& right) {
	return left.first->text(left.second) > right.first->text(right.second);
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Private {


enum SchemeType {
	ST_None = 0,
	ST_Event,
	ST_FocalMechanism,
	ST_Origin,
	ST_OriginGroup,
	ST_FocalMechanismGroup
};


class TreeItem : public QTreeWidgetItem {
	public:
		explicit TreeItem(int type, const EventListViewPrivate::ItemConfig &cfg)
		: QTreeWidgetItem(type), _enabled(true), config(cfg) {}
		explicit TreeItem(QTreeWidget *view, int type, const EventListViewPrivate::ItemConfig &cfg)
		: QTreeWidgetItem(view, type), _enabled(true), config(cfg) {}
		explicit TreeItem(QTreeWidgetItem *parent, int type, const EventListViewPrivate::ItemConfig &cfg)
		: QTreeWidgetItem(parent, type), _enabled(true), config(cfg) {}

		virtual void setEnabled(bool e) {
			if ( _enabled == e ) {
				return;
			}
			_enabled = e;

			/*
			if ( e )
				setFlags(flags() | Qt::ItemIsEnabled);
			else
				setFlags(flags() & ~Qt::ItemIsEnabled);
			*/
		}

		[[nodiscard]]
		bool isEnabled() const { return _enabled; }

		[[nodiscard]]
		QVariant data(int column, int role) const override {
			if ( !_enabled && role == Qt::ForegroundRole ) {
				return config.disabledColor;
			}
			return QTreeWidgetItem::data(column, role);
		}

	private:
		bool _enabled;

	protected:
		const EventListViewPrivate::ItemConfig &config;
};


class SchemeTreeItem : public TreeItem {
	protected:
		SchemeTreeItem(int type, PublicObject* object,
		               const EventListViewPrivate::ItemConfig &cfg,
		               QTreeWidgetItem * parent = nullptr)
		: TreeItem(parent, type, cfg), _object(object) { init(); }

	public:
		void init() {
			setTextAlignment(config.columnMap[COL_ID], Qt::AlignLeft | Qt::AlignVCenter);
			setTextAlignment(config.columnMap[COL_OTIME], Qt::AlignLeft | Qt::AlignVCenter);
			setTextAlignment(config.columnMap[COL_TIME_AGO], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_TYPE], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_FM], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_PHASES], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_ORIGINS], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_RMS], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_AZIMUTHAL_GAP], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_M], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_MTYPE], Qt::AlignLeft | Qt::AlignVCenter);
			//setTextAlignment(MCOUNT, Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_LAT], Qt::AlignRight | Qt::AlignVCenter);
			setTextAlignment(config.columnMap[COL_LON], Qt::AlignRight | Qt::AlignVCenter);
			setTextAlignment(config.columnMap[COL_DEPTH], Qt::AlignRight | Qt::AlignVCenter);
			setTextAlignment(config.columnMap[COL_DEPTH_TYPE], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_REGION], Qt::AlignLeft | Qt::AlignVCenter);

			if ( config.customColumn != -1 ) {
				setTextAlignment(config.customColumn, Qt::AlignLeft);
			}

			for ( const auto &col : config.originScriptColumns ) {
				setTextAlignment(col.pos, Qt::AlignCenter);
			}

			for ( const auto &col : config.eventScriptColumns ) {
				setTextAlignment(col.pos, Qt::AlignCenter);
			}
		}

		virtual void update(EventListView*) = 0;

		void updateTimeAgo() {
			bool ok = true;
			double epoch = data(config.columnMap[COL_OTIME], Qt::UserRole).toDouble(&ok);

			if ( !ok ) {
				setText(config.columnMap[COL_TIME_AGO], "");
				setData(config.columnMap[COL_TIME_AGO], Qt::UserRole, QVariant());
				return;
			}

			TimeSpan ts = Time::UTC() - Time(epoch);

			int sec = ts.seconds();
			int days = sec / 86400;
			int hours = (sec - days * 86400) / 3600;
			int minutes = (sec - days * 86400 - hours * 3600) / 60;
			int seconds = sec - days * 86400 - hours * 3600 - 60 * minutes;

			QString text;

			if ( days > 0 ) {
				text = QString("%1d %2h").arg(days).arg(hours);
			}
			else {
				if ( hours > 0 ) {
					text = QString("%1h %2m").arg(hours).arg(minutes);
				}
				else {
					if ( minutes > 0 ) {
						text = QString("%1m %2s").arg(minutes).arg(seconds);
					}
					else {
						text = QString("%1s").arg(seconds);
					}
				}
			}

			setText(config.columnMap[COL_TIME_AGO], text);
			setData(config.columnMap[COL_TIME_AGO], Qt::UserRole, QVariant(ts.length()));

			// background and foreground color if color gradient is specified
			if ( !config.otimeAgo.gradient.empty() ) {
				QColor bg = config.otimeAgo.gradient.colorAt(
				        ts.seconds(), config.otimeAgo.discrete);

				// configured background color contains no alpha and has no
				// effect
				if ( bg.alpha() == 0 ) {
					setBackground(config.columnMap[COL_TIME_AGO], Qt::NoBrush);
					setForeground(config.columnMap[COL_TIME_AGO], Qt::NoBrush);
					return;
				}

				// Set foreground color either to white or black depending on
				// the gray value of the background. The effective background
				// gray value depends on the color value of the configured color
				// gradient, its alpha value and the default background.
				// Open issue: Check if the current row is using
				// QPalette::AlternateBase instead of QPalette::Base.
				int defaultBGGray = qGray(SCApp->palette(). color(QPalette::Normal, QPalette::Base).rgb());
				int bgGray = qGray(bg.rgb());
				double alpha = static_cast<double>(bg.alpha()) / 256.0;
				int effectiveBGGray = bgGray * alpha + defaultBGGray * (1.0 - alpha);

				/*
				if ( ts.length() < 3600 ) {
					std::cerr << "defaultBGGray: " << defaultBGGray
					          << ", s: " << ts.length()
					          << ", alpha: " << alpha
					          << ", bgGray: " << bgGray
					          << ", effectiveBGGray: " << effectiveBGGray
					          << std::endl;
				}
				*/

				setBackground(config.columnMap[COL_TIME_AGO], bg);
				setForeground(config.columnMap[COL_TIME_AGO],
				              effectiveBGGray < 128 ? Qt::white : Qt::black);
			}
		}

		[[nodiscard]]
		PublicObject* object() const { return _object.get(); }

	private:
		PublicObjectPtr _object;
};


class OriginTreeItem : public SchemeTreeItem {
	public:
		OriginTreeItem(Origin *origin,
		               const EventListViewPrivate::ItemConfig &config,
		               QTreeWidgetItem *parent = nullptr)
		: SchemeTreeItem(ST_Origin, origin, config, parent) {
			QFont f = font(config.columnMap[COL_REGION]);
			f.setItalic(true);
			setFont(config.columnMap[COL_REGION], f);

			if ( TimeFormat.empty() ) {
				TimeFormat = "... %T";
				if ( SCScheme.precision.originTime > 0 ) {
					TimeFormat += ".%";
					TimeFormat += Core::toString(SCScheme.precision.originTime);
					TimeFormat += "f";
				}
			}

			update(nullptr);
		}

		~OriginTreeItem() override = default;
		/*
		{
			if ( origin() )
				std::cout << "removed origin " << origin()->publicID() << " from list" << std::endl;
			else
				std::cout << "removed empty origin item from list" << std::endl;
		}
		*/

		[[nodiscard]]
		Origin* origin() const { return static_cast<Origin*>(object()); }

		void setPublishState(bool ps) {
			_published = ps;
		}

		void update(EventListView */*unused*/) override {
			auto *ori = origin();
			setText(config.columnMap[COL_ID], QString("%1").arg(ori->publicID().c_str()));
			POPULATE_AGENCY(ori->creationInfo().agencyID());
			try {
				setText(config.columnMap[COL_AUTHOR], ori->creationInfo().author().c_str());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_AUTHOR], "");
			}
			setText(config.columnMap[COL_OTIME], timeToString(ori->time().value(), TimeFormat.c_str()));
			setData(config.columnMap[COL_OTIME], Qt::UserRole, QVariant(static_cast<double>(ori->time().value())));
			setText(config.columnMap[COL_M], "-"); // Mag
			setText(config.columnMap[COL_MTYPE], "-"); // MagType
			//setText(MCOUNT, "-"); // MagCount
			try {
				setText(config.columnMap[COL_PHASES], QString("%1").arg(ori->quality().usedPhaseCount()));
				setData(config.columnMap[COL_PHASES], Qt::UserRole, ori->quality().usedPhaseCount());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_PHASES], "-");
				setData(config.columnMap[COL_PHASES], Qt::UserRole, QVariant());
			}

			try {
				setText(config.columnMap[COL_RMS], QString("%1").arg(ori->quality().standardError(), 0, 'f', SCScheme.precision.rms));
				setData(config.columnMap[COL_RMS], Qt::UserRole, ori->quality().standardError());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_RMS], "-");
				setData(config.columnMap[COL_RMS], Qt::UserRole, QVariant());
			}

			try {
				setText(config.columnMap[COL_AZIMUTHAL_GAP], QString("%1").arg(ori->quality().azimuthalGap(), 0, 'f', 0));
				setData(config.columnMap[COL_AZIMUTHAL_GAP], Qt::UserRole, ori->quality().azimuthalGap());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_AZIMUTHAL_GAP], "-");
				setData(config.columnMap[COL_AZIMUTHAL_GAP], Qt::UserRole, QVariant());
			}

			double lat = ori->latitude();
			double lon = ori->longitude();

			setText(config.columnMap[COL_LAT], QString("%1 %2").arg(fabs(lat), 0, 'f', SCScheme.precision.location).arg(lat < 0?"S":"N")); // Lat
			setData(config.columnMap[COL_LAT], Qt::UserRole, lat);
			setText(config.columnMap[COL_LON], QString("%1 %2").arg(fabs(lon), 0, 'f', SCScheme.precision.location).arg(lon < 0?"W":"E")); // Lon
			setData(config.columnMap[COL_LON], Qt::UserRole, lon);

			try {
				setText(config.columnMap[COL_DEPTH], depthToString(ori->depth(), SCScheme.precision.depth) + " km");
				setData(config.columnMap[COL_DEPTH], Qt::UserRole, ori->depth().value());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_DEPTH], "-"); // Depth
				setData(config.columnMap[COL_DEPTH], Qt::UserRole, QVariant());
			}

			try {
				setText(config.columnMap[COL_DEPTH_TYPE], ori->depthType().toString());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_DEPTH_TYPE], "-"); // Depth
			}

			char stat = objectStatusToChar(ori);
			setText(config.columnMap[COL_TYPE],
					QString("%1%2")
					.arg(_published?">":"")
					.arg(stat)
					); // Type

			try {
				switch ( ori->evaluationMode() ) {
					case DataModel::AUTOMATIC:
						setForeground(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
						break;
					case DataModel::MANUAL:
						setForeground(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.manual);
						break;
					default:
						break;
				};
			}
			catch ( ... ) {
				setForeground(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
			}

			setText(config.columnMap[COL_REGION], Regions::getRegionName(lat,lon).c_str()); // Region
			//setText(2, QString("%1").arg(origin->arrivalCount()));

			if ( config.customColumn != -1 ) {
				setText(config.customColumn, config.customDefaultText);
				setToolTip(config.customColumn, config.customDefaultText);
				setData(config.customColumn, Qt::ForegroundRole, {});
				if ( !config.originCommentID.empty() ) {
					for ( size_t i = 0; i < ori->commentCount(); ++i ) {
						if ( ori->comment(i)->id() == config.originCommentID ) {
							setText(config.customColumn, ori->comment(i)->text().c_str());
							setToolTip(config.customColumn, ori->comment(i)->text().c_str());
							QMap<std::string, QColor>::const_iterator it =
								config.customColorMap.find(ori->comment(i)->text());
							if ( it != config.customColorMap.end() ) {
								setData(config.customColumn, Qt::ForegroundRole, it.value());
							}
							break;
						}
					}
				}
			}

			setToolTip(config.columnMap[COL_OTIME], timeToString(ori->time().value(), "%F %T.%f", true));
			setToolTip(config.columnMap[COL_ID], text(config.columnMap[COL_ID]));
			setToolTip(config.columnMap[COL_REGION], text(config.columnMap[COL_REGION])); // Region ToolTip

			updateTimeAgo();
		}


	protected:
		void setHighlight(bool highlight) {
			QFont f = font(0);
			f.setBold(highlight);
			setFont(0,f);

			setData(config.columnMap[COL_ID], Qt::UserRole, highlight);

			/*
			QTreeWidgetItem * p = parent();
			if ( p && highlight ) {
				bool expanded = false;
				QTreeWidget* tree = treeWidget();
				if ( tree )
					expanded = p->isExpanded();
				//SEISCOMP_DEBUG("Reposition child");
				p->insertChild(0, p->takeChild(p->indexOfChild(this)));
				if ( !expanded && tree )
					tree->collapseItem(p);
			}
			*/
		}

	private:
		bool _published{false};

	public:
		static std::string TimeFormat;

	friend class EventTreeItem;
};


std::string OriginTreeItem::TimeFormat;


class FocalMechanismTreeItem : public SchemeTreeItem {
	public:
		FocalMechanismTreeItem(FocalMechanism *origin,
		                       const EventListViewPrivate::ItemConfig &config,
		                       QTreeWidgetItem *parent = nullptr)
		  : SchemeTreeItem(ST_FocalMechanism, origin, config, parent) {
			update(nullptr);
		}

		~FocalMechanismTreeItem() override = default;

		[[nodiscard]]
		FocalMechanism* focalMechanism() const { return static_cast<FocalMechanism*>(object()); }

		void setPublishState(bool ps) {
			_published = ps;
		}

		void update(EventListView */*unused*/) override {
			FocalMechanism* fm = focalMechanism();
			setText(config.columnMap[COL_ID], QString("%1").arg(fm->publicID().c_str()));
			POPULATE_AGENCY(fm->creationInfo().agencyID());
			try {
				setText(config.columnMap[COL_AUTHOR], fm->creationInfo().author().c_str());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_AUTHOR], "");
			}

			Origin *fmBaseOrg;

			if ( fm->momentTensorCount() > 0 ) {
				MomentTensor *mt = fm->momentTensor(0);
				fmBaseOrg = Origin::Find(mt->derivedOriginID());

				Magnitude *momentmag = Magnitude::Find(mt->momentMagnitudeID());
				if ( momentmag ) {
					setText(config.columnMap[COL_M], QString("%1").arg(momentmag->magnitude().value(), 0, 'f', SCScheme.precision.magnitude));
					setData(config.columnMap[COL_M], Qt::UserRole, QVariant(momentmag->magnitude().value()));
					setText(config.columnMap[COL_MTYPE], QString("%1").arg(momentmag->type().c_str()));
				}
				else {
					setText(config.columnMap[COL_M], "-"); // Mag
					setText(config.columnMap[COL_MTYPE], "-"); // MagType
				}
			}
			else {
				fmBaseOrg = Origin::Find(fm->triggeringOriginID());
			}

			if ( fmBaseOrg ) {
				setText(config.columnMap[COL_OTIME], timeToString(fmBaseOrg->time().value(), OriginTreeItem::TimeFormat.c_str()));
				setData(config.columnMap[COL_OTIME], Qt::UserRole, QVariant(static_cast<double>(fmBaseOrg->time().value())));

				try {
					setText(config.columnMap[COL_PHASES], QString("%1").arg(fmBaseOrg->quality().usedPhaseCount()));
				}
				catch ( ... ) {
					setText(config.columnMap[COL_PHASES], "-");
				}

				try {
					setText(config.columnMap[COL_RMS], QString("%1").arg(fmBaseOrg->quality().standardError()));
				}
				catch ( ... ) {
					setText(config.columnMap[COL_RMS], "-");
				}

				try {
					setText(config.columnMap[COL_AZIMUTHAL_GAP], QString("%1").arg(fmBaseOrg->quality().azimuthalGap(), 0, 'f', 0));
				}
				catch ( ... ) {
					setText(config.columnMap[COL_AZIMUTHAL_GAP], "-");
				}

				double lat = fmBaseOrg->latitude();
				double lon = fmBaseOrg->longitude();

				setText(config.columnMap[COL_LAT], QString("%1 %2").arg(fabs(lat), 0, 'f', SCScheme.precision.location).arg(lat < 0?"S":"N")); // Lat
				setText(config.columnMap[COL_LON], QString("%1 %2").arg(fabs(lon), 0, 'f', SCScheme.precision.location).arg(lon < 0?"W":"E")); // Lon

				try {
					setText(config.columnMap[COL_DEPTH], depthToString(fmBaseOrg->depth(), SCScheme.precision.depth) + " km");
				}
				catch ( ... ) {
					setText(config.columnMap[COL_DEPTH], "-"); // Depth
				}

				try {
					setText(config.columnMap[COL_DEPTH_TYPE], fmBaseOrg->depthType().toString());
				}
				catch ( ... ) {
					setText(config.columnMap[COL_DEPTH_TYPE], "-"); // Depth
				}

				setText(config.columnMap[COL_REGION], Regions::getRegionName(lat, lon).c_str()); // Region
			}

			updateTimeAgo();

			char stat = objectStatusToChar(fm);
			setText(config.columnMap[COL_TYPE], QString("%1").arg(stat));

			try {
				switch ( fm->evaluationMode() ) {
					case DataModel::AUTOMATIC:
						setForeground(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
						break;
					case DataModel::MANUAL:
						setForeground(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.manual);
						break;
					default:
						break;
				};
			}
			catch ( ... ) {
				setForeground(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
			}
		}


	protected:
		void setHighlight(bool highlight) {
			QFont f = font(0);
			f.setBold(highlight);
			setFont(0,f);

			setData(config.columnMap[COL_ID], Qt::UserRole, highlight);
		}
	private:
		bool _published{false};

	friend class EventTreeItem;
};


class EventTreeItem : public SchemeTreeItem {
	public:
		EventTreeItem(Event* event, const EventListViewPrivate::ItemConfig &config,
		              QTreeWidgetItem * parent = nullptr)
		  : SchemeTreeItem(ST_Event, event, config, parent) {
			_showOnlyOnePerAgency = false;
			_resort = false;
			_hasMultipleAgencies = false;
			_published = false;

			setText(config.columnMap[COL_PHASES], "-");
			setText(config.columnMap[COL_RMS], "-");
			setText(config.columnMap[COL_AZIMUTHAL_GAP], "-");
			setText(config.columnMap[COL_M], "-");
			setText(config.columnMap[COL_MTYPE], "-");
			setText(config.columnMap[COL_DEPTH], "-");
			setText(config.columnMap[COL_DEPTH_TYPE], "-");

			QFont f = SCApp->font();
			f.setUnderline(true);
			setData(config.columnMap[COL_FM], Qt::FontRole, f);
			setData(config.columnMap[COL_FM], Qt::ForegroundRole, SCApp->palette().color(QPalette::Link));

			_origins = nullptr;
			_focalMechanisms = nullptr;

			if ( TimeFormat.empty() ) {
				TimeFormat = "%F %T";
				if ( SCScheme.precision.originTime > 0 ) {
					TimeFormat += ".%";
					TimeFormat += Core::toString(SCScheme.precision.originTime);
					TimeFormat += "f";
				}
			}

			update(nullptr);
		}

		~EventTreeItem() override = default;
		/*
		{
			if ( event() )
				std::cout << "removed event " << event()->publicID() << " from list" << std::endl;
			else
				std::cout << "removed empty event item from list" << std::endl;
			//if ( _origins ) delete _origins;
			//if ( _focalMechanisms ) delete _focalMechanisms;
		}
		*/

		[[nodiscard]]
		Event* event() const { return static_cast<Event*>(object()); }

		[[nodiscard]]
		QTreeWidgetItem *origins() const { return _origins; }

		[[nodiscard]]
		int originItemCount() const {
			return _origins?_origins->childCount():0;
		}

		[[nodiscard]]
		QTreeWidgetItem *originItem(int i) const {
			return _origins?_origins->child(i):nullptr;
		}

		[[nodiscard]]
		QTreeWidgetItem *takeOrigin(int i) const {
			return _origins?_origins->takeChild(i):nullptr;
		}

		void addOriginItem(QTreeWidgetItem *item) {
			if ( !_origins ) {
				_origins = new TreeItem(this, ST_OriginGroup, config);
				_origins->setEnabled(isEnabled());
				_origins->setFlags(_origins->flags() & ~Qt::ItemIsDragEnabled);
				QFont f = _origins->font(0);
				f.setItalic(true);
				_origins->setFont(0, f);
				_origins->setText(0, "Origins");
			}

			_origins->addChild(item);
		}

		void addOriginItem(int i, QTreeWidgetItem *item) {
			if ( !_origins ) {
				_origins = new TreeItem(this, ST_OriginGroup, config);
				_origins->setEnabled(isEnabled());
				_origins->setFlags(_origins->flags() & ~Qt::ItemIsDragEnabled);
				QFont f = _origins->font(0);
				f.setItalic(true);
				_origins->setFont(0, f);
				_origins->setText(0, "Origins");
			}

			_origins->insertChild(i, item);
		}

		[[nodiscard]]
		QTreeWidgetItem *focalMechanisms() const { return _focalMechanisms; }

		[[nodiscard]]
		int focalMechanismItemCount() const {
			return _focalMechanisms?_focalMechanisms->childCount():0;
		}

		[[nodiscard]]
		QTreeWidgetItem *focalMechanismItem(int i) const {
			return _focalMechanisms?_focalMechanisms->child(i):nullptr;
		}

		QTreeWidgetItem *takeFocalMechanism(int i) {
			if ( !_focalMechanisms ) {
				return nullptr;
			}

			auto *item = _focalMechanisms->takeChild(i);
			if ( _focalMechanisms->childCount() == 0 ) {
				delete _focalMechanisms;
				_focalMechanisms = nullptr;
			}
			return item;
		}

		void addFocalMechanismItem(QTreeWidgetItem *item) {
			if ( !_focalMechanisms ) {
				_focalMechanisms = new TreeItem(this, ST_FocalMechanismGroup, config);
				_focalMechanisms->setEnabled(isEnabled());
				_focalMechanisms->setFlags(_focalMechanisms->flags() & ~Qt::ItemIsDragEnabled);
				QFont f = _origins->font(0);
				f.setItalic(true);
				_focalMechanisms->setFont(0, f);
				_focalMechanisms->setText(0, "FocalMechanisms");
			}

			_focalMechanisms->addChild(item);
		}

		void addFocalMechanismItem(int i, QTreeWidgetItem *item) {
			if ( !_focalMechanisms ) {
				_focalMechanisms = new TreeItem(this, ST_FocalMechanismGroup, config);
				_focalMechanisms->setEnabled(isEnabled());
				_focalMechanisms->setFlags(_focalMechanisms->flags() & ~Qt::ItemIsDragEnabled);
				QFont f = _focalMechanisms->font(0);
				f.setItalic(true);
				_focalMechanisms->setFont(0, f);
				_focalMechanisms->setText(0, "FocalMechanisms");
			}

			_focalMechanisms->insertChild(0, item);
		}

		void setShowOneItemPerAgency(bool e) {
			if ( _showOnlyOnePerAgency != e ) {
				_showOnlyOnePerAgency = e;
				updateHideState();
			}
		}

		void setPublishState(bool p) {
			_published = p;

			/*
			setData(TYPE, Qt::BackgroundRole, _published?Qt::green:QVariant());
			QFont f = font(TYPE);
			f.setBold(_published);
			setFont(TYPE, f);
			*/
		}

		void resort() {
			_resort = true;
		}

		void updateHideState() {
			if ( _origins ) {
				if ( _showOnlyOnePerAgency ) {
					QMap<QString, QTreeWidgetItem*> seenAgencies;

					for ( int i = 0; i < _origins->childCount(); ++i ) {
						auto *oitem = static_cast<OriginTreeItem*>(_origins->child(i));
						QString agency = oitem->text(config.columnMap[COL_AGENCY]);

						auto it = seenAgencies.find(agency);
						bool hide = false;

						// Not the first origin this agency
						if ( it != seenAgencies.end() ) {
							// Is it the preferred origin?
							if ( oitem->data(config.columnMap[COL_ID], Qt::UserRole).toBool() ) {
								it.value()->setHidden(true);
							}
							else {
								hide = true;
							}
						}
						else {
							seenAgencies.insert(agency, oitem);
						}

						if ( oitem->isHidden() != hide ) {
							oitem->setHidden(hide);
						}
					}
				}
				else {
					for ( int i = 0; i < _origins->childCount(); ++i ) {
						if ( _origins->child(i)->isHidden() ) {
							_origins->child(i)->setHidden(false);
						}
					}
				}
			}

			if ( _focalMechanisms ) {
				if ( _showOnlyOnePerAgency ) {
					QMap<QString, QTreeWidgetItem*> seenAgencies;

					for ( int i = 0; i < _focalMechanisms->childCount(); ++i ) {
						auto *fmitem = static_cast<FocalMechanismTreeItem*>(_focalMechanisms->child(i));
						QString agency = fmitem->text(config.columnMap[COL_AGENCY]);

						QMap<QString, QTreeWidgetItem*>::iterator it = seenAgencies.find(agency);

						bool hide = false;

						// Not the first origin this agency
						if ( it != seenAgencies.end() ) {
							// Is it the preferred origin?
							if ( fmitem->data(config.columnMap[COL_ID], Qt::UserRole).toBool() ) {
								it.value()->setHidden(true);
							}
							else {
								hide = true;
							}
						}
						else {
							seenAgencies.insert(agency, fmitem);
						}

						if ( fmitem->isHidden() != hide ) {
							fmitem->setHidden(hide);
						}
					}
				}
				else {
					for ( int i = 0; i < _origins->childCount(); ++i ) {
						if ( _origins->child(i)->isHidden() ) {
							_origins->child(i)->setHidden(false);
						}
					}
				}
			}
		}

		void update(EventListView *view) override {
			auto *ev = event();
			if ( !ev ) {
				setText(config.columnMap[COL_ID], "<>");
				setText(config.columnMap[COL_OTIME], "Unassociated");
				return;
			}

			if ( _resort && _origins ) {
				_hasMultipleAgencies = false;

				// Reset origin process columns
				for ( const auto &col : config.originScriptColumns) {
					int pos = col.pos;
					if ( config.eventScriptPositions.contains(pos) ) {
						continue;
					}

					setBackground(pos, Qt::NoBrush);
					setForeground(pos, Qt::NoBrush);
					setText(pos, "");
					setToolTip(pos, "");
				}

				// Preferred origin changed => resort origins
				auto childs = _origins->takeChildren();
				if ( !childs.empty() ) {
					std::stable_sort(childs.begin(), childs.end(), originItemLessThan);

					QString firstAgency;

					for ( auto it = childs.begin(); it != childs.end(); ++it ) {
						auto *oi = static_cast<OriginTreeItem*>(*it);

						if ( it == childs.begin() ) {
							firstAgency = oi->text(config.columnMap[COL_AGENCY]);
						}
						else if ( firstAgency != oi->text(config.columnMap[COL_AGENCY]) ) {
							_hasMultipleAgencies = true;
						}

						oi->setHighlight(false);
					}

					for ( auto it = childs.begin(); it != childs.end(); ++it ) {
						auto *oi = static_cast<OriginTreeItem*>(*it);
						if ( oi->object()->publicID() == ev->preferredOriginID() ) {
							oi->setHighlight(true);

							// Copy item states from preferred origin item
							// if column is not part of event script columns
							for ( const auto &col : config.originScriptColumns ) {
								int pos = col.pos;
								if ( config.eventScriptPositions.contains(pos) ) {
									continue;
								}

								setText(pos, oi->text(pos));
								setBackground(pos, oi->background(pos));
								setForeground(pos, oi->foreground(pos));
								setToolTip(pos, oi->toolTip(pos));
							}

							childs.erase(it);
							childs.push_front(oi);
							break;
						}
					}

					_origins->insertChildren(0, childs);

					if ( _showOnlyOnePerAgency ) {
						updateHideState();
					}
				}
			}

			if ( _resort && _focalMechanisms ) {
				// Preferred origin changed => resort origins
				auto childs = _focalMechanisms->takeChildren();
				if ( !childs.empty() ) {
					std::stable_sort(childs.begin(), childs.end(), fmItemLessThan);

					for ( auto &child : childs ) {
						auto *fmi = static_cast<FocalMechanismTreeItem*>(child);
						fmi->setHighlight(false);
					}

					for ( auto it = childs.begin(); it != childs.end(); ++it ) {
						auto *fmi = static_cast<FocalMechanismTreeItem*>(*it);
						if ( fmi->object()->publicID() == ev->preferredFocalMechanismID() ) {
							fmi->setHighlight(true);
							childs.erase(it);
							childs.push_front(fmi);
							break;
						}
					}

					_focalMechanisms->insertChildren(0, childs);

					if ( _showOnlyOnePerAgency ) {
						updateHideState();
					}
				}
			}

			_resort = false;

			try {
				setText(config.columnMap[COL_EVENTTYPE], ev->type().toString());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_EVENTTYPE], "");
			}

			try {
				setText(config.columnMap[COL_EVENTTYPE_CERTAINTY], ev->typeCertainty().toString());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_EVENTTYPE_CERTAINTY], "");
			}

			if ( ev->preferredFocalMechanismID().empty() && !ev->focalMechanismReferenceCount() ) {
				setData(config.columnMap[COL_FM], Qt::DisplayRole, QVariant());
				setData(config.columnMap[COL_FM], Qt::ToolTipRole, QVariant());
				setData(config.columnMap[COL_FM], Qt::UserRole + 1, QVariant());
			}
			else if ( treeWidget() && view ) {
				if ( ev->focalMechanismReferenceCount() > 0 ) {
					setText(config.columnMap[COL_FM], QString("%1").arg(ev->focalMechanismReferenceCount()));
				}
				else {
					setText(config.columnMap[COL_FM], QObject::tr("Yes"));
				}
				if ( !ev->preferredFocalMechanismID().empty() ) {
					QFont f = font(config.columnMap[COL_FM]);
					f.setBold(true);
					setData(config.columnMap[COL_FM], Qt::FontRole, f);
				}
				else {
					setData(config.columnMap[COL_FM], Qt::FontRole, QVariant());
				}
				setData(config.columnMap[COL_FM], Qt::ToolTipRole, QObject::tr("Load event and open the focal mechanism tab"));
				setData(config.columnMap[COL_FM], Qt::UserRole + 1, QVariant::fromValue<void*>(ev));
			}

			auto *origin = Origin::Find(ev->preferredOriginID());
			auto *nm = Magnitude::Find(ev->preferredMagnitudeID());

			setText(config.columnMap[COL_ID], QString("%1").arg(ev->publicID().c_str()));
			setText(config.columnMap[COL_REGION], QString("%1").arg(eventRegion(ev).c_str()));
			setText(config.columnMap[COL_ORIGINS], QString("%1").arg(ev->originReferenceCount()));
			setData(config.columnMap[COL_ORIGINS], Qt::UserRole, QVariant::fromValue<unsigned>(ev->originReferenceCount()));

			if ( nm ) {
				QFont f = font(config.columnMap[COL_M]);
				f.setBold(true);
				setFont(config.columnMap[COL_M],f);
				setText(config.columnMap[COL_M], QString("%1").arg(QString("%1").arg(nm->magnitude().value(), 0, 'f', SCScheme.precision.magnitude)));
				setData(config.columnMap[COL_M], Qt::UserRole, QVariant(nm->magnitude().value()));
				setText(config.columnMap[COL_MTYPE], QString("%1").arg(nm->type().c_str()));

				/*
					//! display the station Count of a magnitude
					try {
						int staCount = nm->stationCount();
						setText(MCOUNT, QString("%1").arg(staCount, 0, 'd', 0, ' '));
					}
					catch(...){
						setText(MCOUNT, QString("-"));
					}
					//! -----------------------------------------------------
					*/
			}
			else if ( ev->preferredMagnitudeID().empty() ) {
				QFont f = font(config.columnMap[COL_M]);
				f.setBold(false);
				setFont(config.columnMap[COL_M],f);
				setText(config.columnMap[COL_M], "-");
				setText(config.columnMap[COL_MTYPE], "-"); // stationCount
				//setText(MCOUNT, "-"); // stationCount
			}

			//! this lines are for displaying defining Phase Count of an origin
			if ( origin ) {
				POPULATE_AGENCY(origin->creationInfo().agencyID());

				try {
					setText(config.columnMap[COL_AUTHOR], origin->creationInfo().author().c_str());
				}
				catch ( ValueException& ) {
					setText(config.columnMap[COL_AUTHOR], "");
				}

				int column = config.columnMap[COL_OTIME];
				setText(column, timeToString(origin->time().value(), TimeFormat.c_str()));
				setData(column, Qt::UserRole, QVariant(static_cast<double>(origin->time().value())));

				double lat = origin->latitude();
				double lon = origin->longitude();

				column = config.columnMap[COL_LAT];
				setText(column, QString("%1 %2").arg(fabs(lat), 0, 'f', SCScheme.precision.location).arg(lat < 0?"S":"N")); // Lat
				setData(column, Qt::UserRole, lat);

				column = config.columnMap[COL_LON];
				setText(column, QString("%1 %2").arg(fabs(lon), 0, 'f', SCScheme.precision.location).arg(lon < 0?"W":"E")); // Lon
				setData(column, Qt::UserRole, lon);

				column = config.columnMap[COL_DEPTH];
				try {
					setText(column, QString("%1 km").arg(depthToString(origin->depth(), SCScheme.precision.depth))); // Depth
					setData(column, Qt::UserRole, origin->depth().value());
				}
				catch ( ValueException& ) {
					setText(column, "-");
					setData(column, Qt::UserRole, QVariant());
				}

				column = config.columnMap[COL_DEPTH_TYPE];
				try {
					setText(column, origin->depthType().toString()); // Depth type
				}
				catch ( ValueException& ) {
					setText(column, "-");
				}

				char stat = objectStatusToChar(origin);
				setText(config.columnMap[COL_TYPE],
				        QString("%1%2%3")
				        .arg(_published?"*":"")
				        .arg(stat)
				        .arg(_hasMultipleAgencies?"+":"")
				        ); // Type
				try {
					switch ( origin->evaluationMode() ) {
						case DataModel::AUTOMATIC:
							setForeground(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
							break;
						case DataModel::MANUAL:
							setForeground(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.manual);
							break;
						default:
							break;
					};
				}
				catch ( ValueException& ) {
					setForeground(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
				}

				try{
					const OriginQuality &quality = origin->quality();
					setText(config.columnMap[COL_PHASES], QString("%1").arg(quality.usedPhaseCount()));
					setData(config.columnMap[COL_PHASES], Qt::UserRole, static_cast<double>(quality.usedPhaseCount()));
				}
				catch ( ValueException& ) {
					setText(config.columnMap[COL_PHASES], "-");
					setData(config.columnMap[COL_PHASES], Qt::UserRole, QVariant());
				}

				try {
					const OriginQuality &quality = origin->quality();
					setText(config.columnMap[COL_RMS], QString("%1").arg(quality.standardError(), 0, 'f', SCScheme.precision.rms));
					setData(config.columnMap[COL_RMS], Qt::UserRole, quality.standardError());
				}
				catch ( ValueException& ) {
					setText(config.columnMap[COL_RMS], "-");
					setData(config.columnMap[COL_RMS], Qt::UserRole, QVariant());
				}

				try {
					const OriginQuality &quality = origin->quality();
					setText(config.columnMap[COL_AZIMUTHAL_GAP], QString("%1").arg(quality.azimuthalGap(), 0, 'f', 0));
					setData(config.columnMap[COL_AZIMUTHAL_GAP], Qt::UserRole, quality.azimuthalGap());
				}
				catch ( ValueException& ) {
					setText(config.columnMap[COL_AZIMUTHAL_GAP], "-");
					setData(config.columnMap[COL_AZIMUTHAL_GAP], Qt::UserRole, QVariant());
				}

				if ( config.customColumn != -1 ) {
					setData(config.customColumn, Qt::ForegroundRole, QVariant());
					setText(config.customColumn, config.customDefaultText);
					setToolTip(config.customColumn, config.customDefaultText);
					if ( !config.originCommentID.empty() ) {
						for ( size_t i = 0; i < origin->commentCount(); ++i ) {
							if ( origin->comment(i)->id() == config.originCommentID ) {
								setText(config.customColumn, origin->comment(i)->text().c_str());
								setToolTip(config.customColumn, origin->comment(i)->text().c_str());
								auto it = config.customColorMap.find(origin->comment(i)->text());
								if ( it != config.customColorMap.end() ) {
									setData(config.customColumn, Qt::ForegroundRole, it.value());
								}
								break;
							}
						}
					}
					else if ( !config.eventCommentID.empty() ) {
						for ( size_t i = 0; i < ev->commentCount(); ++i ) {
							if ( ev->comment(i)->id() == config.eventCommentID ) {
								if ( ev->comment(i)->text().empty() ) {
									break;
								}

								setText(config.customColumn, ev->comment(i)->text().c_str());
								setToolTip(config.customColumn, ev->comment(i)->text().c_str());
								auto it = config.customColorMap.find(ev->comment(i)->text());
								if ( it != config.customColorMap.end() ) {
									setData(config.customColumn, Qt::ForegroundRole, it.value());
								}
								break;
							}
						}
					}
				}
			}
			else {
				setText(config.columnMap[COL_PHASES], "-");
				setText(config.columnMap[COL_RMS], "-");
				setText(config.columnMap[COL_AZIMUTHAL_GAP], "-");
				setText(config.columnMap[COL_M], "-");
				setText(config.columnMap[COL_MTYPE], "-");
				setText(config.columnMap[COL_DEPTH], "-");
				setText(config.columnMap[COL_DEPTH_TYPE], "-");

				setText(config.columnMap[COL_AUTHOR], "");
				setText(config.columnMap[COL_OTIME], "no preferred origin");

				setText(config.columnMap[COL_LAT], "");
				setData(config.columnMap[COL_LAT], Qt::UserRole, QVariant());

				setText(config.columnMap[COL_LON], "");
				setData(config.columnMap[COL_LON], Qt::UserRole, QVariant());

				setText(config.columnMap[COL_DEPTH], "-");
				setData(config.columnMap[COL_DEPTH], Qt::UserRole, QVariant());

				setText(config.columnMap[COL_DEPTH_TYPE], "-");

				setText(config.columnMap[COL_TYPE], "");
				setForeground(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);

				setText(config.columnMap[COL_PHASES], "-");
				setData(config.columnMap[COL_PHASES], Qt::UserRole, QVariant());

				setText(config.columnMap[COL_RMS], "-");
				setData(config.columnMap[COL_RMS], Qt::UserRole, QVariant());

				setText(config.columnMap[COL_AZIMUTHAL_GAP], "-");
				setData(config.columnMap[COL_AZIMUTHAL_GAP], Qt::UserRole, QVariant());

				if ( config.customColumn != -1 ) {
					setText(config.customColumn, config.customDefaultText);
					setToolTip(config.customColumn, config.customDefaultText);
					setData(config.customColumn, Qt::ForegroundRole, QVariant());
				}
			}

			updateTimeAgo();

			//! --------------------------------------------------------------
			try {
				setEnabled(!config.hiddenEventTypes.contains(ev->type()));
			}
			catch ( ValueException& ) {
				setEnabled(true);
			}

			QString summary;

			for ( int i = 0; i < columnCount(); ++i ) {
				if ( i > 0 ) {
					summary += '\n';
				}

				summary += QString("%1: %2").arg(config.header[i], text(i));
			}

			setToolTip(config.columnMap[COL_ID], summary);
		}

	private:
		static bool originItemLessThan(const QTreeWidgetItem *i1, const QTreeWidgetItem *i2) {
			const auto *oi1 = static_cast<const OriginTreeItem*>(i1);
			const auto *oi2 = static_cast<const OriginTreeItem*>(i2);

			auto *o1 = static_cast<Origin*>(oi1->object());
			auto *o2 = static_cast<Origin*>(oi2->object());

			if ( !o1 || !o2 ) {
				return false;
			}

			try {
				return o1->creationInfo().creationTime() > o2->creationInfo().creationTime();
			}
			catch ( Core::ValueException & ) {}

			return false;
		}

		static bool fmItemLessThan(const QTreeWidgetItem *i1, const QTreeWidgetItem *i2) {
			const auto *fmi1 = static_cast<const FocalMechanismTreeItem*>(i1);
			const auto *fmi2 = static_cast<const FocalMechanismTreeItem*>(i2);

			auto *fm1 = static_cast<FocalMechanism *>(fmi1->object());
			auto *fm2 = static_cast<FocalMechanism *>(fmi2->object());

			if ( !fm1 || !fm2 ) {
				return false;
			}

			try {
				return fm1->creationInfo().creationTime() > fm2->creationInfo().creationTime();
			}
			catch ( Core::ValueException & ) {}

			return false;
		}

		TreeItem *_origins;
		TreeItem *_focalMechanisms;

		std::string _lastPreferredOriginID;
		bool _showOnlyOnePerAgency;
		bool _resort;
		bool _hasMultipleAgencies;
		bool _published;

		static std::string TimeFormat;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string EventTreeItem::TimeFormat;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool sendJournal(const std::string &objectID, const std::string &action,
                 const std::string &params, const char *group) {
	JournalEntryPtr entry = new JournalEntry;
	entry->setObjectID(objectID);
	entry->setAction(action);
	entry->setParameters(params);
	entry->setSender(SCApp->author());
	entry->setCreated(Core::Time::UTC());

	NotifierPtr n = new Notifier("Journaling", OP_ADD, entry.get());
	NotifierMessagePtr nm = new NotifierMessage;
	nm->attach(n.get());
	return SCApp->sendMessage(group, nm.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class TreeWidget : public QTreeWidget {
	public:
		TreeWidget(QWidget *p)
		: QTreeWidget(p) {}

		void startDrag(Qt::DropActions supportedActions) override {
			if ( !currentItem() || currentItem()->type() == ST_None ) {
				SEISCOMP_DEBUG("About to drag an item without type");
				return;
			}

			auto *item = static_cast<SchemeTreeItem*>(currentItem());
			if ( !item->object() ) {
				SEISCOMP_DEBUG("Item has no object attached");
				return;
			}

			QMimeData *mimeData = nullptr;
			switch ( item->type() ) {
				case ST_Event:
					mimeData = new QMimeData;
					mimeData->setData("uri/event", item->object()->publicID().c_str());
					break;
				case ST_Origin:
					mimeData = new QMimeData;
					mimeData->setData("uri/origin", item->object()->publicID().c_str());
					break;
				default:
					SEISCOMP_DEBUG("Unknown item type");
					break;
			}

			if ( !mimeData ) {
				return;
			}

			for ( int i = 0; i < item->columnCount(); ++i ) {
				item->setBackground(i, palette().color(QPalette::Highlight));
				item->setForeground(i, palette().color(QPalette::HighlightedText));
			}

			//SEISCOMP_DEBUG("Start drag");
			auto *drag = new QDrag(this);
			drag->setMimeData(mimeData);
			drag->exec(Qt::MoveAction);

			for ( int i = 0; i < item->columnCount(); ++i ) {
				item->setBackground(i, Qt::NoBrush);
				item->setForeground(i, Qt::NoBrush);
			}
		}

		void dragEnterEvent(QDragEnterEvent *event) override {
			if ( event->source() != this ) {
				event->ignore();
				return;
			}

			dragMoveEvent(event);
			event->accept();
		}

		void dragMoveEvent(QDragMoveEvent *event) override {
			QTreeWidget::dragMoveEvent(event);

			auto *item = static_cast<SchemeTreeItem*>(itemAt(QT_EVENT_POS(event)));

			/*
			if ( _lastDropItem && item != _lastDropItem ) {
				for ( int i = 0; i < _lastDropItem->columnCount(); ++i )
					_lastDropItem->setBackground(i, Qt::NoBrush);
			}
			*/

			setCurrentItem(nullptr);

			if ( !item || item->type() != ST_Event || !item->object()  ) {
				//SEISCOMP_DEBUG("Drop item is not an event");
				event->ignore();
				return;
			}

			_lastDropItem = item;
			setCurrentItem(_lastDropItem);
			/*
			for ( int i = 0; i < item->columnCount(); ++i )
				item->setBackground(i, Qt::green);
			*/
			event->accept();
		}

		void dragLeaveEvent(QDragLeaveEvent *event) override {
			setCurrentItem(nullptr);
			/*
			if ( _lastDropItem ) {
				for ( int i = 0; i < _lastDropItem->columnCount(); ++i )
					_lastDropItem->setBackground(i, Qt::NoBrush);
			}
			*/

			_lastDropItem = nullptr;
		}

		void dropEvent(QDropEvent *event) override {
			_lastDropItem = nullptr;
		}


	private:
		QTreeWidgetItem *_lastDropItem{nullptr};
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class CommandWaitDialog : public QDialog {
	public:
		CommandWaitDialog(QWidget *parent)
		: QDialog(parent) {
			setWindowTitle("Status");
			setWindowModality(Qt::ApplicationModal);

			auto *l = new QVBoxLayout;
			setLayout(l);

			auto *topvl = new QVBoxLayout;

			auto *hl = new QHBoxLayout;
			_labelCommand = new QLabel;
			QFont f = _labelCommand->font();
			f.setBold(true);
			_labelCommand->setFont(f);

			_labelStatus = new QLabel;
			hl->addWidget(_labelCommand);
			hl->addWidget(_labelStatus);
			hl->addStretch();

			_labelMessage = new QLabel;
			f = _labelMessage->font();
			f.setItalic(true);
			_labelMessage->setFont(f);

			_progressBar = new QProgressBar;
			_progressBar->setRange(0,0);

			topvl->addLayout(hl);
			topvl->addWidget(_labelMessage);

			hl = new QHBoxLayout;
			hl->addLayout(topvl);
			hl->addStretch();

			_labelIcon = new QLabel;
			topvl = new QVBoxLayout;
			topvl->addWidget(_labelIcon);
			topvl->addStretch();

			hl->addLayout(topvl);
			l->addLayout(hl);

			l->addWidget(_progressBar);

			hl = new QHBoxLayout;
			hl->addStretch();

			l->addStretch();

			auto *closeButton = new QPushButton("Close");
			connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
			hl->addWidget(closeButton);

			l->addLayout(hl);
		}

		void setCommand(const std::string &obj, const std::string &cmd) {
			_objectID = obj;
			_command = cmd;

			if ( _command == CMD_SPLIT_ORIGIN ) {
				_labelCommand->setText("Split origin");
			}
			else if ( _command == CMD_NEW_EVENT ) {
				_labelCommand->setText("Form new event");
			}
			else if ( _command == CMD_MERGE_EVENT ) {
				_labelCommand->setText("Merge events");
			}
			else if ( _command == CMD_GRAB_ORIGIN ) {
				_labelCommand->setText("Move origin");
			}
			else {
				_labelCommand->setText(_command.c_str());
			}

			_labelStatus->setPalette(QPalette());

			auto icon = style()->standardIcon(QStyle::SP_MessageBoxInformation);
			_labelIcon->setPixmap(icon.pixmap(32,32));

			_labelStatus->setText("(waiting)");
			_labelMessage->setText("Waiting for command to finish...");

			_progressBar->setRange(0,0);
			_progressBar->setValue(0);
		}

		bool handle(JournalEntry *entry) {
			if ( entry->objectID() != _objectID ) {
				return false;
			}

			if ( entry->action().compare(0, _command.size(), _command) != 0 ) {
				return false;
			}

			if ( entry->action().compare(_command.size(), 2, "OK") == 0 ) {
				close();
				return true;
			}

			std::string status = entry->action().substr(_command.size());

			_progressBar->setRange(0,100);
			_progressBar->setValue(100);
			_labelMessage->setText(entry->parameters().c_str());
			_labelStatus->setText(QString("(%1)").arg(status.c_str()));

			if ( status == "Failed" ) {
				QPalette pal = _labelStatus->palette();
				pal.setColor(QPalette::WindowText, Qt::red);
				_labelStatus->setPalette(pal);
				QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxCritical);
				_labelIcon->setPixmap(icon.pixmap(32,32));
			}

			return true;
		}

	private:
		QLabel       *_labelIcon;
		QLabel       *_labelCommand;
		QLabel       *_labelStatus;
		QLabel       *_labelMessage;
		QProgressBar *_progressBar;
		std::string   _objectID;
		std::string   _command;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct ConfigProcessColumn {
	int     pos;
	QString label;
	QString originScript;
	QString eventScript;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class EventFilterWidget : public QWidget {
	public:
		EventFilterWidget(QWidget *parent = nullptr)
		: QWidget(parent) {
			_ui.setupUi(this);
		}

		void setView(EventListView *view) {
			if ( _view ) {
				_view->disconnect(_ui.btnReset, SIGNAL(clicked()));
			}

			_view = view;

			if ( _view ) {
				connect(_ui.btnReset, SIGNAL(clicked()), _view, SLOT(clearDatabaseFilter()));
			}
		}

		void setFilter(const EventListView::Filter &filter) {
			_ui.fromLatitude->setValue(filter.minLatitude ? *filter.minLatitude : _ui.fromLatitude->minimum());
			_ui.toLatitude->setValue(filter.maxLatitude ? *filter.maxLatitude : _ui.toLatitude->minimum());
			_ui.fromLongitude->setValue(filter.minLongitude ? *filter.minLongitude : _ui.fromLongitude->minimum());
			_ui.toLongitude->setValue(filter.maxLongitude ? *filter.maxLongitude : _ui.toLongitude->minimum());
			_ui.fromDepth->setValue(filter.minDepth ? *filter.minDepth : _ui.fromDepth->minimum());
			_ui.toDepth->setValue(filter.maxDepth ? *filter.maxDepth : _ui.toDepth->minimum());
			_ui.fromPhase->setValue(filter.minPhaseCount ? *filter.minPhaseCount : _ui.fromPhase->minimum());
			_ui.toPhase->setValue(filter.maxPhaseCount ? *filter.maxPhaseCount : _ui.toPhase->minimum());
			_ui.fromMagnitude->setValue(filter.minMagnitude ? *filter.minMagnitude : _ui.fromMagnitude->minimum());
			_ui.toMagnitude->setValue(filter.maxMagnitude ? *filter.maxMagnitude : _ui.toMagnitude->minimum());
			_ui.editEventID->setText(filter.eventID.c_str());
		}

		/**
		 * Returns a filter structure according to the current settings.
		 */
		[[nodiscard]]
		EventListView::Filter filter() const {
			EventListView::Filter f;

			if ( _ui.fromLatitude->isValid() ) {
				f.minLatitude = _ui.fromLatitude->value();
			}
			if ( _ui.toLatitude->isValid() ) {
				f.maxLatitude = _ui.toLatitude->value();
			}

			if ( _ui.fromLongitude->isValid() ) {
				f.minLongitude = _ui.fromLongitude->value();
			}
			if ( _ui.toLongitude->isValid() ) {
				f.maxLongitude = _ui.toLongitude->value();
			}

			if ( _ui.fromDepth->isValid() ) {
				f.minDepth = _ui.fromDepth->value();
			}
			if ( _ui.toDepth->isValid() ) {
				f.maxDepth = _ui.toDepth->value();
			}

			if ( _ui.fromMagnitude->isValid() ) {
				f.minMagnitude = _ui.fromMagnitude->value();
			}
			if ( _ui.toMagnitude->isValid() ) {
				f.maxMagnitude = _ui.toMagnitude->value();
			}

			if ( _ui.fromPhase->isValid() ) {
				f.minPhaseCount = _ui.fromPhase->value();
			}
			if ( _ui.toPhase->isValid() ) {
				f.maxPhaseCount = _ui.toPhase->value();
			}

			f.eventID = _ui.editEventID->text().toStdString();

			return f;
		}

	private:
		EventListView   *_view{nullptr};
		Ui::EventFilter  _ui;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Helper class to allow proper positioning of the tool buttons menu
class CustomWidgetMenu : public QMenu {
	public:
		CustomWidgetMenu(QWidget *parent = nullptr) : QMenu(parent) {}

		[[nodiscard]]
		QSize sizeHint() const override { return QWidget::sizeHint(); }
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}


using namespace Private;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventListView::Filter::isNull() const {
	return !minLatitude && !maxLatitude &&
	       !minLongitude && !maxLongitude &&
	       !minDepth && !maxDepth &&
	       !minMagnitude && !maxMagnitude &&
	       !minPhaseCount && !maxPhaseCount &&
	       eventID.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListViewPrivate::EventListViewPrivate()
: _ui(new Ui::EventListView) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListViewPrivate::~EventListViewPrivate() {
	delete _ui;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListView::EventListView(Seiscomp::DataModel::DatabaseQuery* reader, bool withOrigins,
                             bool withFocalMechanisms, QWidget * parent, Qt::WindowFlags f)
: QWidget(parent, f)
, _d_ptr(new EventListViewPrivate) {
	SC_D._reader = reader;
	SC_D._withOrigins = withOrigins;
	SC_D._withFocalMechanisms = withFocalMechanisms;
	SC_D._blockSelection = false;
	SC_D._blockRemovingOfExpiredEvents = false;
	SC_D._blockCountSignal = false;
	SC_D._ui->setupUi(this);

	SC_D._visibleEventCount = 0;
	SC_D._regionIndex = 0;
	SC_D._commandWaitDialog = nullptr;

	QBoxLayout *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	SC_D._ui->frameList->setLayout(l);

	SC_D._treeWidget = new TreeWidget(SC_D._ui->frameList);
	SC_D._treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	SC_D._treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	SC_D._treeWidget->setMouseTracking(true);
	SC_D._treeWidget->viewport()->installEventFilter(this);
	SC_D._treeWidget->setAutoScroll(true);

	l->addWidget(SC_D._treeWidget);

	setSortingEnabled(true);

	SC_D._ui->btnFilter->setIcon(icon("filter"));

	SC_D._unassociatedEventItem = nullptr;
	SC_D._updateLocalEPInstance = false;

	SC_D._itemConfig.disabledColor = palette().color(QPalette::Disabled, QPalette::Text);
	SC_D._itemConfig.columnMap.clear();

	try {
		std::vector<std::string> cols = SCApp->configGetStrings("eventlist.visibleColumns");

		// initialize all columns with index 0
		SC_D._itemConfig.columnMap = QVector<int>(EventListColumns::Quantity, 0);

		// create ordered list of visible columns ids
		std::vector<int> configuredCols;
		for ( auto &col : cols ) {
			EventListColumns v;
			if ( !v.fromString(col) ) {
				if ( col != "TP" ) {
					std::cerr << "ERROR: eventlist.visibleColumns: invalid column name '"
					          << col << ", ignoring" << std::endl;
					continue;
				}

				v = COL_MTYPE;
				std::cerr << "WARNING: eventlist.visibleColumns: name 'TP' "
				             "has changed to 'MType', please update your configuration" << std::endl;
			}
			configuredCols.push_back(v);
		}

		// register columns keeping default order of invisible columns while
		// respecting configured order of visible columns
		// otime is first culumn and always visible
		int i = COL_OTIME;
		colVisibility[i] = true;
		SC_D._itemConfig.columnMap[i] = i;

		auto configuredColsIt = configuredCols.begin();
		for ( ++i; i < EventListColumns::Quantity; ++i ) {
			if ( std::find(configuredCols.begin(), configuredCols.end(), i) != configuredCols.end() ) {
				colVisibility[i] = true;
				SC_D._itemConfig.columnMap[*configuredColsIt++] = i;
				continue;
			}

			SC_D._itemConfig.columnMap[i] = i;
			colVisibility[i] = false;
		}
	}
	catch ( ... ) {
		for ( int i = 0; i < EventListColumns::Quantity; ++i ) {
			SC_D._itemConfig.columnMap.append(i);
		}
	}

	SC_D._itemConfig.header.reserve(EventListColumns::Quantity);
	for ( int i = 0; i < EventListColumns::Quantity; ++i ) {
		SC_D._itemConfig.header.append("");
	}

	for ( int i = 0; i < SC_D._itemConfig.columnMap.size(); ++i ) {
		auto &header = SC_D._itemConfig.header[SC_D._itemConfig.columnMap[i]];
		switch ( i ) {
			case COL_OTIME:
				if ( SCScheme.dateTime.useLocalTime ) {
					header = QString(EEventListColumnsNames::name(i)).arg(Core::Time::LocalTimeZone().c_str());
				}
				else {
					header = QString(EEventListColumnsNames::name(i)).arg("UTC");
				}
				break;
			case COL_AZIMUTHAL_GAP:
				header = QString(EEventListColumnsNames::name(i)) + " (°)";
				break;
			case COL_LAT:
				header = QString(EEventListColumnsNames::name(i)) + " (°)";
				break;
			case COL_LON:
				header = QString(EEventListColumnsNames::name(i)) + " (°)";
				break;
			case COL_RMS:
				header = QString(EEventListColumnsNames::name(i)) + " (s)";
				break;
			default:
				header =  EEventListColumnsNames::name(i);
				break;
		}
	}

	SC_D._itemConfig.customColumn = -1;
	SC_D._itemConfig.customDefaultText = "-";

	if ( !withOrigins ) {
		colVisibility[COL_ORIGINS] = false;
	}

	try {
		auto types = SCApp->configGetStrings("eventlist.filter.types.blacklist");
		for ( const auto &sType : types ) {
			EventType type;
			if ( type.fromString(sType) ) {
				SC_D._itemConfig.hiddenEventTypes.insert(type);
			}
			else {
				std::cerr << "WARNING: eventlist.filter.types.blacklist: unknown type: "
				          << sType << std::endl;
			}
		}
	}
	catch ( ... ) {
		SC_D._itemConfig.hiddenEventTypes.insert(NOT_EXISTING);
		SC_D._itemConfig.hiddenEventTypes.insert(OTHER_EVENT);
	}

	try {
		std::vector<std::string> prefAgencies = SCApp->configGetStrings("eventlist.filter.agencies.whitelist");
		for ( const auto &agency : prefAgencies ) {
			SC_D._itemConfig.preferredAgencies.insert(agency.c_str());
		}
	}
	catch ( ... ) {
		SC_D._itemConfig.preferredAgencies.insert(SCApp->agencyID().c_str());
	}

	try {
		SC_D._checkEventAgency = SCApp->configGetString("eventlist.filter.agencies.type") == "events";
	}
	catch ( ... ) {
		SC_D._checkEventAgency = true;
	}

	try { SC_D._ui->cbHideForeign->setText(SCApp->configGetString("eventlist.filter.agencies.label").c_str()); }
	catch ( ... ) {}

	try { SC_D._ui->cbHideForeign->setChecked(SCApp->configGetBool("eventlist.filter.agencies.enabled")); }
	catch ( ... ) {}

	try { SC_D._ui->cbHideOther->setText(SCApp->configGetString("eventlist.filter.types.label").c_str()); }
	catch ( ... ) {}

	try { SC_D._ui->cbHideOther->setChecked(SCApp->configGetBool("eventlist.filter.types.enabled")); }
	catch ( ... ) {}

	try { SC_D._ui->cbFilterRegions->setChecked(SCApp->configGetBool("eventlist.filter.regions.enabled")); }
	catch ( ... ) {}

	try {
		SC_D._itemConfig.customColumn = SCApp->configGetInt("eventlist.customColumn.pos");
	}
	catch ( ... ) {}

	try {
		SC_D._itemConfig.customDefaultText = SCApp->configGetString("eventlist.customColumn.default").c_str();
	}
	catch ( ... ) {}

	try {
		std::vector<std::string> customColors = SCApp->configGetStrings("eventlist.customColumn.colors");
		for ( const auto &customColor : customColors ) {
			size_t pos = customColor.rfind(':');
			if ( pos == std::string::npos ) {
				continue;
			}

			std::string value = customColor.substr(0, pos);
			std::string strColor = customColor.substr(pos+1);
			QColor color;
			if ( fromString(color, strColor) ) {
				SC_D._itemConfig.customColorMap[value] = color;
			}
		}
	}
	catch ( ... ) {}

	std::string customColumn;

	try {
		customColumn = SCApp->configGetString("eventlist.customColumn.name");
	}
	catch ( ... ) {
		try {
			customColumn = SCApp->configGetString("eventlist.customColumn");
			SEISCOMP_WARNING("The parameter 'eventlist.customColumn' is deprecated and will be removed in future. "
			                 "Please replace with 'eventlist.customColumn.name'.");
		}
		catch ( ... ) {}
	}

	if ( !customColumn.empty() ) {
		if ( SC_D._itemConfig.customColumn >= 0 &&
		     SC_D._itemConfig.customColumn < SC_D._itemConfig.header.size() ) {
			SC_D._itemConfig.header.insert(SC_D._itemConfig.customColumn, customColumn.c_str());
		}
		else {
			SC_D._itemConfig.header.append(customColumn.c_str());
			SC_D._itemConfig.customColumn = SC_D._itemConfig.header.size()-1;
		}

		if ( SC_D._itemConfig.customColumn >= 0 &&
		     SC_D._itemConfig.customColumn < SC_D._itemConfig.columnMap.size() ) {
			for ( int i = 0; i < SC_D._itemConfig.columnMap.size(); ++i ) {
				auto &idx = SC_D._itemConfig.columnMap[i];
				if ( idx >= SC_D._itemConfig.customColumn ) {
					++idx;
				}
			}
		}
	}

	try {
		SC_D._itemConfig.originCommentID = SCApp->configGetString("eventlist.customColumn.originCommentID");
	}
	catch ( ... ) {}
	try {
		SC_D._itemConfig.eventCommentID = SCApp->configGetString("eventlist.customColumn.eventCommentID");
	}
	catch ( ... ) {}

	// Read script column configuration. A column is shared between origins and
	// events if the label and position are equal.
	QVector<ConfigProcessColumn> scriptColumns;
	std::vector<std::string> processProfiles;
	try {
		processProfiles = SCApp->configGetStrings("eventlist.scripts.columns");
	}
	catch ( ... ) {}

	for ( const auto &profile : processProfiles ) {
		ConfigProcessColumn item;
		std::string prefix = "eventlist.scripts.column." + profile;
		try {
			item.pos = SCApp->configGetInt(prefix + ".pos");
		}
		catch ( ... ) {
			item.pos = -1;
		}

		QString script;
		try {
			script = Environment::Instance()->absolutePath(
			             SCApp->configGetString(prefix + ".script")).c_str();
		}
		catch ( ... ) {}

		if ( script.isEmpty() ) {
			std::cerr << "WARNING: " << prefix
			          << ".script is not set: ignoring" << std::endl;
			continue;
		}

		try {
			item.label = SCApp->configGetString(prefix + ".label").c_str();
		}
		catch ( ... ) {
			std::cerr << "WARNING: " << prefix
			          << ".label is not set: ignoring" << std::endl;
			continue;
		}


		try {
			auto types = SCApp->configGetStrings(prefix + ".types");
			for ( const auto &type : types ) {
				if ( !compareNoCase(type, "Origin") ) {
					item.originScript = script;
				}
				else if ( !compareNoCase(type, "Event") ) {
					item.eventScript = script;
				}
				else {
					std::cerr << "WARNING: " << prefix
					          << ".types: type '" << type
					          << "' unsupported" << std::endl;
				}
			}
		}
		catch ( ... ) {
			item.originScript = script;
		}
		if ( item.originScript.isEmpty() && item.eventScript.isEmpty() ) {
			std::cerr << "WARNING: " << prefix
			          << ".types: no valid type found, ignoring" << std::endl;
			continue;
		}

		// event run: check for columns with same position and label
		bool matchFound = false;
		for ( auto &other : scriptColumns ) {
			if ( other.pos == item.pos && other.label == item.label ) {
				if ( !item.originScript.isEmpty() ) {
					other.originScript = item.originScript;
				}
				if ( !item.eventScript.isEmpty() ) {
					other.eventScript = item.eventScript;
				}
				matchFound = true;
				break;
			}
		}
		if ( !matchFound ) {
			scriptColumns.append(item);
		}
	}

	// Apply process column configuration
	for ( auto &item : scriptColumns ) {
		if ( item.pos >= 0 && item.pos < SC_D._itemConfig.header.size() ) {
			SC_D._itemConfig.header.insert(item.pos, item.label);
			if ( item.pos <= SC_D._itemConfig.customColumn ) {
				++SC_D._itemConfig.customColumn;
			}
		}
		else {
			SC_D._itemConfig.header.append(item.label);
			item.pos = SC_D._itemConfig.header.size()-1;
		}

		if ( item.pos >= 0 && item.pos < SC_D._itemConfig.columnMap.size() ) {
			// Remap predefined columns
			for ( int i = 0; i < SC_D._itemConfig.columnMap.size(); ++i ) {
				if ( SC_D._itemConfig.columnMap[i] >= item.pos ) {
					++SC_D._itemConfig.columnMap[i];
				}
			}

			// Remap origin and event process columns
			for ( int i = 0; i < SC_D._itemConfig.originScriptColumns.size(); ++i ) {
				auto &col = SC_D._itemConfig.originScriptColumns[i];
				if ( col.pos >= item.pos ) {
					++col.pos;
					++SC_D._itemConfig.originScriptColumnMap[col.script];
				}
			}
			for ( int i = 0; i < SC_D._itemConfig.eventScriptColumns.size(); ++i ) {
				auto &col = SC_D._itemConfig.eventScriptColumns[i];
				if ( col.pos >= item.pos ) {
					++col.pos;
					++SC_D._itemConfig.eventScriptColumnMap[col.script];
				}
			}
		}

		if ( !item.originScript.isEmpty() ) {
			EventListViewPrivate::ProcessColumn col;
			col.pos = item.pos;
			col.script = item.originScript;
			SC_D._itemConfig.originScriptColumns.append(col);
			SC_D._itemConfig.originScriptColumnMap[col.script] = col.pos;
		}
		if ( !item.eventScript.isEmpty() ) {
			EventListViewPrivate::ProcessColumn col;
			col.pos = item.pos;
			col.script = item.eventScript;
			SC_D._itemConfig.eventScriptColumns.append(col);
			SC_D._itemConfig.eventScriptColumnMap[col.script] = col.pos;
		}
	}

	// Create set of event script column positions for faster lookup
	for ( int i = 0; i < SC_D._itemConfig.eventScriptColumns.size(); ++i ) {
		SC_D._itemConfig.eventScriptPositions << SC_D._itemConfig.eventScriptColumns[i].pos;
	}

	if ( !SC_D._withOrigins && !SC_D._withFocalMechanisms ) {
		SC_D._treeWidget->setRootIsDecorated(false);
	}

	Region reg;
	reg.name = "- custom -";
	reg.bbox = Geo::GeoBoundingBox(-90, -180, 90, 180);
	SC_D._filterRegions.append(reg);

	// Read region definitions for filters
	{
		std::vector<std::string> regionProfiles;

		try {
			regionProfiles = SCApp->configGetStrings("eventlist.filter.regions.profiles");
		}
		catch ( ... ) {
			try {
				regionProfiles = SCApp->configGetStrings("eventlist.regions");
				SEISCOMP_WARNING("The parameter 'eventlist.regions' is deprecated and will be removed in future. "
				                 "Please replace with 'eventlist.filter.regions.profiles'.");
			}
			catch ( ... ) {}
		}

		for ( const auto &regionProfile : regionProfiles ) {
			std::string name;
			std::vector<double> defs;

			reg = Region();

			try {
				name = SCApp->configGetString("eventlist.filter.regions.region." + regionProfile + ".name");
			}
			catch ( ... ) {
				try {
					name = SCApp->configGetString("eventlist.region." + regionProfile + ".name");
					SEISCOMP_WARNING("The parameter 'eventlist.region.%s.name' is deprecated and will be removed in future. "
					                 "Please replace with 'eventlist.filter.regions.region.%s.name'.",
					                 regionProfile.c_str(), regionProfile.c_str());
				}
				catch ( ... ) {
					std::cerr << "WARNING: eventlist.filter.regions.region."
					          << regionProfile << ".name is not set: ignoring"
					          << std::endl;
					continue;
				}
			}

			try {
				auto poly = SCApp->configGetString("eventlist.filter.regions.region." + regionProfile + ".poly");
				if ( !poly.empty() ) {
					for ( size_t i = 0; i < Seiscomp::Regions::polyRegions().regionCount(); ++i ) {
						auto *feature = Seiscomp::Regions::polyRegions().region(i);
						if ( feature->name() == poly ) {
							reg.poly = feature;
							break;
						}
					}

					if ( !reg.poly ) {
						for ( auto *feature : Geo::GeoFeatureSetSingleton::getInstance().features() ) {
							if ( feature->name() == poly ) {
								reg.poly = feature;
								break;
							}
						}
					}

					if ( !reg.poly ) {
						std::cerr << "WARNING: eventlist.filter.regions.region."
						          << regionProfile << ".poly: polygon '"
						          << poly
						          << "' has not been found "
						             "(neither in fep nor in spatial "
						             "vector layer)"
						          << std::endl;
						continue;
					}

					if ( !reg.poly->closedPolygon() ) {
						std::cerr << "WARNING: eventlist.filter.regions.region."
						          << regionProfile << ".poly: feature '"
						          << poly
						          << "' is not a polygon (not closed) which "
						             "will not work for polygon based filtering"
						          << std::endl;
						continue;
					}
				}
			}
			catch ( ... ) {}

			if ( !reg.poly ) {
				try {
					defs = SCApp->configGetDoubles("eventlist.filter.regions.region." + regionProfile + ".rect");
				}
				catch ( ... ) {
					try {
						defs = SCApp->configGetDoubles("eventlist.region." + regionProfile + ".rect");
						SEISCOMP_WARNING("The parameter 'eventlist.region.%s.rect' is deprecated and will be removed in future. "
						                 "Please replace with 'eventlist.filter.regions.region.%s.rect'.",
						                 regionProfile.c_str(), regionProfile.c_str());
					}
					catch ( ... ) {
						std::cerr << "WARNING: eventlist.filter.regions.region."
						          << regionProfile << ".rect requires exactly 4 parameters (nothing given): ignoring"
						          << std::endl;
						continue;
					}
				}

				if ( defs.size() != 4 ) {
					std::cerr << "WARNING: eventlist.filter.regions.region."
					          << regionProfile << ".rect requires exactly 4 parameters ("
					          << defs.size() << " given): ignoring"
					          << std::endl;
					continue;
				}

				reg.bbox = Geo::GeoBoundingBox(defs[0], defs[1], defs[2], defs[3]);
			}

			if ( name.empty() ) {
				std::cerr << "WARNING: eventlist.filter.regions.region."
				          << regionProfile << ".name is empty: ignoring"
				          << std::endl;
				continue;
			}

			reg.name = name.c_str();
			SC_D._filterRegions.append(reg);
		}
	}

	// Initialize database filter
	try { SC_D._filter.minLatitude = SCApp->configGetDouble("eventlist.filter.database.minlat"); }
	catch ( ... ) {}
	try { SC_D._filter.maxLatitude = SCApp->configGetDouble("eventlist.filter.database.maxlat"); }
	catch ( ... ) {}
	try { SC_D._filter.minLongitude = SCApp->configGetDouble("eventlist.filter.database.minlon"); }
	catch ( ... ) {}
	try { SC_D._filter.maxLongitude = SCApp->configGetDouble("eventlist.filter.database.maxlon"); }
	catch ( ... ) {}
	try { SC_D._filter.minDepth = SCApp->configGetDouble("eventlist.filter.database.mindepth"); }
	catch ( ... ) {}
	try { SC_D._filter.maxDepth = SCApp->configGetDouble("eventlist.filter.database.maxdepth"); }
	catch ( ... ) {}
	try { SC_D._filter.minMagnitude = SCApp->configGetDouble("eventlist.filter.database.minmag"); }
	catch ( ... ) {}
	try { SC_D._filter.maxMagnitude = SCApp->configGetDouble("eventlist.filter.database.maxmag"); }
	catch ( ... ) {}
	try { SC_D._filter.minPhaseCount = SCApp->configGetInt("eventlist.filter.database.minphasecount");}
	catch ( ... ) {}
	try { SC_D._filter.maxPhaseCount = SCApp->configGetInt("eventlist.filter.database.maxphasecount"); }
	catch ( ... ) {}

	for ( int i = 0; i < SC_D._filterRegions.size(); ++i ) {
		SC_D._ui->lstFilterRegions->addItem(SC_D._filterRegions[i].name);
	}

	if ( SC_D._ui->lstFilterRegions->count() > 1 ) {
		SC_D._regionIndex = 1;
		SC_D._ui->lstFilterRegions->setCurrentIndex(SC_D._regionIndex);
		SC_D._ui->btnChangeRegion->hide();
	}

	SC_D._ui->btnFilter->setPopupMode(QToolButton::InstantPopup);

	SC_D._filterWidget = new EventFilterWidget;
	SC_D._filterWidget->setView(this);
	SC_D._filterWidget->setFilter(SC_D._filter);

	auto *vl = new QVBoxLayout;
	vl->addWidget(SC_D._filterWidget);

	// TimeAgo column properties
	double interval = 1.0;
	try {
		interval = std::max(1.0, SCApp->configGetDouble("eventlist.timeAgo.interval"));
	}
	catch ( Config::OptionNotFoundException& ) {}
	catch ( Config::TypeConversionException& ) {}

	SC_D._otimeAgoTimer.setInterval(interval * 1000.);

	try {
		SC_D._itemConfig.otimeAgo.gradient = SCApp->configGetColorGradient(
		        "eventlist.timeAgo.background.gradient",
		        SC_D._itemConfig.otimeAgo.gradient);
		SC_D._itemConfig.otimeAgo.discrete = SCApp->configGetBool(
		        "eventlist.timeAgo.background.discrete");
	}
	catch ( Config::OptionNotFoundException& ) {}
	catch ( Config::TypeConversionException& ) {}

	try {
		SC_D._exportScript = SCApp->configGetPath("eventlist.scripts.export").c_str();
	}
	catch ( Config::OptionNotFoundException& ) {}
	catch ( Config::TypeConversionException& ) {}

	auto *menu = new CustomWidgetMenu(SC_D._ui->btnFilter);
	menu->setLayout(vl);
	SC_D._ui->btnFilter->setMenu(menu);

	connect(SC_D._ui->lstFilterRegions, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(regionSelectionChanged(int)));

	connect(SC_D._ui->btnChangeRegion, SIGNAL(clicked()), this, SLOT(changeRegion()));

	//_treeWidget->setHeaderLabels(QStringList() << "PublicID" << "Desc/Time" << "Mag" << "StaCount" << "defPhaseCount");
	SC_D._treeWidget->setHeaderLabels(SC_D._itemConfig.header);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_OTIME],
		tr("Origin time")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_TIME_AGO],
		tr("Difference between current time and origin time")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_EVENTTYPE_CERTAINTY],
		tr("Certainty of event")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_EVENTTYPE],
		tr("Type of event")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_M],
		tr("Preferred magnitude of event")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_MTYPE],
		tr("Type of preferred magnitude of event")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_RMS],
		tr("RMS of origin as returned by locator")
	);

	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_AZIMUTHAL_GAP],
		tr("Largest azimuth between any 2 neighboring stations providing picks to an origins")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_LAT],
		tr("Latitude of origin")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_LON],
		tr("Longitude of origin")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_DEPTH],
		tr("Depth of origin")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_DEPTH_TYPE],
		tr("Depth type of origin")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_FM],
		tr("Has event any referenced focal mechanism?")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_TYPE],
		tr("Origin evaluation status\n"
		   "A: not set\n"
		   "F: final\n"
		   "V: reviewed\n"
		   "C: confirmed\n"
		   "P: preliminary\n"
		   "R: reported\n"
		   "X: rejected\n"
		   "Origin evaluation mode\n"
		   "red: automatic\n"
		   "green: manual\n"
		   "Trailing\n"
		   "+: event has origins from multiple agencies")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_PHASES],
		tr("Number of arrivals = referenced phase picks")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_ORIGINS],
		tr("Number of origins referenced by event")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_AGENCY],
		tr("ID of agency providing origin")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_AUTHOR],
		tr("ID of author providing origin")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_REGION],
		tr("Region name of origin")
	);
	SC_D._treeWidget->headerItem()->setToolTip(
		SC_D._itemConfig.columnMap[COL_ID],
		tr("Event ID")
	);
	SC_D._treeWidget->setAlternatingRowColors(true);

	for ( int i = 0; i < EventListColumns::Quantity; ++i ) {
		SC_D._treeWidget->header()->setSectionHidden(SC_D._itemConfig.columnMap[i], !colVisibility[i]);
	}

	SC_D._treeWidget->header()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(SC_D._treeWidget->header(), SIGNAL(customContextMenuRequested(QPoint)),
	        this, SLOT(headerContextMenuRequested(QPoint)));

	connect(SC_D._treeWidget->header(), SIGNAL(sectionClicked(int)),
	        this, SLOT(sortItems(int)));

	addAction(SC_D._ui->actionCopyRowToClipboard);

	auto *expandAll = new QAction(this);
	expandAll->setShortcut(Qt::CTRL | Qt::Key_E);

	auto *collapseAll = new QAction(this);
	collapseAll->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_E);

	addAction(expandAll);
	addAction(collapseAll);

	SC_D._ui->btnReadDays->setEnabled(SC_D._reader != nullptr);
	SC_D._ui->btnReadInterval->setEnabled(SC_D._reader != nullptr);

	QT_DTE_SET(SC_D._ui->dateTimeEditStart, QDateTime::currentDateTimeUtc(), SCScheme.dateTime.useLocalTime);
	QT_DTE_SET(SC_D._ui->dateTimeEditEnd, QDateTime::currentDateTimeUtc(), SCScheme.dateTime.useLocalTime);

	if ( SCScheme.dateTime.useLocalTime ) {
		SC_D._ui->dateTimeEditStart->setDisplayFormat(SC_D._ui->dateTimeEditStart->displayFormat() + " " + Core::Time::LocalTimeZone().c_str());
		SC_D._ui->dateTimeEditEnd->setDisplayFormat(SC_D._ui->dateTimeEditEnd->displayFormat() + " " + Core::Time::LocalTimeZone().c_str());
	}
	else {
		SC_D._ui->dateTimeEditStart->setDisplayFormat(SC_D._ui->dateTimeEditStart->displayFormat() + " UTC");
		SC_D._ui->dateTimeEditEnd->setDisplayFormat(SC_D._ui->dateTimeEditEnd->displayFormat() + " UTC");
	}

	initTree();

	SC_D._autoSelect = false;

	connect(SC_D._ui->cbHideOther, SIGNAL(stateChanged(int)), this,  SLOT(onShowOtherEvents(int)));
	SC_D._hideOtherEvents = SC_D._ui->cbHideOther->checkState() == Qt::Checked;

	connect(SC_D._ui->cbHideForeign, SIGNAL(stateChanged(int)), this,  SLOT(onShowForeignEvents(int)));
	SC_D._hideForeignEvents = SC_D._ui->cbHideForeign->checkState() == Qt::Checked;

	connect(SC_D._ui->cbFilterRegions, SIGNAL(stateChanged(int)), this,  SLOT(onHideOutsideRegion(int)));
	SC_D._hideOutsideRegion = SC_D._ui->cbFilterRegions->checkState() == Qt::Checked;

	connect(SC_D._ui->cbFilterRegionMode, SIGNAL(currentIndexChanged(int)), this,  SLOT(onFilterRegionModeChanged(int)));

	connect(SC_D._ui->cbHideFinalRejected, SIGNAL(stateChanged(int)), this,  SLOT(onHideFinalRejectedEvents(int)));
	SC_D._hideFinalRejectedEvents = SC_D._ui->cbHideFinalRejected->checkState() == Qt::Checked;

	connect(SC_D._ui->cbHideNew, SIGNAL(stateChanged(int)), this,  SLOT(onHideNewEvents(int)));
	SC_D._hideNewEvents = SC_D._ui->cbHideNew->checkState() == Qt::Checked;

	connect(SC_D._ui->cbShowLatestOnly, SIGNAL(stateChanged(int)), this,  SLOT(updateAgencyState()));
	SC_D._showOnlyLatestPerAgency = SC_D._ui->cbShowLatestOnly->checkState() == Qt::Checked;

	if ( !SC_D._withOrigins ) {
		SC_D._ui->cbShowLatestOnly->setVisible(false);
	}

	connect(SC_D._ui->btnReadDays, SIGNAL(clicked()), this, SLOT(readLastDays()));
	connect(SC_D._ui->btnReadInterval, SIGNAL(clicked()), this, SLOT(readInterval()));
	connect(SC_D._ui->btnClear, SIGNAL(clicked()), this, SLOT(clear()));
	connect(SC_D._treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(itemSelected(QTreeWidgetItem*,int)));
	connect(SC_D._treeWidget, SIGNAL(itemPressed(QTreeWidgetItem*,int)), this, SLOT(itemPressed(QTreeWidgetItem*,int)));
	connect(SC_D._treeWidget, SIGNAL(itemEntered(QTreeWidgetItem*,int)), this, SLOT(itemEntered(QTreeWidgetItem*,int)));
	connect(SC_D._treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemExpanded(QTreeWidgetItem*)));
	connect(SC_D._treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(SC_D._ui->actionCopyRowToClipboard, SIGNAL(triggered(bool)), this, SLOT(copyRowToClipboard()));

	connect(expandAll, SIGNAL(triggered()), SC_D._treeWidget, SLOT(expandAll()));
	connect(collapseAll, SIGNAL(triggered()), SC_D._treeWidget, SLOT(collapseAll()));
	//_withComments = true;

	SC_D._busyIndicator = new QMovie(this);
	SC_D._busyIndicator->setFileName(":/sc/assets/loader.mng");
	SC_D._busyIndicator->setCacheMode(QMovie::CacheAll);

	SC_D._busyIndicatorLabel = new QLabel(SC_D._treeWidget->viewport());
	SC_D._busyIndicatorLabel->hide();
	SC_D._busyIndicatorLabel->setMovie(SC_D._busyIndicator);
	SC_D._busyIndicatorLabel->setToolTip("PublicObject evaluator is running ...");

	connect(SC_D._busyIndicator, SIGNAL(resized(QSize)),
	        this, SLOT(indicatorResized(QSize)));

	connect(&PublicObjectEvaluator::Instance(), SIGNAL(resultAvailable(QString,QString,QString,QString)),
	        this, SLOT(evalResultAvailable(QString,QString,QString,QString)));
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(resultError(QString,QString,QString,int)),
	        this, SLOT(evalResultError(QString,QString,QString,int)));

	// Start movie when the thread starts
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(started()),
	        SC_D._busyIndicatorLabel, SLOT(show()));
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(started()),
	        SC_D._busyIndicator, SLOT(start()));

	// Stop movie and hide label when the thread finishes
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(finished()),
	        SC_D._busyIndicatorLabel, SLOT(hide()));
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(finished()),
	        SC_D._busyIndicator, SLOT(stop()));

	setFMLinkEnabled(SC_D._itemConfig.createFMLink);

	PublicObjectEvaluator::Instance().setDatabaseURI(SCApp->databaseURI().c_str());

	connect(&SC_D._otimeAgoTimer, SIGNAL(timeout()), this, SLOT(updateOTimeAgo()));
	updateOTimeAgoTimer();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::indicatorResized(const QSize &size) {
	SC_D._busyIndicatorLabel->resize(size);
	SC_D._busyIndicatorLabel->move(
		(SC_D._treeWidget->viewport()->width()-SC_D._busyIndicatorLabel->width())/2,
		(SC_D._treeWidget->viewport()->height()-SC_D._busyIndicatorLabel->height())/2
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::onShowOtherEvents(int checked) {
	SC_D._hideOtherEvents = checked == Qt::Checked;
	updateHideState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::onHideFinalRejectedEvents(int checked) {
	SC_D._hideFinalRejectedEvents = checked == Qt::Checked;
	updateHideState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::onHideNewEvents(int checked) {
	SC_D._hideNewEvents = checked == Qt::Checked;
	updateHideState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::onShowForeignEvents(int checked) {
	SC_D._hideForeignEvents = checked == Qt::Checked;
	updateHideState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::onHideOutsideRegion(int checked) {
	SC_D._hideOutsideRegion = checked == Qt::Checked;
	updateHideState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::onFilterRegionModeChanged(int mode) {
	updateHideState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::updateAgencyState() {
	SC_D._showOnlyLatestPerAgency = SC_D._ui->cbShowLatestOnly->checkState() == Qt::Checked;

	SC_D._treeWidget->setUpdatesEnabled(false);

	QProgressDialog progress(this);
	//progress.setWindowModality(Qt::WindowModal);
	progress.setWindowTitle(tr("Please wait..."));
	progress.setRange(0, SC_D._treeWidget->topLevelItemCount());
	progress.setLabelText(tr("Checking states..."));
	progress.setModal(true);
	progress.setCancelButton(nullptr);

	for ( int i = 0; i < SC_D._treeWidget->topLevelItemCount(); ++i ) {
		auto *item = static_cast<EventTreeItem*>(SC_D._treeWidget->topLevelItem(i));

		progress.setValue(i);
		qApp->processEvents();

		item->setShowOneItemPerAgency(SC_D._showOnlyLatestPerAgency);
	}

	SC_D._treeWidget->setUpdatesEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::updateHideState() {
	bool changed = false;

	SC_D._blockCountSignal = true;

	for ( int i = 0; i < SC_D._treeWidget->topLevelItemCount(); ++i ) {
		auto *item = static_cast<EventTreeItem*>(SC_D._treeWidget->topLevelItem(i));
		if ( updateHideState(item) ) {
			changed = true;
		}
	}

	SC_D._blockCountSignal = false;

	if ( changed ) {
		emit eventsUpdated();
		emit visibleEventCountChanged();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventListView::updateHideState(QTreeWidgetItem *item) {
	auto *eitem = static_cast<EventTreeItem*>(item);
	auto *event = eitem->event();
	if ( !event ) {
		return false;
	}

	bool hide = false;

	if ( SC_D._hideOtherEvents ) {
		try {
			if ( SC_D._itemConfig.hiddenEventTypes.contains(event->type()) ) {
				hide = true;
			}
		}
		catch ( Core::ValueException & ) {}
	}

	if ( !hide && SC_D._hideFinalRejectedEvents ) {
		Origin* preferredOrigin = Origin::Find(event->preferredOriginID());
		if ( preferredOrigin ) {
			char evalStat = objectEvaluationStatusToChar(preferredOrigin);
			if ( evalStat == 'F' || evalStat == 'X') {
				hide = true;
			}
		}
	}

	if ( !hide && SC_D._hideNewEvents ) {
		Origin* preferredOrigin = Origin::Find(event->preferredOriginID());
		if ( preferredOrigin ) {
			if ( preferredOrigin->time().value() > SC_D._filter.endTime ) {
				hide = true;
			}
		}
		else {
			hide = true;
		}
	}


	if ( !hide && SC_D._hideForeignEvents ) {
		if ( SC_D._checkEventAgency ) {
			try {
				if ( !SC_D._itemConfig.preferredAgencies.contains(item->text(SC_D._itemConfig.columnMap[COL_AGENCY])) ) {
					hide = true;
				}
			}
			catch ( Core::ValueException & ) {
				hide = true;
			}
		}
		else {
			bool hasOwnOrigin = false;
			int originItems = eitem->originItemCount();
			for ( int i = 0; i < originItems; ++i ) {
				auto *oitem = static_cast<OriginTreeItem*>(eitem->originItem(i));
				if ( SC_D._itemConfig.preferredAgencies.contains(oitem->text(SC_D._itemConfig.columnMap[COL_AGENCY])) ) {
					hasOwnOrigin = true;
					break;
				}
			}

			if ( !hasOwnOrigin ) {
				hide = true;
			}
		}
	}

	if ( !hide && SC_D._hideOutsideRegion && SC_D._regionIndex >= 0 ) {
		bool invert = SC_D._ui->cbFilterRegionMode->currentIndex() == 1;

		const Region &reg = SC_D._filterRegions[SC_D._regionIndex];
		double lat = item->data(SC_D._itemConfig.columnMap[COL_LAT], Qt::UserRole).toDouble();
		double lon = item->data(SC_D._itemConfig.columnMap[COL_LON], Qt::UserRole).toDouble();

		bool isInRegion = reg.poly ?
			reg.poly->contains(Geo::GeoCoordinate(lat, lon))
			:
			reg.bbox.contains(Geo::GeoCoordinate(lat, lon));

		if ( isInRegion == invert ) {
			hide = true;
		}
	}

	if ( hide != item->isHidden() ) {
		item->setHidden(hide);
		if ( hide ) {
			if ( SC_D._visibleEventCount > 0 ) {
				--SC_D._visibleEventCount;
			}
			emit eventRemovedFromList(event);
			if ( !SC_D._blockCountSignal ) {
				emit visibleEventCountChanged();
			}
		}
		else {
			if ( SC_D._visibleEventCount >= 0 ) {
				++SC_D._visibleEventCount;
			}
			emit eventAddedToList(event, false);
			if ( !SC_D._blockCountSignal ) {
				emit visibleEventCountChanged();
			}
		}

		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListView::~EventListView() {
	PublicObjectEvaluator::Instance().clear(this);
	delete _d_ptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::setRelativeMinimumEventTime(const Seiscomp::Core::TimeSpan& timeAgo) {
	SC_D._timeAgo = timeAgo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::add(Seiscomp::DataModel::Event* event,
                        Seiscomp::DataModel::Origin* origin) {
	if ( !origin && !event ) {
		return;
	}

	SC_D._blockRemovingOfExpiredEvents = true;

	if ( !origin ) {
		SchemeTreeItem *item = findEvent(event->publicID());
		std::map<std::string, OriginPtr> orgs;
		std::map<std::string, FocalMechanismPtr> fms;
		MagnitudePtr prefMag;

		if ( !event->preferredMagnitudeID().empty() ) {
			prefMag = Magnitude::Find(event->preferredMagnitudeID());
		}

		for ( size_t i = 0; i < event->originReferenceCount(); ++i ) {
			Origin *org = Origin::Find(event->originReference(i)->originID());
			if ( org && orgs.find(org->publicID()) == orgs.end() ) {
				orgs[org->publicID()] = org;
			}
		}

		if ( SC_D._withFocalMechanisms ) {
			for ( size_t i = 0; i < event->focalMechanismReferenceCount(); ++i ) {
				FocalMechanism *fm = FocalMechanism::Find(event->focalMechanismReference(i)->focalMechanismID());
				if ( fm && fms.find(fm->publicID()) == fms.end() ) {
					fms[fm->publicID()] = fm;
				}
			}
		}

		if ( SC_D._reader ) {
			if ( event->originReferenceCount() == 0 ) {
				SC_D._reader->load(event);
			}

			DatabaseIterator it = SC_D._reader->getOrigins(event->publicID());
			while ( *it ) {
				Origin *org = Origin::Cast(*it);
				if ( org && orgs.find(org->publicID()) == orgs.end() ) {
					orgs[org->publicID()] = org;
				}
				++it;
			}
			it.close();

			if ( !prefMag && !event->preferredMagnitudeID().empty() ) {
				prefMag = Magnitude::Cast(SC_D._reader->getObject(Magnitude::TypeInfo(), event->preferredMagnitudeID()));
			}
		}

		if ( !item ) {
			item = addEvent(event, false);
		}

		for ( auto &orgItem : orgs ) {
			addOrigin(orgItem.second.get(), item, true);
		}

		if ( SC_D._withFocalMechanisms ) {
			for ( auto &fmItem : fms ) {
				addFocalMechanism(fmItem.second.get(), item);
			}
		}

		item->update(this);

	}
	else if ( event ) {
		auto *item = findEvent(event->publicID());
		MagnitudePtr prefMag = Magnitude::Find(event->preferredMagnitudeID());

		if ( !item ) {
			OriginPtr prefOrg = Origin::Find(event->preferredOriginID());

			if ( !prefOrg && SC_D._reader ) {
				prefOrg = Origin::Cast(SC_D._reader->getObject(Origin::TypeInfo(), event->preferredOriginID()));

				if ( (SC_D._itemConfig.customColumn != -1) && prefOrg && prefOrg->commentCount() == 0 ) {
					SC_D._reader->loadComments(prefOrg.get());
				}
			}

			if ( !prefMag && SC_D._reader && !event->preferredMagnitudeID().empty() ) {
				prefMag = Magnitude::Cast(SC_D._reader->getObject(Magnitude::TypeInfo(), event->preferredMagnitudeID()));
			}

			item = addEvent(event, false);
			if ( prefOrg ) {
				if ( !event->originReference(prefOrg->publicID()) ) {
					event->add(new OriginReference(prefOrg->publicID()));
				}
				addOrigin(prefOrg.get(), item, true);
			}
		}

		if ( !event->originReference(origin->publicID()) ) {
			event->add(new OriginReference(origin->publicID()));
		}

		if ( !findOrigin(origin->publicID()) ) {
			addOrigin(origin, item, true);
		}

		item->update(this);
	}
	else {
		auto *item = findOrigin(origin->publicID());
		if ( !item ) {
			addOrigin(origin, nullptr, false);
		}
	}

	SC_D._blockRemovingOfExpiredEvents = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::setMessagingEnabled(bool e) {
	SC_D._updateLocalEPInstance = !e;
	if ( SC_D._updateLocalEPInstance ) {
		SC_D._treeWidget->setDragEnabled(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::setEventModificationsEnabled(bool e) {
	SC_D._treeWidget->setDragEnabled(e);
	SC_D._treeWidget->setAcceptDrops(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::initTree() {
	SC_D._treeWidget->clear();
	SC_D._visibleEventCount = 0;

	if ( SC_D._withOrigins ) {
		SC_D._unassociatedEventItem = addEvent(nullptr, false);
	}
	else {
		SC_D._unassociatedEventItem = nullptr;
	}

	for (int i = 0; i < SC_D._treeWidget->columnCount(); i++) {
		SC_D._treeWidget->resizeColumnToContents(i);
	}

	PublicObjectEvaluator::Instance().clear(this);
	PublicObjectEvaluator::Instance().setDatabaseURI(SCApp->databaseURI().c_str());

	emit reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventListView::eventFilter(QObject *obj, QEvent *ev) {
	if ( obj == SC_D._treeWidget->viewport() ) {
		if ( ev->type() == QEvent::Drop ) {
			auto *event = static_cast<QDropEvent*>(ev);
			auto *item = static_cast<SchemeTreeItem*>(
			        SC_D._treeWidget->itemAt(QT_EVENT_POS(event)));
			if ( !item || item->type() == ST_None ) {
				event->ignore();
				return true;
			}

			if ( item->type() == ST_Event ) {
				auto *eitem = static_cast<EventTreeItem*>(item);
				auto *evt = eitem->event();
				if ( !evt ) {
					return true;
				}

				if ( event->mimeData()->hasFormat("uri/event") ) {
					QString eventID = event->mimeData()->data("uri/event");

					// Nothing to do, same eventID
					if ( eventID == item->object()->publicID().data() ) {
						return true;
					}

					if ( QMessageBox::question(
						this, "Event merge",
						QString(
							"You requested a merge of event %1 into "
							"event %2. This command will modify the "
							"database.\n"
							"Are you sure you want to continue?"
						).arg(eventID, item->object()->publicID().c_str()),
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes
						) == QMessageBox::No ) {
						event->ignore();
						return true;
					}

					sendJournalAndWait(item->object()->publicID(), CMD_MERGE_EVENT,
					                   eventID.toStdString(), SCApp->messageGroups().event.c_str());
				}
				else if ( event->mimeData()->hasFormat("uri/origin") ) {
					QString originID = event->mimeData()->data("uri/origin");

					if ( QMessageBox::question(
						this, "Origin move",
						QString(
							"You requested to associate origin %1 with "
							"event %2. This command will modify the "
							"database.\n"
							"Are you sure you want to continue?"
						).arg(originID, item->object()->publicID().c_str()),
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes
						) == QMessageBox::No ) {
						event->ignore();
						return true;
					}

					sendJournalAndWait(eitem->event()->publicID(), CMD_GRAB_ORIGIN,
					                   originID.toStdString(), SCApp->messageGroups().event.c_str());
				}
			}
			else {
				event->ignore();
				return true;
			}

			event->accept();
		}
		else if ( ev->type() == QEvent::Resize ) {
			SC_D._busyIndicatorLabel->move(
				(SC_D._treeWidget->viewport()->width()-SC_D._busyIndicatorLabel->width())/2,
				(SC_D._treeWidget->viewport()->height()-SC_D._busyIndicatorLabel->height())/2);
		}
	}

	return QObject::eventFilter(obj, ev);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::clear() {
	initTree();
	emit visibleEventCountChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::clearDatabaseFilter() {
	SC_D._filterWidget->setFilter(EventListView::Filter());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::selectEventFM(const QString &url) {
	auto *ev = static_cast<Event*>(sender()->property("eventPtr").value<void*>());
	if ( ev ) {
		emit eventFMSelected(ev);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::regionSelectionChanged(int index) {
	SC_D._regionIndex = index;

	if ( SC_D._regionIndex == 0 ) {
		SC_D._ui->btnChangeRegion->show();
	}
	else {
		SC_D._ui->btnChangeRegion->hide();
	}

	if ( SC_D._hideOutsideRegion ) {
		updateHideState();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::changeRegion() {
	EventListViewRegionFilterDialog dlg(this, &SC_D._filterRegions[0], &SC_D._filterRegions);
	if ( dlg.exec() == QDialog::Accepted && SC_D._hideOutsideRegion ) {
		updateHideState();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::setInterval(const Seiscomp::Core::TimeWindow &tw) {
	QDateTime start;
	QDateTime end;

	if ( !SCScheme.dateTime.useLocalTime ) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
		start.setTimeZone(QTimeZone::UTC);
		end.setTimeZone(QTimeZone::UTC);
#else
		start.setTimeZone(QTimeZone(Qt::UTC));
		end.setTimeZone(QTimeZone(Qt::UTC));
#endif
		start.setSecsSinceEpoch(tw.startTime().epochSeconds());
		end.setSecsSinceEpoch(tw.endTime().epochSeconds());
	}
	else {
		start.setSecsSinceEpoch(tw.startTime().epochSeconds());
		end.setSecsSinceEpoch(tw.endTime().epochSeconds());
	}

	QT_DTE_SET(SC_D._ui->dateTimeEditStart, start, SCScheme.dateTime.useLocalTime);
	QT_DTE_SET(SC_D._ui->dateTimeEditEnd, end, SCScheme.dateTime.useLocalTime);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::selectFirstEnabledEvent() {
	for ( int i = 0; i < SC_D._treeWidget->topLevelItemCount(); ++i ) {
		auto *item = static_cast<TreeItem*>(SC_D._treeWidget->topLevelItem(i));
		if ( !item->isEnabled() ) {
			continue;
		}

		SEISCOMP_DEBUG("First enabled event in list at index %d", i);
		selectEvent(i);
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::selectEvent(int index) {
	if ( index >= SC_D._treeWidget->topLevelItemCount() ) {
		return;
	}

	SC_D._treeWidget->setCurrentItem(SC_D._treeWidget->topLevelItem(index));
	loadItem(SC_D._treeWidget->currentItem());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::selectEventID(const std::string& publicID) {
	SchemeTreeItem *item = findEvent(publicID);
	if ( item ) {
		SC_D._treeWidget->setCurrentItem(item);
		loadItem(SC_D._treeWidget->currentItem());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::setPreviousEvent() {
	int idx = SC_D._treeWidget->currentIndex().row();
	while ( ++idx < SC_D._treeWidget->topLevelItemCount()-1 ) {
		auto *item = SC_D._treeWidget->topLevelItem(idx);
		if ( !item->isHidden() ) {
			break;
		}
	}

	if ( idx < SC_D._treeWidget->topLevelItemCount()-1 ) {
		auto oldMode = SC_D._treeWidget->selectionMode();
		SC_D._treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
		selectEvent(idx);
		SC_D._treeWidget->setSelectionMode(oldMode);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::setNextEvent() {
	int idx = SC_D._treeWidget->currentIndex().row();
	while ( --idx >= 0 ) {
		auto *item = SC_D._treeWidget->topLevelItem(idx);
		if ( !item->isHidden() ) {
			break;
		}
	}

	if ( idx >= 0 ) {
		auto oldMode = SC_D._treeWidget->selectionMode();
		SC_D._treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
		selectEvent(idx);
		SC_D._treeWidget->setSelectionMode(oldMode);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::readLastDays() {
	SC_D._filter = SC_D._filterWidget->filter();
	SC_D._filter.endTime = Core::Time::UTC();
	SC_D._filter.startTime = SC_D._filter.endTime - Core::TimeSpan(SC_D._ui->spinBox->value()*86400, 0);
	setInterval(Core::TimeWindow(SC_D._filter.startTime, SC_D._filter.endTime));

	if ( SC_D._filter.isNull() ) {
		SC_D._ui->btnFilter->setPalette(QPalette());
	}
	else {
		QPalette p = SC_D._ui->btnFilter->palette();
		p.setColor(QPalette::Button, p.color(QPalette::Highlight));
		SC_D._ui->btnFilter->setPalette(p);
	}

	readFromDatabase(SC_D._filter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::readInterval() {
	SC_D._filter = SC_D._filterWidget->filter();
	SC_D._filter.startTime = Core::Time(SC_D._ui->dateTimeEditStart->date().year(),
	                                    SC_D._ui->dateTimeEditStart->date().month(),
	                                    SC_D._ui->dateTimeEditStart->date().day(),
	                                    SC_D._ui->dateTimeEditStart->time().hour(),
	                                    SC_D._ui->dateTimeEditStart->time().minute(),
	                                    SC_D._ui->dateTimeEditStart->time().second());

	SC_D._filter.endTime = Core::Time(SC_D._ui->dateTimeEditEnd->date().year(),
	                                  SC_D._ui->dateTimeEditEnd->date().month(),
	                                  SC_D._ui->dateTimeEditEnd->date().day(),
	                                  SC_D._ui->dateTimeEditEnd->time().hour(),
	                                  SC_D._ui->dateTimeEditEnd->time().minute(),
	                                  SC_D._ui->dateTimeEditEnd->time().second());

	if ( SC_D._filter.isNull() ) {
		SC_D._ui->btnFilter->setPalette(QPalette());
	}
	else {
		QPalette p = SC_D._ui->btnFilter->palette();
		p.setColor(QPalette::Button, p.color(QPalette::Highlight));
		SC_D._ui->btnFilter->setPalette(p);
	}

	readFromDatabase(SC_D._filter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::readFromDatabase() {
	readInterval();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::readFromDatabase(const Filter &filter) {
	if ( !SC_D._reader ) {
		return;
	}

	initTree();

	EventParameters ep;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	SC_D._blockSelection = true;
	SC_D._blockRemovingOfExpiredEvents = true;
	SC_D._blockCountSignal = true;

	EventPtr event;

	QProgressDialog progress;
	progress.setWindowModality(Qt::WindowModal);
	progress.setRange(0, 0);

	//progress.setWindowModality(Qt::WindowModal);
	progress.setWindowTitle(tr("Please wait..."));
	progress.setLabelText(tr("Reading data..."));

	QMap<int, EventPtr>  eventIDs;
	QMap<int, OriginPtr> originIDs;
	QMap<int, FocalMechanismPtr> fmIDs;

	//Core::TimeWindow timeWindow(Core::Time::UTC() - _timeAgo, Core::Time::UTC());

	SC_D._timeAgo = Core::Time::UTC() - filter.startTime;
	progress.setLabelText(tr("Reading events..."));

	DatabaseIterator it = getEvents(SC_D._reader, filter);
	while ( (event = static_cast<Event*>(*it)) ) {
		if ( progress.wasCanceled() ) {
			break;
		}

		ep.add(event.get());
		eventIDs[it.oid()] = event.get();
		++it;
	}
	it.close();

	// Read comments
	CommentPtr comment;
	it = getComments4Events(SC_D._reader, filter);
	while ( (comment = Comment::Cast(*it)) ) {
		if( progress.wasCanceled() ) {
			break;
		}
		EventPtr evt = eventIDs[it.parentOid()];
		if ( evt ) {
			evt->add(comment.get());
		}
		++it;
	}
	it.close();

	if ( SC_D._withOrigins ) {
		progress.setLabelText(tr("Reading origins..."));

		it = getEventOriginReferences(SC_D._reader, filter);

		OriginReferencePtr oref;

		for ( ; (oref = static_cast<OriginReference*>(*it)); ++it ) {
			if ( progress.wasCanceled() ) {
				break;
			}

			auto mit = eventIDs.find(it.parentOid());
			if ( mit == eventIDs.end() ) {
				continue;
			}

			const EventPtr &ev = mit.value();

			ev->add(oref.get());
		}

		it.close();

		it = getEventOrigins(SC_D._reader, filter);

		OriginPtr origin;

		while ( (origin = static_cast<Origin*>(*it)) ) {
			if ( progress.wasCanceled() ) {
				break;
			}

			ep.add(origin.get());
			//if( _withComments )	originIDs[it.oid()] = origin.get();
			originIDs[it.oid()] = origin.get();
			//pubIDs[ origin->publicID() ] = it.oid();

			++it;
		}
		it.close();

		it = getUnassociatedOrigins(SC_D._reader, filter);

		while ( (origin = static_cast<Origin*>(*it)) ) {
			if ( progress.wasCanceled() ) {
				break;
			}

			ep.add(origin.get());
			//if( _withComments )	originIDs[it.oid()] = origin.get();
			originIDs[it.oid()] = origin.get();
			//pubIDs[ origin->publicID() ] = it.oid();

			++it;
		}
		it.close();

		//fetch comments for relevant origins (marker for publishing)

		it = getComments4Origins(SC_D._reader, filter);

		while ( (comment = Comment::Cast(*it)) ) {
			if( progress.wasCanceled() ) {
				break;
			}
			OriginPtr org = originIDs[it.parentOid()];
			if ( org ) {
				org->add(comment.get());
			}
			++it;
		}
		it.close();
	}

	progress.setLabelText(tr("Reading focal mechanisms references..."));

	it = getEventFocalMechanismReferences(SC_D._reader, filter);

	FocalMechanismReferencePtr fmref;

	for ( ; (fmref = static_cast<FocalMechanismReference*>(*it)); ++it ) {
		if ( progress.wasCanceled() ) {
			break;
		}

		QMap<int, EventPtr>::iterator mit = eventIDs.find(it.parentOid());
		if ( mit == eventIDs.end() ) {
			continue;
		}

		const EventPtr &ev = mit.value();

		ev->add(fmref.get());
	}

	it.close();

	if ( SC_D._withFocalMechanisms ) {
		progress.setLabelText(tr("Reading focal mechanisms..."));

		it = getEventFocalMechanisms(SC_D._reader, filter);

		FocalMechanismPtr fm;

		while ( (fm = static_cast<FocalMechanism*>(*it)) ) {
			if ( progress.wasCanceled() ) {
				break;
			}

			fmIDs[it.oid()] = fm;
			ep.add(fm.get());

			++it;
		}
		it.close();

		it = getEventMomentTensors(SC_D._reader, filter);

		MomentTensorPtr mt;
		std::set<std::string> derivedOriginIDs;

		while ( (mt = static_cast<MomentTensor*>(*it)) ) {
			if ( progress.wasCanceled() ) {
				break;
			}

			fm = fmIDs[it.parentOid()];
			if ( fm ) {
				fm->add(mt.get());
				derivedOriginIDs.insert(mt->derivedOriginID());
			}

			++it;
		}
		it.close();

		// Load derived origin magnitudes
		for ( const auto &oID : derivedOriginIDs ) {
			OriginPtr org = Origin::Find(oID);
			if ( !org ) {
				org = Origin::Cast(SC_D._reader->getObject(Origin::TypeInfo(), oID));
				if ( org ) {
					ep.add(org.get());
				}
			}

			if ( org && org->magnitudeCount() == 0 ) {
				SC_D._reader->loadMagnitudes(org.get());
			}
		}
	}

	EventDescriptionPtr description;
	it = getDescriptions4Events(SC_D._reader, filter);
	while ( (description = EventDescription::Cast(*it)) ) {
		if( progress.wasCanceled() ) {
			break;
		}

		EventPtr evt = eventIDs[it.parentOid()];
		if ( evt ) {
			evt->add(description.get());
		}
		++it;
	}
	it.close();

	QSet<void*> associatedOrigins;

	progress.setLabelText(tr("Reading magnitudes..."));
	std::vector<MagnitudePtr> prefMags;
	std::vector<OriginPtr> prefOrigins;

	prefOrigins.reserve(static_cast<size_t>(eventIDs.count()));
	prefMags.reserve(static_cast<size_t>(eventIDs.count()));

	it = getEventMagnitudes(SC_D._reader, filter);
	MagnitudePtr mag;
	while ( (mag = static_cast<Magnitude*>(*it)) ) {
		prefMags.push_back(mag);
		++it;
	}
	it.close();

	if ( !SC_D._withOrigins ) {
		it = getEventPreferredOrigins(SC_D._reader, filter);
		OriginPtr org;
		while ( (org = static_cast<Origin*>(*it)) ) {
			prefOrigins.push_back(org);
			originIDs[it.oid()] = org;
			++it;
		}
		it.close();

		if ( SC_D._itemConfig.customColumn != -1 ) {
			it = getComments4PrefOrigins(SC_D._reader, filter);
			CommentPtr comment;

			while ( (comment = Comment::Cast(*it)) ) {
				if ( progress.wasCanceled() ) {
					break;
				}

				OriginPtr org = originIDs[it.parentOid()];
				if ( org ) {
					org->add(comment.get());
				}
				++it;
			}
			it.close();
		}
	}

	SC_D._treeWidget->setUpdatesEnabled(false);

	for ( size_t i = 0; i < ep.eventCount(); ++i ) {
		Event *event = ep.event(i);

		EventTreeItem *eventItem = addEvent(event, false);
		bool update = false;

		for ( size_t c = 0; c < event->commentCount(); ++c ) {
			if ( event->comment(c)->text() == "published" ) {
				update = true;
				eventItem->setPublishState(true);
			}
		}

		if ( SC_D._withOrigins && eventItem ) {
			Origin *prefOrg = Origin::Find(event->preferredOriginID());
			// Switch loading of all origin information on
			for ( size_t j = 0; j < event->originReferenceCount(); ++j ) {
				OriginReference *ref = event->originReference(j);
				Origin *o = Origin::Find(ref->originID());
				if ( o ) {
					update = true;
					OriginTreeItem *originItem = addOrigin(o, eventItem, prefOrg == o);
					for ( size_t c = 0; c < o->commentCount(); ++c ) {
						// "OriginPublished" shall be superseded by "published"
					        // but here we accept both
						if ( o->comment(c)->text() == "OriginPublished" ||
						     o->comment(c)->text() == "published" ) {
							originItem->setPublishState(true);
							originItem->update(this);
							break;
						}
					}

					associatedOrigins.insert(o);
				}
			}
		}

		if ( SC_D._withFocalMechanisms && eventItem ) {
			for ( size_t j = 0; j < event->focalMechanismReferenceCount(); ++j ) {
				FocalMechanismReference *ref = event->focalMechanismReference(j);
				FocalMechanism *o = FocalMechanism::Find(ref->focalMechanismID());
				if ( o ) {
					update = true;
					addFocalMechanism(o, eventItem);
				}
			}
		}

		if ( update ) {
			updateHideState(eventItem);
			eventItem->update(this);
		}
	}

	if ( SC_D._withOrigins ) {
		// Switch loading of all origin information on
		for ( size_t i = 0; i < ep.originCount(); ++i ) {
			if ( !associatedOrigins.contains(ep.origin(i)) ) {
				addOrigin(ep.origin(i), nullptr, false);
			}
		}
	}

	for (int i = 0; i < SC_D._treeWidget->columnCount(); i++) {
		SC_D._treeWidget->resizeColumnToContents(i);
	}

	SC_D._treeWidget->setUpdatesEnabled(true);

	if ( SC_D._treeWidget->header()->sortIndicatorSection() >= 0 ) {
		sortItems(SC_D._treeWidget->header()->sortIndicatorSection());
	}

	QApplication::restoreOverrideCursor();

	SC_D._blockSelection = false;
	SC_D._blockRemovingOfExpiredEvents = false;
	SC_D._blockCountSignal = false;

	emit eventsUpdated();
	emit visibleEventCountChanged();

	SEISCOMP_DEBUG("Finished reading of events");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::setAutoSelect(bool s) {
	SC_D._autoSelect = s;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::removeExpiredEvents() {
	if ( SC_D._blockRemovingOfExpiredEvents ) {
		return;
	}

	Core::Time now = Core::Time::UTC();

	for ( int i = 0; i < SC_D._treeWidget->topLevelItemCount(); ++i ) {
		auto *item = static_cast<EventTreeItem*>(SC_D._treeWidget->topLevelItem(i));
		auto *event = item->event();
		if ( event ) {
			auto *o = Origin::Find(event->preferredOriginID());
			bool remove = false;
			if ( o ) {
				remove = (now - o->time()) > SC_D._timeAgo;
			}
			else {
				double time = item->data(SC_D._itemConfig.columnMap[COL_OTIME], Qt::UserRole).toDouble();
				remove = now - Time(time) > SC_D._timeAgo;
			}

			if ( remove ) {
				auto *item = SC_D._treeWidget->takeTopLevelItem(i);
				if ( item ) {
					if ( !item->isHidden() ) {
						if ( SC_D._visibleEventCount > 0 ) {
							--SC_D._visibleEventCount;
						}
						emit eventRemovedFromList(event);
						if ( !SC_D._blockCountSignal ) {
							emit visibleEventCountChanged();
						}
					}
					delete item;
					--i;
				}
			}
		}
		else {
			for ( int j = 0; j < item->originItemCount(); ++j ) {
				auto *child = static_cast<OriginTreeItem*>(item->originItem(j));
				if ( child ) {
					if ( child->origin() ) {
						if ( (now - child->origin()->time()) > SC_D._timeAgo ) {
							auto *it = item->takeOrigin(j);
							if ( it ) {
								delete it;
								--j;
							}
						}
					}
				}
			}

			/*
			for ( int j = 0; j < item->focalMechanismItemCount(); ++j ) {
				FocalMechanismTreeItem *child = (FocalMechanismTreeItem*)item->focalMechanismItem(j);
				if ( child ) {
					if ( child->focalMechanism() ) {
						if ( (now - child->focalMechanism()->time()) > _timeAgo ) {
							QTreeWidgetItem* it = item->takeOrigin(j);
							if ( it ) {
								delete it;
								--j;
							}
						}
					}
				}
			}
			*/
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventTreeItem* EventListView::addEvent(Seiscomp::DataModel::Event* event, bool fromNotification) {
	removeExpiredEvents();

	// Read preferred origin for display purpose
	OriginPtr preferredOrigin;
	if ( event ) {
		preferredOrigin = Origin::Find(event->preferredOriginID());
		if ( !preferredOrigin && SC_D._reader ) {
			preferredOrigin = Origin::Cast(SC_D._reader->getObject(Origin::TypeInfo(), event->preferredOriginID()));
			if ( (SC_D._itemConfig.customColumn != -1) && preferredOrigin && preferredOrigin->commentCount() == 0 ) {
				SC_D._reader->loadComments(preferredOrigin.get());
			}
		}
	}

	// Read preferred magnitude for display purpose
	MagnitudePtr preferredMagnitude;
	if ( event && !event->preferredMagnitudeID().empty() ) {
		preferredMagnitude = Magnitude::Find(event->preferredMagnitudeID());
		if ( !preferredMagnitude && SC_D._reader ) {
			preferredMagnitude = Magnitude::Cast(SC_D._reader->getObject(Magnitude::TypeInfo(), event->preferredMagnitudeID()));
		}
	}

	// Read preferred magnitude for display purpose
	FocalMechanismPtr preferredFocalMechanism;
	if ( event && SC_D._withFocalMechanisms ) {
		preferredFocalMechanism = FocalMechanism::Find(event->preferredFocalMechanismID());
		if ( !preferredFocalMechanism && SC_D._reader ) {
			preferredFocalMechanism = FocalMechanism::Cast(SC_D._reader->getObject(FocalMechanism::TypeInfo(), event->preferredFocalMechanismID()));
		}
	}

	auto *item = new EventTreeItem(event, SC_D._itemConfig);
	item->setShowOneItemPerAgency(SC_D._showOnlyLatestPerAgency);

	if ( SC_D._treeWidget->topLevelItemCount() == 0 ) {
		SC_D._treeWidget->insertTopLevelItem(0, item);
	}
	else {
		int pos = SC_D._treeWidget->topLevelItemCount();
		for ( int i = 0; i  < SC_D._treeWidget->topLevelItemCount(); ++i ) {
			if ( SC_D._treeWidget->topLevelItem(i)->data(SC_D._itemConfig.columnMap[COL_OTIME], Qt::UserRole).toDouble()
				  < item->data(SC_D._itemConfig.columnMap[COL_OTIME], Qt::UserRole).toDouble() ) {
				pos = i;
				break;
			}
		}
		SC_D._treeWidget->insertTopLevelItem(pos, item);
	}

	item->update(this);

	if ( event ) {
		// Show event initially
		if ( SC_D._visibleEventCount >= 0 and !item->isHidden() ) {
			++SC_D._visibleEventCount;
		}

		if ( !updateHideState(item) and !item->isHidden() ) {
			emit eventAddedToList(event, false);
			if ( !SC_D._blockCountSignal ) {
				emit visibleEventCountChanged();
			}
		}
	}

	int fixedItems = 0;
	if ( SC_D._unassociatedEventItem ) {
		fixedItems = 1;
	}
	if ( SC_D._treeWidget->topLevelItemCount() - fixedItems == 1 ) {
		for (int i = 0; i < SC_D._treeWidget->columnCount(); i++) {
			SC_D._treeWidget->resizeColumnToContents(i);
		}
	}

	SC_D._ui->btnClear->setEnabled(true);

	updateEventProcessColumns(item, true);

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginTreeItem *
EventListView::addOrigin(Seiscomp::DataModel::Origin* origin, QTreeWidgetItem* parent, bool highPriority) {
	//removeExpiredEvents();

	auto *item = new OriginTreeItem(origin, SC_D._itemConfig);
	auto *eitem = static_cast<EventTreeItem*>(parent?parent:SC_D._unassociatedEventItem);
	eitem->addOriginItem(0,item);
	eitem->resort();

	SC_D._ui->btnClear->setEnabled(true);

	updateOriginProcessColumns(item, highPriority);

	emit originAdded();

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::updateOriginProcessColumns(QTreeWidgetItem *item, bool highPriority) {
	if ( SC_D._itemConfig.originScriptColumns.empty() || !item ) {
		return;
	}

	auto *oitem = static_cast<OriginTreeItem*>(item);
	auto *origin = oitem->origin();

	if ( !origin ) {
		return;
	}

	QStringList scripts;
	for ( int i = 0; i < SC_D._itemConfig.originScriptColumns.size(); ++i ) {
		scripts << SC_D._itemConfig.originScriptColumns[i].script;
		oitem->setBackground(SC_D._itemConfig.originScriptColumns[i].pos,
		                     SCScheme.colors.records.gaps);
	}

	if ( highPriority ) {
		if ( !PublicObjectEvaluator::Instance().prepend(this, origin->publicID().c_str(), Origin::TypeInfo(), scripts) ) {
			SEISCOMP_WARNING("%s: adding origin evaluation jobs failed",
			                 origin->publicID().c_str());
		}
	}
	else {
		if ( !PublicObjectEvaluator::Instance().append(this, origin->publicID().c_str(), Origin::TypeInfo(), scripts) ) {
			SEISCOMP_WARNING("%s: adding origin evaluation jobs failed",
			                 origin->publicID().c_str());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::updateEventProcessColumns(QTreeWidgetItem *item, bool highPriority) {
	if ( SC_D._itemConfig.eventScriptColumns.empty() || !item ) {
		return;
	}

	auto *eitem = static_cast<EventTreeItem*>(item);
	auto *event = eitem->event();

	if ( !event ) {
		return;
	}

	QStringList scripts;
	for ( int i = 0; i < SC_D._itemConfig.eventScriptColumns.size(); ++i ) {
		scripts << SC_D._itemConfig.eventScriptColumns[i].script;
		eitem->setBackground(SC_D._itemConfig.eventScriptColumns[i].pos,
		                     SCScheme.colors.records.gaps);
	}

	if ( highPriority ) {
		if ( !PublicObjectEvaluator::Instance().prepend(this, event->publicID().c_str(), Event::TypeInfo(), scripts) ) {
			SEISCOMP_WARNING("%s: adding event evaluation jobs failed",
			                 event->publicID().c_str());
		}
	}
	else {
		if ( !PublicObjectEvaluator::Instance().append(this, event->publicID().c_str(), Event::TypeInfo(), scripts) ) {
			SEISCOMP_WARNING("%s: adding event evaluation jobs failed",
			                 event->publicID().c_str());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanismTreeItem* EventListView::addFocalMechanism(
        Seiscomp::DataModel::FocalMechanism *fm, QTreeWidgetItem *parent) {
	auto *item = new FocalMechanismTreeItem(fm, SC_D._itemConfig);
	auto *eitem = static_cast<EventTreeItem*>(parent?parent:SC_D._unassociatedEventItem);
	eitem->addFocalMechanismItem(0,item);
	eitem->resort();

	SC_D._ui->btnClear->setEnabled(true);

	emit focalMechanismAdded();
	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::messageAvailable(Seiscomp::Core::Message *msg,
                                     Seiscomp::Client::Packet */*unused*/) {
	auto *cmsg = CommandMessage::Cast(msg);
	if ( cmsg ) {
		onCommand(cmsg);
		return;
	}

	auto *aomsg = ArtificialOriginMessage::Cast(msg);
	if ( aomsg ) {
		auto *o = aomsg->origin();
		if ( o ) {
			emit originSelected(o, nullptr);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::onCommand(Seiscomp::Gui::CommandMessage *cmsg) {
	if ( cmsg->command() == CM_SHOW_ORIGIN ) {
		auto *item = findOrigin(cmsg->parameter());
		if ( item ) {
			loadItem(item);
			return;
		}

		OriginPtr o = Origin::Find(cmsg->parameter());
		if ( !o ) {
			// lets read it
			if ( SC_D._reader ) {
				o = Origin::Cast(SC_D._reader->getObject(Origin::TypeInfo(), cmsg->parameter()));
				//readPicks(o.get());
			}
		}

		if ( o ) {
			SchemeTreeItem *parent = nullptr;
			EventPtr ev = SC_D._reader->getEvent(o->publicID());

			if ( ev ) {
				parent = findEvent(ev->publicID());
				if ( !parent ) {
					parent = addEvent(ev.get(), false);
				}
			}

			//readPicks(o);
			//emit originSelected(o, nullptr);

			auto *item = addOrigin(o.get(), parent, true);
			if ( parent ) {
				parent->update(this);
			}

			loadItem(item);
		}
		else {
			QMessageBox::warning(
				nullptr,
				tr("Load origin"),
				tr("Received a request to show origin %1\nwhich has not been found.")
				.arg(cmsg->parameter().c_str())
			);
		}
	}
	else if ( cmsg->command() == CM_OBSERVE_LOCATION ) {
		auto *o = Origin::Cast(cmsg->object());
		if ( o ) {
			emit originSelected(o, nullptr);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::notifierAvailable(Seiscomp::DataModel::Notifier *n) {
	SC_D._treeWidget->setUpdatesEnabled(false);

	if ( SC_D._withOrigins ) {
		auto *o = Origin::Cast(n->object());
		if ( o ) {
			switch ( n->operation() ) {
				case OP_ADD:
				{
					auto *item = addOrigin(o, nullptr, false);
					if ( SC_D._autoSelect ) {
						//_treeWidget->setItemSelected(item, true);
						loadItem(item);
					}
					break;
				}
				case OP_UPDATE:
				{
					auto *item = static_cast<SchemeTreeItem*>(findOrigin(o->publicID()));
					if ( item ) {
						updateOriginProcessColumns(item, true);
						item->update(this);
						emit originUpdated(static_cast<Origin*>(item->object()));
						auto *parent = static_cast<EventTreeItem*>(item->parent()->parent());
						auto *e = static_cast<Event*>(parent->object());
						if ( e && e->preferredOriginID() == o->publicID() ) {
							parent->update(this);
							emit eventUpdatedInList(e);
						}
					}
					break;
				}
				default:
					break;
			}

			SC_D._treeWidget->setUpdatesEnabled(true);
			return;
		}
	}

	if ( SC_D._withFocalMechanisms ) {
		auto *fm = FocalMechanism::Cast(n->object());
		if ( fm ) {
			switch ( n->operation() ) {
				case OP_ADD:
					{
						addFocalMechanism(fm, nullptr);
					}
					break;
				case OP_UPDATE:
					{
						auto *item = static_cast<SchemeTreeItem*>(findFocalMechanism(fm->publicID()));
						if ( item ) {
							item->update(this);
							emit focalMechanismUpdated(static_cast<FocalMechanism*>(item->object()));
							auto *parent = static_cast<EventTreeItem*>(item->parent()->parent());
							auto *e = static_cast<Event*>(parent->object());
							if ( e && e->preferredFocalMechanismID() == fm->publicID() ) {
								parent->update(this);
								emit eventUpdatedInList(e);
							}
						}
					}
					break;
				default:
					break;
			}

			SC_D._treeWidget->setUpdatesEnabled(true);
			return;
		}
	}

	auto *e = Event::Cast(n->object());
	if ( e ) {
		switch ( n->operation() ) {
			case OP_ADD:
			{
				auto *item = static_cast<EventTreeItem*>(findEvent(e->publicID()));
				if ( !item ) {
					addEvent(e, true);
				}
				break;
			}
			case OP_REMOVE:
			{
				auto *item = static_cast<EventTreeItem*>(findEvent(e->publicID()));
				if ( item ) {
					SEISCOMP_DEBUG("Delete event item %s", e->publicID().c_str());
					bool visibleItem = !item->isHidden();
					EventPtr event = item->event();
					delete item;
					if ( visibleItem ) {
						if ( SC_D._visibleEventCount > 0 ) {
							--SC_D._visibleEventCount;
						}
						emit eventRemovedFromList(event.get());
						if ( !SC_D._blockCountSignal ) {
							emit visibleEventCountChanged();
						}
					}
				}
				break;
			}
			case OP_UPDATE:
			{
				auto *item = static_cast<EventTreeItem*>(findEvent(e->publicID()));
				if ( !item ) {
					item = addEvent(e, true);
				}
				else {
					updateHideState(item);
				}

				if ( item ) {
					auto *event = static_cast<Event*>(item->object());

					auto *originItem = findOrigin(event->preferredOriginID());
					OriginPtr preferredOrigin;

					if ( originItem ) {
						if ( originItem->parent()->parent() != item ) {
							int index = originItem->parent()->indexOfChild(originItem);
							SEISCOMP_DEBUG("Reparent originItem (update Event), index(%d)", index);
							if ( index >= 0 ) {
								auto *taken = originItem->parent()->takeChild(index);
								if ( taken ) {
									item->addOriginItem(taken);
									item->resort();
								}
							}
						}
						else if ( item->child(0) != originItem ) {
							item->resort();
						}
					}
					else {
						preferredOrigin = Origin::Find(event->preferredOriginID());
						if (!preferredOrigin && SC_D._reader) {
							preferredOrigin = Origin::Cast(SC_D._reader->getObject(Origin::TypeInfo(), event->preferredOriginID()));
							if ( preferredOrigin && SC_D._withOrigins ) {
								addOrigin(preferredOrigin.get(), item, true);
							}
						}
					}

					MagnitudePtr nm;
					if ( !event->preferredMagnitudeID().empty() ) {
						nm = Magnitude::Find(event->preferredMagnitudeID());
						if ( !nm && SC_D._reader ) {
							nm = Magnitude::Cast(SC_D._reader->getObject(Magnitude::TypeInfo(), event->preferredMagnitudeID()));
						}
					}

					updateEventProcessColumns(item, true);
					item->update(this);

					if ( SC_D._withFocalMechanisms ) {
						auto *fmItem = findFocalMechanism(event->preferredFocalMechanismID());
						FocalMechanismPtr preferredFM;
						OriginPtr derivedOrigin;

						if ( fmItem ) {
							if ( fmItem->parent()->parent() != item ) {
								int index = fmItem->parent()->indexOfChild(fmItem);
								SEISCOMP_DEBUG("Reparent originItem (update Event), index(%d)", index);
								if ( index >= 0 ) {
									QTreeWidgetItem *taken = fmItem->parent()->takeChild(index);
									if ( taken ) {
										item->addFocalMechanismItem(taken);
										item->resort();
									}
								}
							}
							else if ( item->child(0) != fmItem ) {
								item->resort();
							}
						}
						else {
							preferredFM = FocalMechanism::Find(event->preferredFocalMechanismID());
							if ( !preferredFM && SC_D._reader ) {
								preferredFM = FocalMechanism::Cast(SC_D._reader->getObject(FocalMechanism::TypeInfo(), event->preferredFocalMechanismID()));
							}

							if ( preferredFM && SC_D._reader ) {
								if ( preferredFM->momentTensorCount() == 0 ) {
									SC_D._reader->loadMomentTensors(preferredFM.get());
								}
							}

							if ( preferredFM && (preferredFM->momentTensorCount() > 0) ) {
								derivedOrigin = Origin::Find(preferredFM->momentTensor(0)->derivedOriginID());
								if ( !derivedOrigin && SC_D._reader ) {
									derivedOrigin = Origin::Cast(SC_D._reader->getObject(Origin::TypeInfo(), preferredFM->momentTensor(0)->derivedOriginID()));
									SC_D._reader->loadMagnitudes(derivedOrigin.get());
								}
							}

							if ( preferredFM ) {
								addFocalMechanism(preferredFM.get(), item);
							}
						}

						item->update(this);
					}

					emit eventUpdatedInList(event);
				}
				break;
			}
			default:
				break;
		}

		SC_D._treeWidget->setUpdatesEnabled(true);
		return;
	}

	if ( SC_D._withOrigins ) {
		auto *ref = OriginReference::Cast(n->object());
		if ( ref ) {
			switch ( n->operation() ) {
				case OP_ADD:
				{
					auto *eventItem = static_cast<EventTreeItem*>(findEvent(n->parentID()));
					if ( eventItem ) {
						SEISCOMP_INFO("found eventitem with publicID '%s', registered(%d)", eventItem->object()->publicID().c_str(), eventItem->object()->registered());
						auto *originItem = findOrigin(ref->originID());
						if ( originItem ) {
							if ( originItem->parent()->parent() != eventItem ) {
								int index = originItem->parent()->indexOfChild(originItem);
								SEISCOMP_DEBUG("Reparent originItem (add OriginReference), index(%d)", index);
								if ( index >= 0 ) {
									auto *taken = originItem->parent()->takeChild(index);
									if ( taken ) {
										eventItem->addOriginItem(taken);
										eventItem->resort();
										eventItem->update(this);
									}
								}
							}
						}
						else {
							auto *org = Origin::Find(ref->originID());
							if ( !org ) {
								org = Origin::Cast(SC_D._reader->getObject(Origin::TypeInfo(), ref->originID()));
							}

							if ( org ) {
								addOrigin(org, eventItem, false);
							}
						}
						if ( !SC_D._checkEventAgency ) {
							updateHideState(eventItem);
						}
					}
					else {
						auto *originItem = findOrigin(ref->originID());
						if ( originItem ) {
							delete originItem;
							originItem = nullptr;
						}
					}
					break;
				}
				case OP_REMOVE:
				{
					auto *eventItem = static_cast<EventTreeItem*>(findEvent(n->parentID()));
					if ( eventItem ) {
						eventItem->update(this);
						auto *originItem = findOrigin(ref->originID());
						if ( originItem && originItem->parent()->parent() == eventItem ) {
							int index = originItem->parent()->indexOfChild(originItem);
							SEISCOMP_DEBUG("Reparent originItem (remove OriginReference), index(%d)", index);
							if ( index >= 0 ) {
								auto *taken = originItem->parent()->takeChild(index);
								if ( taken ) {
									eventItem = static_cast<EventTreeItem*>(SC_D._unassociatedEventItem);
									if ( eventItem ) {
										eventItem->addOriginItem(taken);
										eventItem->update(this);
									}
									else {
										delete taken;
									}
								}
							}
						}
						if ( !SC_D._checkEventAgency ) {
							updateHideState(eventItem);
						}
					}
					break;
				}
				default:
					break;
			};

			SC_D._treeWidget->setUpdatesEnabled(true);
			return;
		}
	}

	if ( SC_D._withFocalMechanisms ) {
		auto *fm_ref = FocalMechanismReference::Cast(n->object());
		if ( fm_ref ) {
			switch ( n->operation() ) {
				case OP_ADD:
					{
						auto *eventItem = static_cast<EventTreeItem*>(findEvent(n->parentID()));
						if ( eventItem ) {
							SEISCOMP_INFO("found eventitem with publicID '%s', registered(%d)", eventItem->object()->publicID().c_str(), eventItem->object()->registered());
							auto *fmItem = findFocalMechanism(fm_ref->focalMechanismID());
							if ( fmItem && fmItem->parent()->parent() != eventItem ) {
								int index = fmItem->parent()->indexOfChild(fmItem);
								SEISCOMP_DEBUG("Reparent fmItem (add FocalMechanismReference), index(%d)", index);
								if ( index >= 0 ) {
									auto *taken = fmItem->parent()->takeChild(index);
									if ( taken ) {
										eventItem->addFocalMechanismItem(taken);
										eventItem->resort();
										eventItem->update(this);
									}
								}
							}
							else if ( !fmItem ) {
								FocalMechanismPtr fm = FocalMechanism::Find(fm_ref->focalMechanismID());
								OriginPtr derivedOrigin;
								if ( !fm && SC_D._reader ) {
									fm = FocalMechanism::Cast(SC_D._reader->getObject(FocalMechanism::TypeInfo(), fm_ref->focalMechanismID()));
								}

								if ( fm && SC_D._reader ) {
									if ( fm->momentTensorCount() == 0 ) {
										SC_D._reader->loadMomentTensors(fm.get());
									}
								}

								if ( fm && (fm->momentTensorCount() > 0) ) {
									derivedOrigin = Origin::Find(fm->momentTensor(0)->derivedOriginID());
									if ( !derivedOrigin && SC_D._reader ) {
										derivedOrigin = Origin::Cast(SC_D._reader->getObject(Origin::TypeInfo(), fm->momentTensor(0)->derivedOriginID()));
										SC_D._reader->loadMagnitudes(derivedOrigin.get());
									}
								}

								if ( fm ) {
									addFocalMechanism(fm.get(), eventItem);
									eventItem->update(this);
								}
							}
						}
						else {
							auto *fmItem = findFocalMechanism(fm_ref->focalMechanismID());
							if ( fmItem ) {
								delete fmItem;
								fmItem = nullptr;
							}
						}
					}
					break;
				case OP_REMOVE:
				{
					auto *eventItem = static_cast<EventTreeItem*>(findEvent(n->parentID()));
					if ( eventItem ) {
						auto *fmItem = findFocalMechanism(fm_ref->focalMechanismID());
						if ( fmItem && fmItem->parent()->parent() == eventItem ) {
							int index = fmItem->parent()->indexOfChild(fmItem);
							SEISCOMP_DEBUG("Remove fmItem (remove FocalMechanismReference), index(%d)", index);
							if ( index >= 0 ) {
								auto *taken = eventItem->takeFocalMechanism(index);
								if ( taken ) {
									delete taken;
									taken = nullptr;
								}
							}
						}
					}
					break;
				}
				default:
					break;
			};

			SC_D._treeWidget->setUpdatesEnabled(true);
			return;
		}
	}

	auto *mag = Magnitude::Cast(n->object());
	if ( mag ) {
		for ( int i = 0; i < SC_D._treeWidget->topLevelItemCount(); ++i ) {
			auto *item = static_cast<EventTreeItem*>(SC_D._treeWidget->topLevelItem(i));
			if ( item->event() && item->event()->preferredMagnitudeID() == mag->publicID() ) {
				item->update(this);
				emit eventUpdatedInList(item->event());
			}
		}

		SC_D._treeWidget->setUpdatesEnabled(true);
		return;
	}

	auto *comment = Comment::Cast(n->object());
	if ( comment ) {
		switch ( n->operation() ) {
			case OP_ADD:
			case OP_UPDATE:
				{
					auto *eventItem = static_cast<EventTreeItem*>(findEvent(n->parentID()));
					if ( eventItem ) {
						if ( comment->text() == "published" ) {
							eventItem->setPublishState(true);
						}
						updateEventProcessColumns(eventItem, true);
						eventItem->update(this);
					}
					else if ( SC_D._withOrigins ) {
						auto *origItem = findOrigin( n->parentID() );
						if ( origItem ) {
							// "OriginPublished" shall be superseded by "published"
							// but here we accept both
							if( comment->text() == "OriginPublished" ||
							    comment->text() == "published") {
								origItem->setPublishState(true);
							}
							updateOriginProcessColumns(origItem, true);
							origItem->update(this);

							eventItem = static_cast<EventTreeItem*>(origItem->parent()->parent());
							auto *o = static_cast<Origin*>(origItem->object());
							auto *e = static_cast<Event*>(eventItem->object());
							if ( e && e->preferredOriginID() == o->publicID() ) {
								eventItem->update(this);
							}
						}
					}
				}
				break;
			default:
				break;
		}

		SC_D._treeWidget->setUpdatesEnabled(true);
		return;
	}

	auto *je = JournalEntry::Cast(n->object());
	if ( je ) {
		if ( n->operation() == OP_ADD ) {
			if ( !SC_D._commandWaitDialog ) {
				SC_D._treeWidget->setUpdatesEnabled(true);
				return;
			}

			auto *dlg = static_cast<CommandWaitDialog*>(SC_D._commandWaitDialog);
			dlg->handle(je);
		}

		SC_D._treeWidget->setUpdatesEnabled(true);
		return;
	}

	SC_D._treeWidget->setUpdatesEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::updateOrigin(Seiscomp::DataModel::Origin* origin) {
	EventParametersPtr ep;

	if ( SC_D._updateLocalEPInstance ) {
 		ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
	}
	else {
		ep = new EventParameters;
	}

	if ( !ep ) {
		return;
	}

	bool wasEnabled = Notifier::IsEnabled();

	if ( !SC_D._updateLocalEPInstance || !origin->parent() ) {
		Notifier::Disable();
		ep->add(origin);
		Notifier::Enable();
	}

	// create the update notifier
	origin->update();

	auto *item = findOrigin(origin->publicID());
	if ( item ) {
		item->update(this);
		auto *parent = static_cast<EventTreeItem*>(item->parent()->parent());
		auto *e = static_cast<Event*>(parent->object());
		if ( e && e->preferredOriginID() == origin->publicID() ) {
			parent->update(this);
			emit eventUpdatedInList(e);
		}
	}

	if ( !SC_D._updateLocalEPInstance ) {
		// send the notifier
		MessagePtr msg = Notifier::GetMessage();
		if ( msg ) {
			//SCApp->sendMessage("LOGGING", msg.get());
			SCApp->sendMessage(SCApp->messageGroups().location.c_str(), msg.get());
		}
	}

	Notifier::SetEnabled(wasEnabled);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::insertOrigin(Seiscomp::DataModel::Origin* origin,
                                 Seiscomp::DataModel::Event* baseEvent,
                                 const Seiscomp::Gui::ObjectChangeList<Seiscomp::DataModel::Pick> &changedPicks,
                                 const std::vector<Seiscomp::DataModel::AmplitudePtr>& newAmplitudes) {
	EventParametersPtr ep;

	if ( SC_D._updateLocalEPInstance ) {
 		ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
	}
	else {
		ep = new EventParameters;
	}

	if ( !ep ) {
		return;
	}

	bool wasEnabled = Notifier::IsEnabled();

	Notifier::Enable();

	// Send picks
	for ( const auto &pickItem : changedPicks ) {
		if ( pickItem.second ) {
			ep->add(pickItem.first.get());
		}
		// TODO: handle updates
	}

	NotifierMessagePtr msg = Notifier::GetMessage();
	if ( msg && !SC_D._updateLocalEPInstance ) {
		//SCApp->sendMessage("LOGGING", msg.get());
		SCApp->sendMessage(SCApp->messageGroups().pick.c_str(), msg.get());
	}

	for ( const auto &newAmplitude : newAmplitudes ) {
		ep->add(newAmplitude.get());
	}

	msg = Notifier::GetMessage();
	if ( msg && !SC_D._updateLocalEPInstance ) {
		SCApp->sendMessage(SCApp->messageGroups().amplitude.c_str(), msg.get());
	}

	// Insert origin to Eventparameters
	ep->add(origin);

	// When a baseOrigin was given insert the new origin to the event of
	// of the baseOrigin when available so the EventAssociationTool has
	// less work to do to find a appropriate Event
	OriginReferencePtr ref;

	if ( baseEvent ) {
		if ( !SC_D._updateLocalEPInstance ) {
			Notifier::Disable();
			ep->add(baseEvent);
			Notifier::Enable();
		}

		ref = new OriginReference(origin->publicID());
		baseEvent->add(ref.get());
	}

	// Send new origin and maybe the manual event assoziation
	msg = Notifier::GetMessage();
	if ( msg ) {
		//SCApp->sendMessage("LOGGING", msg.get());
		if ( !SC_D._updateLocalEPInstance ) {
			SCApp->sendMessage(SCApp->messageGroups().location.c_str(), msg.get());
		}
		if ( ref && baseEvent ) {
			emit originReferenceAdded(baseEvent->publicID(), ref.get());
		}
	}

	Notifier::SetEnabled(wasEnabled);

	for ( auto &notifier : *msg ) {
		notifierAvailable(notifier.get());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::updateFocalMechanism(Seiscomp::DataModel::FocalMechanism *fm) {
	EventParametersPtr ep;

	if ( SC_D._updateLocalEPInstance ) {
 		ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
	}
	else {
		ep = new EventParameters;
	}

	if ( !ep ) {
		return;
	}

	bool wasEnabled = Notifier::IsEnabled();

	if ( !SC_D._updateLocalEPInstance || !fm->parent()->parent() ) {
		Notifier::Disable();
		ep->add(fm);
		Notifier::Enable();
	}

	// create the update notifier
	fm->update();

	auto *item = findFocalMechanism(fm->publicID());
	if ( item ) {
		item->update(this);
		auto *parent = static_cast<EventTreeItem*>(item->parent()->parent());
		auto *e = static_cast<Event*>(parent->object());
		if ( e && e->preferredOriginID() == fm->publicID() ) {
			parent->update(this);
			emit eventUpdatedInList(e);
		}
	}

	if ( !SC_D._updateLocalEPInstance ) {
		// send the notifier
		MessagePtr msg = Notifier::GetMessage();
		if ( msg ) {
			//SCApp->sendMessage("LOGGING", msg.get());
			SCApp->sendMessage(SCApp->messageGroups().focalMechanism.c_str(), msg.get());
		}
	}

	Notifier::SetEnabled(wasEnabled);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::insertFocalMechanism(Seiscomp::DataModel::FocalMechanism *fm,
                                         Seiscomp::DataModel::Event *event,
                                         Seiscomp::DataModel::Origin *origin) {
	EventParametersPtr ep;

	if ( SC_D._updateLocalEPInstance ) {
 		ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
	}
	else {
		ep = new EventParameters;
	}

	if ( !ep ) {
		return;
	}

	bool wasEnabled = Notifier::IsEnabled();

	Notifier::Enable();

	// Send derived origins
	for ( size_t i = 0; i < fm->momentTensorCount(); ++i ) {
		auto *org = Origin::Find(fm->momentTensor(i)->derivedOriginID());
		if ( org ) {
			ep->add(org);
		}
	}

	// Insert origin to Eventparameters
	ep->add(fm);

	// When an event was given insert the new fm to the event
	// so the EventAssociationTool has less work to  find an
	// appropriate event.
	if ( event ) {
		if ( !SC_D._updateLocalEPInstance ) {
			Notifier::Disable();
			ep->add(event);
			Notifier::Enable();
		}

		// Add focal mechanism to event
		FocalMechanismReferencePtr fmref = new FocalMechanismReference;
		fmref->setFocalMechanismID(fm->publicID());
		event->add(fmref.get());
	}


	// Send new fm and maybe the manual event association
	NotifierMessagePtr msg = Notifier::GetMessage();
	if ( msg ) {
		//SCApp->sendMessage("LOGGING", msg.get());
		if ( !SC_D._updateLocalEPInstance ) {
			SCApp->sendMessage(SCApp->messageGroups().focalMechanism.c_str(), msg.get());
		}
	}

	Notifier::SetEnabled(wasEnabled);

	for ( auto &notifier : *msg ) {
		notifierAvailable(notifier.get());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventTreeItem* EventListView::findEvent(const std::string& publicID) {
	for ( int i = 0; i < SC_D._treeWidget->topLevelItemCount(); ++i ) {
		auto *item = static_cast<SchemeTreeItem*>(SC_D._treeWidget->topLevelItem(i));
		if ( item->object() && item->object()->publicID() == publicID ) {
			return static_cast<EventTreeItem*>(item);
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OriginTreeItem* EventListView::findOrigin(const std::string& publicID) {
	for ( int i = 0; i < SC_D._treeWidget->topLevelItemCount(); ++i ) {
		auto *item = SC_D._treeWidget->topLevelItem(i);
		for ( int j = 0; j < item->childCount(); ++j ) {
			auto *citem = item->child(j);
			for ( int k = 0; k < citem->childCount(); ++k ) {
				auto *schemeItem = static_cast<SchemeTreeItem*>(citem->child(k));
				if ( schemeItem->object() && schemeItem->object()->publicID() == publicID ) {
					return static_cast<OriginTreeItem*>(schemeItem);
				}
			}
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanismTreeItem* EventListView::findFocalMechanism(const std::string &publicID) {
	for ( int i = 0; i < SC_D._treeWidget->topLevelItemCount(); ++i ) {
		auto *item = SC_D._treeWidget->topLevelItem(i);
		for ( int j = 0; j < item->childCount(); ++j ) {
			auto *citem = item->child(j);
			for ( int k = 0; k < citem->childCount(); ++k ) {
				auto *schemeItem = static_cast<SchemeTreeItem*>(citem->child(k));
				if ( schemeItem->object() && schemeItem->object()->publicID() == publicID ) {
					return static_cast<FocalMechanismTreeItem*>(schemeItem);
				}
			}
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::loadItem(QTreeWidgetItem *item) {
	if ( SC_D._blockSelection ) {
		return;
	}

	auto *schemeItem = dynamic_cast<SchemeTreeItem*>(item);
	if ( !schemeItem ) {
		return;
	}

	SC_D._blockSelection = true;

	auto *o = Origin::Cast(schemeItem->object());
	if ( o ) {
		Event* event = nullptr;
		auto *parentItem = static_cast<SchemeTreeItem*>(schemeItem->parent()->parent());
		if ( parentItem ) {
			event = Event::Cast(parentItem->object());
		}

		//readPicks(o);

		PublicObjectEvaluator::Instance().moveToFront(o->publicID().c_str());
		emit originSelected(o, event);

		SC_D._blockSelection = false;

		return;
	}

	auto *fm = FocalMechanism::Cast(schemeItem->object());
	if ( fm ) {
		Event* event = nullptr;
		auto *parentItem = static_cast<SchemeTreeItem*>(schemeItem->parent()->parent());
		if ( parentItem ) {
			event = Event::Cast(parentItem->object());
		}

		emit focalMechanismSelected(fm, event);

		SC_D._blockSelection = false;

		return;
	}

	auto *e = Event::Cast(schemeItem->object());
	if ( e ) {
		PublicObjectEvaluator::Instance().moveToFront(e->publicID().c_str());
		emit eventSelected(e);

		if ( SC_D._withOrigins ) {
			Origin* o = Origin::Find(e->preferredOriginID());
			if ( o ) {
				//readPicks(o);
				PublicObjectEvaluator::Instance().moveToFront(o->publicID().c_str());
				emit originSelected(o, e);
			}
		}
		else if (SC_D. _withFocalMechanisms ) {
			auto *fm = FocalMechanism::Find(e->preferredFocalMechanismID());
			if ( fm ) {
				emit focalMechanismSelected(fm, e);
			}
		}

		SC_D._blockSelection = false;

		return;
	}

	SC_D._blockSelection = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::updateOTimeAgoTimer() {
	auto *header = SC_D._treeWidget->header();
	bool hidden = header->isSectionHidden(SC_D._itemConfig.columnMap[COL_TIME_AGO]);
	if ( hidden && SC_D._otimeAgoTimer.isActive() ) {
		SC_D._otimeAgoTimer.stop();
	}
	else if ( !hidden && !SC_D._otimeAgoTimer.isActive() ) {
		SC_D._otimeAgoTimer.start();
	}
	updateOTimeAgo();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::itemSelected(QTreeWidgetItem* item, int column) {
	if ( QApplication::keyboardModifiers() != Qt::NoModifier ) {
		return;
	}

	if ( column == SC_D._itemConfig.columnMap[COL_FM] ) {
		auto *ev = static_cast<Event*>(item->data(column, Qt::UserRole + 1).value<void*>());
		if ( ev ) {
			emit eventFMSelected(ev);
			return;
		}
	}

	loadItem(item);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::itemPressed(QTreeWidgetItem *item, int column) {
	if ( QApplication::mouseButtons() != Qt::RightButton ) {
		return;
	}

	QMenu popup(this);
	auto *actionLoad = popup.addAction(tr("Select"));
	popup.addSeparator();
	auto *actionCopyCell = popup.addAction(tr("Copy cell to clipboard"));
	auto *actionCopyRow = popup.addAction(tr("Copy row to clipboard"));
	auto *actionCopyRows = popup.addAction(tr("Copy selected rows to clipboard"));
	auto *actionExportEventIDs = popup.addAction(tr("Export eventIDs of selected rows"));
	actionExportEventIDs->setToolTip(tr("Export all selected event ids and run a script configured with 'eventlist.scripts.export'."));

	QAction *actionNewEvent = nullptr;
	QAction *actionSplitOrg = nullptr;

	auto *oitem = static_cast<OriginTreeItem*>(item);
	Origin *org = nullptr;

	if ( item->type() == ST_Origin && !SC_D._updateLocalEPInstance ) {
		org = oitem->origin();
		if ( org ) {
			popup.addSeparator();

			if ( oitem->parent()->parent() == SC_D._unassociatedEventItem ) {
				actionNewEvent = popup.addAction("Form new event for origin");
			}
			else {
				actionSplitOrg = popup.addAction("Split origin and create new event");
			}
		}
	}

	auto *action = popup.exec(QCursor::pos());
	if ( !action ) {
		return;
	}

	if ( action == actionLoad ) {
		loadItem(item);
	}
	else if ( action == actionCopyCell ) {
		auto *cb = QApplication::clipboard();
		if ( cb ) {
			cb->setText(item->text(column));
		}
	}
	else if ( action == actionCopyRow ) {
		auto *cb = QApplication::clipboard();
		QString text;
		for ( int i = 0; i < item->columnCount(); ++i ) {
			if ( i > 0 ) {
				text += ";";
			}
			text += item->text(i);
		}

		if ( cb ) {
			cb->setText(text);
		}
	}
	else if ( action == actionCopyRows ) {
		SCApp->copyToClipboard(SC_D._treeWidget, SC_D._treeWidget->header());
	}
	else if ( action == actionExportEventIDs ) {
		if ( SC_D._exportScript.isEmpty() ) {
			QMessageBox::critical(this, tr("Export"),
			                      tr("No script has been configured in 'eventlist.scripts.export', nothing to do."));
			return;
		}

		QString list;

		for ( int i = 0; i < SC_D._treeWidget->topLevelItemCount(); ++i ) {
			auto *item = static_cast<EventTreeItem*>(SC_D._treeWidget->topLevelItem(i));
			if ( item->isSelected() && item->event() ) {
				list += item->event()->publicID().c_str();
				list += "\n";
			}
		}

		if ( !list.isEmpty() ) {
			SEISCOMP_DEBUG("Executing script %s", SC_D._exportScript.toStdString());
			QProcess script;
			#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
			script.start(SC_D._exportScript, QStringList(), QProcess::WriteOnly);
			#else
			script.start(SC_D._exportScript, QProcess::WriteOnly);
			#endif
			if ( !script.waitForStarted() ) {
				SEISCOMP_ERROR("Failed executing script %s", SC_D._exportScript.toStdString());
				QMessageBox::warning(this, tr("Export"), tr("Can't execute script %1\n%2")
				                     .arg(SC_D._exportScript, script.errorString()));
			}
			else {
				script.write(list.toUtf8());
				script.closeWriteChannel();
				QProgressDialog dlgProgress;
				dlgProgress.setRange(0, 0);
				dlgProgress.setLabelText(SC_D._exportScript);
				dlgProgress.setCancelButtonText("Stop");
				connect(&script, SIGNAL(finished(int)), &dlgProgress, SLOT(accept()));
				if ( dlgProgress.exec() != QDialog::Accepted ) {
					script.kill();
					dlgProgress.setLabelText(tr("Wait for script to finish"));
					dlgProgress.setCancelButtonText(QString());
					dlgProgress.exec();
				}
			}
		}
	}
	else if ( action == actionNewEvent ) {
		if ( QMessageBox::question(
			this, "Form a new event",
			QString(
				"You requested to form a new event for origin %1. "
				"This command will modify the  database.\n"
				"Are you sure you want to continue?"
			).arg(org->publicID().c_str()),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes
			) == QMessageBox::No ) {
			return;
		}

		sendJournalAndWait(org->publicID(), CMD_NEW_EVENT, "", SCApp->messageGroups().event.c_str());
	}
	else if ( action == actionSplitOrg ) {
		if ( QMessageBox::question(
			this, "Split origin",
			QString(
				"You requested to remove origin %1 from its event and create "
				"a new event. "
				"This command will modify the  database.\n"
				"Are you sure you want to continue?"
			).arg(org->publicID().c_str()),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes
			) == QMessageBox::No ) {
			return;
		}

		auto *eitem = static_cast<EventTreeItem*>(oitem->parent()->parent());
		auto *e = eitem->event();
		if ( e ) {
			sendJournalAndWait(e->publicID(), CMD_SPLIT_ORIGIN, org->publicID(), SCApp->messageGroups().event.c_str());
		}
		else {
			QMessageBox::critical(
				this, "Error",
				"Internal error."
			);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::copyRowToClipboard() {
	auto *cb = QApplication::clipboard();
	if ( !cb ) {
		return;
	}

	QString text;
	QList<QTreeWidgetItem *> items = SC_D._treeWidget->selectedItems();
	foreach ( QTreeWidgetItem* item, items ) {
		if ( !text.isEmpty() ) {
			text += '\n';
		}

		for ( int i = 0; i < item->columnCount(); ++i ) {
			if ( i > 0 ) {
				text += ";";
			}
			text += item->text(i);
		}
	}

	cb->setText(text);

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QList<Event*> EventListView::selectedEvents() {
	QList<Event*> events;
	if ( SC_D._blockSelection ) {
		return events;
	}

	SC_D._blockSelection = true;

	QList<QTreeWidgetItem *> items = SC_D._treeWidget->selectedItems();
	foreach ( QTreeWidgetItem* item, items ) {
		auto *schemeItem = dynamic_cast<SchemeTreeItem*>(item);
		if ( !schemeItem ) {
			continue;
		}

		auto *e = Event::Cast(schemeItem->object());
		events.push_back(e);
	}

	SC_D._blockSelection = false;

	return events;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QTreeWidget *EventListView::eventTree() {
	return SC_D._treeWidget;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::DataModel::Event *
EventListView::eventFromTreeItem(QTreeWidgetItem *item) {
	auto *schemeItem = dynamic_cast<SchemeTreeItem*>(item);
	return schemeItem ? Event::Cast(schemeItem->object()) : nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int EventListView::eventCount() const {
	return SC_D._treeWidget->topLevelItemCount() - (SC_D._unassociatedEventItem ? 1 : 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int EventListView::visibleEventCount() const {
	if ( SC_D._visibleEventCount < 0 ) {
		qWarning() << "Counting visible events";
		SC_D._visibleEventCount = 0;
		for ( int i = 0; i < SC_D._treeWidget->topLevelItemCount(); ++i ) {
			auto *item = static_cast<EventTreeItem*>(SC_D._treeWidget->topLevelItem(i));
			if ( item->event() ) {
				++SC_D._visibleEventCount;
			}
		}
	}

	return SC_D._visibleEventCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::setSortingEnabled(bool enable) {
	auto *header = SC_D._treeWidget->header();
	if ( !header ) {
		return;
	}

	if ( enable ) {
		header->setSortIndicator(0, Qt::DescendingOrder);
		header->setSortIndicatorShown(true);
		header->setSectionsClickable(true);
	}
	else {
		header->setSortIndicator(-1, Qt::DescendingOrder);
		header->setSortIndicatorShown(false);
		header->setSectionsClickable(true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::sortItems(int col) {
	int count = SC_D._treeWidget->topLevelItemCount();
	auto *header = SC_D._treeWidget->header();
	if ( !header ) {
		return;
	}

	Qt::SortOrder order = header->sortIndicatorOrder();

	SC_D._treeWidget->blockSignals(true);

	QVector<SortItem> items(count);
	for ( int i = 0; i < items.count(); ++i ) {
		items[i].first = SC_D._treeWidget->takeTopLevelItem(0);
		items[i].second = col;
	}

	LessThan compare;

	if ( col == SC_D._itemConfig.columnMap[COL_OTIME] ||
	     col == SC_D._itemConfig.columnMap[COL_TIME_AGO] ||
	     col == SC_D._itemConfig.columnMap[COL_M] ||
	     col == SC_D._itemConfig.columnMap[COL_PHASES] ||
	     col == SC_D._itemConfig.columnMap[COL_RMS] ||
	     col == SC_D._itemConfig.columnMap[COL_AZIMUTHAL_GAP] ||
	     col == SC_D._itemConfig.columnMap[COL_LAT] ||
	     col == SC_D._itemConfig.columnMap[COL_LON] ||
	     col == SC_D._itemConfig.columnMap[COL_DEPTH] ) {
		compare = (order == Qt::AscendingOrder ? &itemLessThan<double> : &itemGreaterThan<double>);
	}
	else if ( col == SC_D._itemConfig.columnMap[COL_ORIGINS] ) {
		compare = (order == Qt::AscendingOrder ? &itemLessThan<unsigned> : &itemGreaterThan<unsigned>);
	}
	else {
		compare = (order == Qt::AscendingOrder ? &itemTextLessThan : &itemTextGreaterThan);
	}

	std::stable_sort(items.begin(), items.end(), compare);

	for ( int i = 0; i < items.count(); ++i ) {
		SC_D._treeWidget->addTopLevelItem(items[i].first);
		static_cast<SchemeTreeItem*>(items[i].first)->update(this);
	}

	updateHideState();

	SC_D._treeWidget->blockSignals(false);

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::setControlsHidden(bool hide) {
	SC_D._ui->frameControls->setHidden(hide);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::setCustomControls(QWidget* widget) const {
	SC_D._ui->frameCustomControls->setLayout(new QHBoxLayout());
	SC_D._ui->frameCustomControls->layout()->setContentsMargins(0, 0, 0, 0);
	SC_D._ui->frameCustomControls->layout()->addWidget(widget);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::setFMLinkEnabled(bool e) {
	SC_D._itemConfig.createFMLink = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::moveSection(int from, int to) {
	auto *header = SC_D._treeWidget->header();
	if ( !header || from < 0 || to < 0 ) {
		return;
	}

	int count = header->count();
	if ( from >= count || to >= count ) {
		return;
	}

	header->moveSection(from, to);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::headerContextMenuRequested(const QPoint &pos) {
	int count = SC_D._treeWidget->header()->count();
	auto *model = SC_D._treeWidget->header()->model();

	QMenu menu;

	QVector<QAction*> actions;

	for ( int i = 0; i < count; ++i ) {
		if ( i == SC_D._itemConfig.columnMap[COL_ORIGINS] && !SC_D._withOrigins ) {
			continue;
		}

		actions.append(menu.addAction(model->headerData(i, Qt::Horizontal).toString()));
		actions.back()->setData(i);
		actions.back()->setCheckable(true);
		actions.back()->setChecked(!SC_D._treeWidget->header()->isSectionHidden(i));

		if ( i == 0 ) {
			actions[i]->setEnabled(false);
		}
	}

	auto *result = menu.exec(SC_D._treeWidget->header()->mapToGlobal(pos));
	if ( !result ) {
		return;
	}

	int section = result->data().toInt();
	if ( section == -1 ) {
		return;
	}

	SC_D._treeWidget->header()->setSectionHidden(section, !result->isChecked());
	if ( result->isChecked() ) {
		SC_D._treeWidget->resizeColumnToContents(section);
	}
	if ( section == SC_D._itemConfig.columnMap[COL_TIME_AGO] ) {
		updateOTimeAgoTimer();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventListView::sendJournalAndWait(const std::string &objectID,
                                       const std::string &action,
                                       const std::string &params,
                                       const char *group) {
	if ( !sendJournal(objectID, action, params, group) ) {
		return false;
	}

	if ( !SC_D._commandWaitDialog ) {
		SC_D._commandWaitDialog = new CommandWaitDialog(this);
		SC_D._commandWaitDialog->setAttribute(Qt::WA_DeleteOnClose);
		connect(SC_D._commandWaitDialog, SIGNAL(destroyed(QObject*)),
		        this, SLOT(waitDialogDestroyed(QObject*)));
		static_cast<CommandWaitDialog*>(SC_D._commandWaitDialog)->show();
	}

	static_cast<CommandWaitDialog*>(SC_D._commandWaitDialog)->setCommand(objectID, action);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::waitDialogDestroyed(QObject *o) {
	if ( SC_D._commandWaitDialog == o ) {
		SC_D._commandWaitDialog = nullptr;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::itemEntered(QTreeWidgetItem *item, int column) {
	if ( column == SC_D._itemConfig.columnMap[COL_FM]
	  && item->data(column, Qt::UserRole+1).isValid() ) {
		setCursor(Qt::PointingHandCursor);
	}
	else {
		unsetCursor();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::itemExpanded(QTreeWidgetItem *item) {
	if ( item->type() != ST_OriginGroup ) {
		return;
	}
	if ( SC_D._itemConfig.originScriptColumns.isEmpty() ) {
		return;
	}

	// If an origin group is expanded, raise its priority

	int rows = item->childCount();
	for ( int i = rows-1; i >= 0; --i ) {
		QTreeWidgetItem *oitem = item->child(i);
		PublicObjectEvaluator::Instance().moveToFront(oitem->text(SC_D._itemConfig.columnMap[COL_ID]));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
	if ( !current ) {
		return;
	}

	if ( (current->type() == ST_Origin && !SC_D._itemConfig.originScriptColumns.isEmpty()) ||
	     (current->type() == ST_Event && !SC_D._itemConfig.eventScriptColumns.isEmpty())) {
		PublicObjectEvaluator::Instance().moveToFront(current->text(SC_D._itemConfig.columnMap[COL_ID]));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::evalResultAvailable(const QString &publicID,
                                        const QString &className,
                                        const QString &script,
                                        const QString &result) {
	std::string pid = publicID.toStdString();

	// Origin processing result
	if ( className == Origin::TypeInfo().className() ) {
		auto *item = findOrigin(pid);
		if ( !item ) {
			return;
		}

		auto it = SC_D._itemConfig.originScriptColumnMap.find(script);
		if ( it == SC_D._itemConfig.originScriptColumnMap.end() ) {
			return;
		}

		item->setText(it.value(), result);
		item->setBackground(it.value(), Qt::NoBrush);
		item->setForeground(it.value(), Qt::NoBrush);

		// Update the event item
		auto *eitem = static_cast<EventTreeItem*>(item->parent()->parent());
		if ( !eitem->event() ) {
			return;
		}

		// If it is the preferred item, copy the column states, unless the
		// column defines a specific event script
		if ( eitem->event()->preferredOriginID() == pid ) {
			for ( int i = 0; i < SC_D._itemConfig.originScriptColumns.size(); ++i ) {
				int pos = SC_D._itemConfig.originScriptColumns[i].pos;
				if ( SC_D._itemConfig.eventScriptPositions.contains(pos) ) {
					continue;
				}
				eitem->setText(pos, item->text(pos));
				eitem->setBackground(pos, item->background(pos));
				eitem->setForeground(pos, item->foreground(pos));
			}
		}
	}
	// Origin processing result
	else if ( className == Event::TypeInfo().className() ) {
		auto *item = findEvent(pid);
		if ( !item ) {
			return;
		}

		auto it = SC_D._itemConfig.eventScriptColumnMap.find(script);
		if ( it == SC_D._itemConfig.eventScriptColumnMap.end() ) {
			return;
		}

		item->setText(it.value(), result);
		item->setBackground(it.value(), Qt::NoBrush);
		item->setForeground(it.value(), Qt::NoBrush);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::evalResultError(const QString &publicID,
                                    const QString &className,
                                    const QString &script,
                                    int error) {
	std::string pid = publicID.toStdString();

	// Origin processing result
	if ( className == Origin::TypeInfo().className() ) {
		auto *item = findOrigin(pid);
		if ( !item ) {
			return;
		}

		auto it = SC_D._itemConfig.originScriptColumnMap.find(script);
		if ( it == SC_D._itemConfig.originScriptColumnMap.end() ) {
			return;
		}

		// Error state
		item->setBackground(it.value(), Qt::NoBrush);
		item->setForeground(it.value(), Qt::darkRed);

		item->setText(it.value(), "!");
		item->setToolTip(it.value(),
		                 QString("%1\n\n%2")
		                 .arg(script, PublicObjectEvaluator::Instance().errorMsg(error)));
	}
	// Event processing result
	else if ( className == Event::TypeInfo().className() ) {
		auto *item = findEvent(pid);
		if ( !item ) {
			return;
		}

		QHash<QString,int>::iterator it = SC_D._itemConfig.eventScriptColumnMap.find(script);
		if ( it == SC_D._itemConfig.eventScriptColumnMap.end() ) {
			return;
		}

		// Error state
		item->setBackground(it.value(), Qt::NoBrush);
		item->setForeground(it.value(), Qt::darkRed);

		item->setText(it.value(), "!");
		item->setToolTip(it.value(),
		                 QString("%1\n\n%2")
		                 .arg(script, PublicObjectEvaluator::Instance().errorMsg(error)));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListView::updateOTimeAgo() {
	QTreeWidgetItemIterator it(SC_D._treeWidget);
	while ( *it ) {
		auto *item = dynamic_cast<SchemeTreeItem*>(*it);
		if ( item ) {
			item->updateTimeAgo();
		}
		++it;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListViewRegionFilterDialog::EventListViewRegionFilterDialog(QWidget *parent,
                                                                 EventListView::Region *target,
                                                                 EventListView::FilterRegions *regionList)
: QDialog(parent)
, _ui(new Ui::EventListViewRegionFilterDialog)
, _target(target), _regionList(regionList) {
	_ui->setupUi(this);

	_ui->cbPolys->addItem(tr("- Unset -"));

	for ( size_t i = 0; i < Seiscomp::Regions::polyRegions().regionCount(); ++i ) {
		auto *feature = Seiscomp::Regions::polyRegions().region(i);
		if ( feature->closedPolygon() ) {
			_ui->cbPolys->addItem(feature->name().c_str());
		}

		for ( auto *feature : Geo::GeoFeatureSetSingleton::getInstance().features() ) {
			if ( feature->closedPolygon() ) {
				_ui->cbPolys->addItem(feature->name().c_str());
			}
		}
	}

	_ui->labelPolys->setEnabled(_ui->cbPolys->count() > 0);
	_ui->cbPolys->setEnabled(_ui->cbPolys->count() > 0);

	_ui->edMinLat->setText(QString::number(_target->bbox.south));
	_ui->edMaxLat->setText(QString::number(_target->bbox.north));
	_ui->edMinLon->setText(QString::number(_target->bbox.west));
	_ui->edMaxLon->setText(QString::number(_target->bbox.east));

	if ( target->poly ) {
		_ui->cbPolys->setCurrentIndex(_ui->cbPolys->findText(_target->poly->name().c_str()));
	}

	QValidator *valLat = new QDoubleValidator(-90,90,6,this);
	QValidator *valLong = new QDoubleValidator(-180,180,6,this);

	_ui->edMinLat->setValidator(valLat); _ui->edMaxLat->setValidator(valLat);
	_ui->edMinLon->setValidator(valLong); _ui->edMaxLon->setValidator(valLong);

	if ( _regionList->isEmpty() ) {
		return;
	}

	for ( const auto &region : *_regionList ) {
		_ui->cbRegions->addItem(region.name);
	}

	connect(_ui->cbRegions, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(regionSelectionChanged(int)));

	connect(_ui->okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(_ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventListViewRegionFilterDialog::~EventListViewRegionFilterDialog() {
	delete _ui;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListViewRegionFilterDialog::regionSelectionChanged(int idx) {
	QString text = _ui->cbRegions->itemText(idx);
	for ( const auto &region : *_regionList ) {
		if ( region.name == text ) {
			_ui->edMinLat->setText(QString::number(region.bbox.south));
			_ui->edMaxLat->setText(QString::number(region.bbox.north));
			_ui->edMinLon->setText(QString::number(region.bbox.west));
			_ui->edMaxLon->setText(QString::number(region.bbox.east));
			if ( region.poly ) {
				_ui->cbPolys->setCurrentIndex(
				        _ui->cbPolys->findText(region.poly->name().c_str()));
			}
			else {
				_ui->cbPolys->setCurrentIndex(0);
			}
			return;
		}
	}

	_ui->edMinLat->setText("");
	_ui->edMaxLat->setText("");
	_ui->edMinLon->setText("");
	_ui->edMaxLon->setText("");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListViewRegionFilterDialog::showError(const QString &msg) {
	QMessageBox::critical(this, "Error", msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventListViewRegionFilterDialog::accept() {
	// Copy minimum latitude
	if ( _ui->edMinLat->text().isEmpty() ) {
		showError("Minimum latitude must not be empty.");
		_ui->edMinLat->setFocus();
		return;
	}
	_target->bbox.south = _ui->edMinLat->text().toDouble();

	// Copy maximum latitude
	if ( _ui->edMaxLat->text().isEmpty() ) {
		showError("Maximum latitude must not be empty.");
		_ui->edMaxLat->setFocus();
		return;
	}
	_target->bbox.north = _ui->edMaxLat->text().toDouble();

	// Copy minimum longitude
	if ( _ui->edMinLon->text().isEmpty() ) {
		showError("Minimum longitude must not be empty.");
		_ui->edMinLon->setFocus();
		return;
	}
	_target->bbox.west = _ui->edMinLon->text().toDouble();

	// Copy maximum longitude
	if ( _ui->edMaxLon->text().isEmpty() ) {
		showError("Maximum longitude must not be empty.");
		_ui->edMaxLon->setFocus();
		return;
	}
	_target->bbox.east = _ui->edMaxLon->text().toDouble();

	std::string poly = _ui->cbPolys->currentText().toStdString();
	_target->poly = nullptr;

	if ( !poly.empty() ) {
		for ( size_t i = 0; i < Seiscomp::Regions::polyRegions().regionCount(); ++i ) {
			auto *feature = Seiscomp::Regions::polyRegions().region(i);
			if ( feature->name() == poly ) {
				_target->poly = feature;
				break;
			}
		}

		if ( !_target->poly ) {
			for ( auto *feature : Geo::GeoFeatureSetSingleton::getInstance().features() ) {
				if ( feature->name() == poly ) {
					_target->poly = feature;
					break;
				}
			}
		}
	}

	QDialog::accept();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // ns Seiscomp::Gui
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
