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


#ifndef SEISCOMP_DATAMODEL_INVENTORY_H
#define SEISCOMP_DATAMODEL_INVENTORY_H


#include <vector>
#include <seiscomp/datamodel/stationgroup.h>
#include <seiscomp/datamodel/auxdevice.h>
#include <seiscomp/datamodel/sensor.h>
#include <seiscomp/datamodel/datalogger.h>
#include <seiscomp/datamodel/responsepaz.h>
#include <seiscomp/datamodel/responsefir.h>
#include <seiscomp/datamodel/responseiir.h>
#include <seiscomp/datamodel/responsepolynomial.h>
#include <seiscomp/datamodel/responsefap.h>
#include <seiscomp/datamodel/network.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Inventory);
DEFINE_SMARTPOINTER(StationGroup);
DEFINE_SMARTPOINTER(AuxDevice);
DEFINE_SMARTPOINTER(Sensor);
DEFINE_SMARTPOINTER(Datalogger);
DEFINE_SMARTPOINTER(ResponsePAZ);
DEFINE_SMARTPOINTER(ResponseFIR);
DEFINE_SMARTPOINTER(ResponseIIR);
DEFINE_SMARTPOINTER(ResponsePolynomial);
DEFINE_SMARTPOINTER(ResponseFAP);
DEFINE_SMARTPOINTER(Network);


class SC_SYSTEM_CORE_API Inventory : public PublicObject {
	DECLARE_SC_CLASS(Inventory)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Inventory();

		//! Copy constructor
		Inventory(const Inventory &other);

		//! Destructor
		~Inventory() override;


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Inventory &operator=(const Inventory &other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Inventory &other) const;
		bool operator!=(const Inventory &other) const;

		//! Wrapper that calls operator==
		bool equal(const Inventory &other) const;


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool add(StationGroup *obj);
		bool add(AuxDevice *obj);
		bool add(Sensor *obj);
		bool add(Datalogger *obj);
		bool add(ResponsePAZ *obj);
		bool add(ResponseFIR *obj);
		bool add(ResponseIIR *obj);
		bool add(ResponsePolynomial *obj);
		bool add(ResponseFAP *obj);
		bool add(Network *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(StationGroup *obj);
		bool remove(AuxDevice *obj);
		bool remove(Sensor *obj);
		bool remove(Datalogger *obj);
		bool remove(ResponsePAZ *obj);
		bool remove(ResponseFIR *obj);
		bool remove(ResponseIIR *obj);
		bool remove(ResponsePolynomial *obj);
		bool remove(ResponseFAP *obj);
		bool remove(Network *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeStationGroup(size_t i);
		bool removeStationGroup(const StationGroupIndex &i);
		bool removeAuxDevice(size_t i);
		bool removeAuxDevice(const AuxDeviceIndex &i);
		bool removeSensor(size_t i);
		bool removeSensor(const SensorIndex &i);
		bool removeDatalogger(size_t i);
		bool removeDatalogger(const DataloggerIndex &i);
		bool removeResponsePAZ(size_t i);
		bool removeResponsePAZ(const ResponsePAZIndex &i);
		bool removeResponseFIR(size_t i);
		bool removeResponseFIR(const ResponseFIRIndex &i);
		bool removeResponseIIR(size_t i);
		bool removeResponseIIR(const ResponseIIRIndex &i);
		bool removeResponsePolynomial(size_t i);
		bool removeResponsePolynomial(const ResponsePolynomialIndex &i);
		bool removeResponseFAP(size_t i);
		bool removeResponseFAP(const ResponseFAPIndex &i);
		bool removeNetwork(size_t i);
		bool removeNetwork(const NetworkIndex &i);

		//! Retrieve the number of objects of a particular class
		size_t stationGroupCount() const;
		size_t auxDeviceCount() const;
		size_t sensorCount() const;
		size_t dataloggerCount() const;
		size_t responsePAZCount() const;
		size_t responseFIRCount() const;
		size_t responseIIRCount() const;
		size_t responsePolynomialCount() const;
		size_t responseFAPCount() const;
		size_t networkCount() const;

		//! Index access
		//! @return The object at index i
		StationGroup *stationGroup(size_t i) const;
		StationGroup *stationGroup(const StationGroupIndex &i) const;

		AuxDevice *auxDevice(size_t i) const;
		AuxDevice *auxDevice(const AuxDeviceIndex &i) const;

		Sensor *sensor(size_t i) const;
		Sensor *sensor(const SensorIndex &i) const;

		Datalogger *datalogger(size_t i) const;
		Datalogger *datalogger(const DataloggerIndex &i) const;

		ResponsePAZ *responsePAZ(size_t i) const;
		ResponsePAZ *responsePAZ(const ResponsePAZIndex &i) const;

		ResponseFIR *responseFIR(size_t i) const;
		ResponseFIR *responseFIR(const ResponseFIRIndex &i) const;

		ResponseIIR *responseIIR(size_t i) const;
		ResponseIIR *responseIIR(const ResponseIIRIndex &i) const;

		ResponsePolynomial *responsePolynomial(size_t i) const;
		ResponsePolynomial *responsePolynomial(const ResponsePolynomialIndex &i) const;

		ResponseFAP *responseFAP(size_t i) const;
		ResponseFAP *responseFAP(const ResponseFAPIndex &i) const;

		Network *network(size_t i) const;
		Network *network(const NetworkIndex &i) const;

		//! Find an object by its unique attribute(s)
		StationGroup *findStationGroup(const std::string& publicID) const;
		AuxDevice *findAuxDevice(const std::string& publicID) const;
		Sensor *findSensor(const std::string& publicID) const;
		Datalogger *findDatalogger(const std::string& publicID) const;
		ResponsePAZ *findResponsePAZ(const std::string& publicID) const;
		ResponseFIR *findResponseFIR(const std::string& publicID) const;
		ResponseIIR *findResponseIIR(const std::string& publicID) const;
		ResponsePolynomial *findResponsePolynomial(const std::string& publicID) const;
		ResponseFAP *findResponseFAP(const std::string& publicID) const;
		Network *findNetwork(const std::string& publicID) const;

		//! Implement Object interface
		bool assign(Object *other) override;
		bool attachTo(PublicObject *parent) override;
		bool detachFrom(PublicObject *parent) override;
		bool detach() override;

		//! Creates a clone
		Object *clone() const override;

		//! Implement PublicObject interface
		bool updateChild(Object *child) override;

		void accept(Visitor *visitor) override;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Aggregations
		std::vector<StationGroupPtr> _stationGroups;
		std::vector<AuxDevicePtr> _auxDevices;
		std::vector<SensorPtr> _sensors;
		std::vector<DataloggerPtr> _dataloggers;
		std::vector<ResponsePAZPtr> _responsePAZs;
		std::vector<ResponseFIRPtr> _responseFIRs;
		std::vector<ResponseIIRPtr> _responseIIRs;
		std::vector<ResponsePolynomialPtr> _responsePolynomials;
		std::vector<ResponseFAPPtr> _responseFAPs;
		std::vector<NetworkPtr> _networks;

	DECLARE_SC_CLASSFACTORY_FRIEND(Inventory);
};


}
}


#endif
