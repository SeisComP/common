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


#define SEISCOMP_COMPONENT Locator

#include <seiscomp/logging/log.h>
#include <seiscomp/gui/core/locator.h>
#include <seiscomp/gui/core/application.h>
#include <seiscomp/client/inventory.h>
#include <seiscomp/utils/timer.h>


namespace Seiscomp {
namespace Gui {


namespace {

struct comparePick {
	bool operator()(const Seismology::LocatorInterface::PickItem &first,
	                const Seismology::LocatorInterface::PickItem &second) const {
		return first.pick->time().value() < second.pick->time().value();
	}
};


}


DataModel::Origin* relocate(Seismology::LocatorInterface *locator, DataModel::Origin* origin) {
	if ( !locator )
		throw Core::GeneralException("No locator type set.");

	DataModel::Origin* newOrg = nullptr;
	std::string errorMsg;

	try {
		Util::StopWatch stopWatch;
		newOrg = locator->relocate(origin);
		double elapsed = stopWatch.elapsed();
		SEISCOMP_DEBUG("Locator took %fms", elapsed*1E3);
		if ( newOrg )
			return newOrg;

		errorMsg = "The Relocation failed for some reason.";
	}
	catch ( Core::GeneralException& e ) {
		errorMsg = e.what();
	}

	// if no initial location is supported throw the error
	// after the first try
	if ( !locator->supports(Seismology::LocatorInterface::InitialLocation) )
		throw Core::GeneralException(errorMsg);

	Seismology::LocatorInterface::PickList picks;
	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		DataModel::Arrival* arrival = origin->arrival(i);
		try {
			if ( arrival->weight() < 0.5 ) continue;
		}
		catch ( ... ) {}

		DataModel::Pick* pick = locator->getPick(arrival);
		if ( !pick )
			throw Core::GeneralException("pick '" + arrival->pickID() + "' not found");

		picks.push_back(Seismology::LocatorInterface::PickItem(pick,
		                                                       Seismology::LocatorInterface::F_ALL));
	}

	if ( picks.empty() )
		throw Core::GeneralException("No picks given to relocate");

	std::sort(picks.begin(), picks.end(), comparePick());
	DataModel::SensorLocation *sloc = locator->getSensorLocation(picks.front().pick.get());
	if ( !sloc )
		throw Core::GeneralException("station '" + picks.front().pick->waveformID().networkCode() +
		                             "." + picks.front().pick->waveformID().stationCode() + "' not found");

	DataModel::OriginPtr tmp = DataModel::Origin::Create();
	*tmp = *origin;
	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		DataModel::ArrivalPtr ar = new DataModel::Arrival(*origin->arrival(i));
		tmp->add(ar.get());
	}

	tmp->setLatitude(sloc->latitude());
	tmp->setLongitude(sloc->longitude());
	tmp->setDepth(DataModel::RealQuantity(11.0));
	tmp->setTime(picks.front().pick->time());

	Util::StopWatch stopWatch;
	newOrg = locator->relocate(tmp.get());
	double elapsed = stopWatch.elapsed();
	SEISCOMP_DEBUG("Locator took %fms", elapsed*1E3);

	if ( newOrg )
		return newOrg;

	throw Core::GeneralException(errorMsg);
}


}
}
