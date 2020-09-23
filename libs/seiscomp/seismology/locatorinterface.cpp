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


#include <seiscomp/seismology/locatorinterface.h>
#include <seiscomp/datamodel/utils.h>
#include <seiscomp/datamodel/inventory.h>
#include <seiscomp/core/interfacefactory.ipp>


#define SEISCOMP_COMPONENT Locator


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Seismology::LocatorInterface, SC_SYSTEM_CORE_API);

using namespace Seiscomp::DataModel;

namespace Seiscomp {
namespace Seismology {


SensorLocationDelegate::SensorLocationDelegate() {}


LocatorInterface::LocatorInterface() {
	_usingFixedDepth = false;
	_enableDistanceCutOff = false;
	_ignoreInitialLocation = false;
}


LocatorInterface::~LocatorInterface() {
}


LocatorInterface* LocatorInterface::Create(const char* service) {
	if ( service == nullptr ) return nullptr;
	return LocatorInterfaceFactory::Create(service);
}


const std::string &LocatorInterface::name() const {
	return _name;
}


void LocatorInterface::setSensorLocationDelegate(SensorLocationDelegate *delegate) {
	_sensorLocationDelegate = delegate;
}


LocatorInterface::IDList LocatorInterface::parameters() const {
	return IDList();
}


std::string LocatorInterface::parameter(const std::string &name) const {
	return "";
}


bool LocatorInterface::setParameter(const std::string &name,
                                    const std::string &value) {
	return false;
}


std::string LocatorInterface::lastMessage(MessageType) const {
	return "";
}


bool LocatorInterface::supports(Capability c) const {
	return (capabilities() & c) > 0;
}


void LocatorInterface::setFixedDepth(double depth, bool use) {
	_usingFixedDepth = use;
	_fixedDepth = depth;
}

void LocatorInterface::useFixedDepth(bool use) {
	_usingFixedDepth = use;
}


void LocatorInterface::releaseDepth() {
	useFixedDepth(false);
}


void LocatorInterface::setDistanceCutOff(double distance) {
	_enableDistanceCutOff = true;
	_distanceCutOff = distance;
}


void LocatorInterface::releaseDistanceCutOff() {
	_enableDistanceCutOff = false;
}


Pick* LocatorInterface::getPick(Arrival* arrival) const {
	DataModel::Pick* pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
	if ( pick == nullptr )
		return nullptr;

	return pick;
}


SensorLocation* LocatorInterface::getSensorLocation(Pick* pick) const {
	if ( _sensorLocationDelegate )
		return _sensorLocationDelegate->getSensorLocation(pick);

	Inventory *inv = Inventory::Cast(PublicObject::Find("Inventory"));
	return DataModel::getSensorLocation(inv, pick);
}


PickNotFoundException::PickNotFoundException()
: Core::GeneralException() {}


PickNotFoundException::PickNotFoundException(const std::string& str)
: Core::GeneralException(str) {}


LocatorException::LocatorException()
: Core::GeneralException() {}


LocatorException::LocatorException(const std::string& str)
: Core::GeneralException(str) {}


StationNotFoundException::StationNotFoundException()
: Core::GeneralException() {}


StationNotFoundException::StationNotFoundException(const std::string& str)
: Core::GeneralException(str) {}


int arrivalToFlags(const DataModel::Arrival *arrival) {
	try {
		if ( arrival->weight() == 0 )
			return LocatorInterface::F_NONE;
	}
	catch ( ... ) {}

	int flags = LocatorInterface::F_ALL;

	try {
		if ( !arrival->timeUsed() )
			flags &= ~LocatorInterface::F_TIME;
	}
	catch ( ... ) {}

	try {
		if ( !arrival->backazimuthUsed() )
			flags &= ~LocatorInterface::F_BACKAZIMUTH;
	}
	catch ( ... ) {}

	try {
		if ( !arrival->horizontalSlownessUsed() )
			flags &= ~LocatorInterface::F_SLOWNESS;
	}
	catch ( ... ) {}

	return flags;
}


void flagsToArrival(DataModel::Arrival *arrival, int flags) {
	arrival->setTimeUsed(flags & LocatorInterface::F_TIME);
	arrival->setBackazimuthUsed(flags & LocatorInterface::F_BACKAZIMUTH);
	arrival->setHorizontalSlownessUsed(flags & LocatorInterface::F_SLOWNESS);
}


} // of namespace Seismology
} // of namespace Seiscomp
