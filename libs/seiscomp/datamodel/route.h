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


#ifndef SEISCOMP_DATAMODEL_ROUTE_H
#define SEISCOMP_DATAMODEL_ROUTE_H


#include <string>
#include <vector>
#include <seiscomp/datamodel/routearclink.h>
#include <seiscomp/datamodel/routeseedlink.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Route);
DEFINE_SMARTPOINTER(RouteArclink);
DEFINE_SMARTPOINTER(RouteSeedlink);

class Routing;


class SC_SYSTEM_CORE_API RouteIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		RouteIndex();
		RouteIndex(const std::string& networkCode,
		           const std::string& stationCode,
		           const std::string& locationCode,
		           const std::string& streamCode);

		//! Copy constructor
		RouteIndex(const RouteIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const RouteIndex&) const;
		bool operator!=(const RouteIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string networkCode;
		std::string stationCode;
		std::string locationCode;
		std::string streamCode;
};


/**
 * \brief This type describes an ArcLink route (collection of servers
 * \brief that provide specific datastreams)
 */
class SC_SYSTEM_CORE_API Route : public PublicObject {
	DECLARE_SC_CLASS(Route)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Route();

	public:
		//! Copy constructor
		Route(const Route& other);

		//! Constructor with publicID
		Route(const std::string& publicID);

		//! Destructor
		~Route() override;
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Route* Create();
		static Route* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Route* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Route& operator=(const Route& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Route& other) const;
		bool operator!=(const Route& other) const;

		//! Wrapper that calls operator==
		bool equal(const Route& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Network code
		void setNetworkCode(const std::string& networkCode);
		const std::string& networkCode() const;

		//! Station code (empty for any station)
		void setStationCode(const std::string& stationCode);
		const std::string& stationCode() const;

		//! Location code (empty for any location)
		void setLocationCode(const std::string& locationCode);
		const std::string& locationCode() const;

		//! Stream (Channel) code (empty for any stream)
		void setStreamCode(const std::string& streamCode);
		const std::string& streamCode() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const RouteIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Route* lhs) const;

	
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
		bool add(RouteArclink* obj);
		bool add(RouteSeedlink* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(RouteArclink* obj);
		bool remove(RouteSeedlink* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeRouteArclink(size_t i);
		bool removeRouteArclink(const RouteArclinkIndex& i);
		bool removeRouteSeedlink(size_t i);
		bool removeRouteSeedlink(const RouteSeedlinkIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t routeArclinkCount() const;
		size_t routeSeedlinkCount() const;

		//! Index access
		//! @return The object at index i
		RouteArclink* routeArclink(size_t i) const;
		RouteArclink* routeArclink(const RouteArclinkIndex& i) const;

		RouteSeedlink* routeSeedlink(size_t i) const;
		RouteSeedlink* routeSeedlink(const RouteSeedlinkIndex& i) const;

		//! Find an object by its unique attribute(s)

		Routing* routing() const;

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
		// Index
		RouteIndex _index;

		// Aggregations
		std::vector<RouteArclinkPtr> _routeArclinks;
		std::vector<RouteSeedlinkPtr> _routeSeedlinks;

	DECLARE_SC_CLASSFACTORY_FRIEND(Route);
};


}
}


#endif
