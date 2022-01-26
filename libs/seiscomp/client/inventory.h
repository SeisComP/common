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


#ifndef SEISCOMP_CLIENT_INVENTORY_H
#define SEISCOMP_CLIENT_INVENTORY_H


#include <seiscomp/datamodel/inventory.h>
#include <seiscomp/datamodel/pick.h>
#include <seiscomp/datamodel/databasereader.h>
#include <seiscomp/datamodel/utils.h>
#include <seiscomp/utils/stringfirewall.h>
#include <seiscomp/client.h>

#include <map>
#include <set>


namespace Seiscomp {
namespace Client {


struct SC_SYSTEM_CLIENT_API StationLocation {
	StationLocation();
	StationLocation(double lat, double lon, double elevation);

	double latitude;
	double longitude;
	double elevation;
};


typedef std::vector<Seiscomp::DataModel::Station*> StationList;

class SC_SYSTEM_CLIENT_API Inventory {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	private:
		//! Private c'tor. This class implements the singleton pattern and
		//! can be accessed through the static Instance() method.
		Inventory();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		static Inventory* Instance();
		static void Reset();

		void load(const char *filename);
		void load(DataModel::DatabaseReader*);
		void setInventory(DataModel::Inventory*);

		int filter(const Util::StringFirewall *networkTypeFW,
		           const Util::StringFirewall *stationTypeFW);

		void loadStations(DataModel::DatabaseReader*);

		//! Returns the station location for a network- and stationcode and
		//! a time. If the station has not been found a ValueException will
		//! be thrown.
		StationLocation stationLocation(const std::string& networkCode,
		                                const std::string& stationCode,
		                                const Core::Time&) const;

		//! Returns the station for a network- and stationcode and
		//! a time. If the station has not been found nullptr will be returned.
		DataModel::Station* getStation(const std::string &networkCode,
		                               const std::string &stationCode,
		                               const Core::Time &,
		                               DataModel::InventoryError *error = nullptr) const;

		//! Returns the sensorlocation for a network-, station- and locationcode and
		//! a time. If the sensorlocation has not been found nullptr will be returned.
		DataModel::SensorLocation* getSensorLocation(const std::string &networkCode,
		                                             const std::string &stationCode,
		                                             const std::string &locationCode,
		                                             const Core::Time &,
		                                             DataModel::InventoryError *error = nullptr) const;

		//! Returns the stream for a network-, station-, location- and channelcode and
		//! a time. If the stream has not been found nullptr will be returned.
		DataModel::Stream* getStream(const std::string &networkCode,
		                             const std::string &stationCode,
		                             const std::string &locationCode,
		                             const std::string &channelCode,
		                             const Core::Time &,
		                             DataModel::InventoryError *error = nullptr) const;

		//! Returns the three streams (vertical, horizontal1, horizontal2) corresponding
		//! to the given network-, station-, location- and channel code
		DataModel::ThreeComponents getThreeComponents(const std::string& networkCode,
		                                             const std::string& stationCode,
		                                             const std::string& locationCode,
		                                             const std::string& channelCode,
		                                             const Core::Time&) const;

		//! Returns the station used for a pick. If the station has not been found
		//! nullptr will be returned.
		DataModel::Station* getStation(const DataModel::Pick*) const;

		//! Returns the sensor location used for a pick. If the sensor location has
		//! not been found nullptr will be returned.
		DataModel::SensorLocation* getSensorLocation(const DataModel::Pick*) const;

		DataModel::Stream* getStream(const DataModel::Pick*) const;

		//! Returns the three streams (vertical, horizontal1, horizontal2) corresponding
		//! to the picked stream.
		DataModel::ThreeComponents getThreeComponents(const DataModel::Pick*) const;

		double getGain(const std::string& networkCode,
		               const std::string& stationCode,
		               const std::string& locationCode,
		               const std::string& channelCode,
		               const Core::Time&);

		//! Returns all defined stations for the given time
		int getAllStations(StationList&, const Core::Time&);

		DataModel::Inventory* inventory();


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		DataModel::InventoryPtr _inventory;
		static Inventory        _instance;
};


}
}


#endif
