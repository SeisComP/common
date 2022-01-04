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


#ifndef SEISCOMP_DATAMODEL_STATIONGROUP_H
#define SEISCOMP_DATAMODEL_STATIONGROUP_H


#include <seiscomp/datamodel/types.h>
#include <string>
#include <seiscomp/core/datetime.h>
#include <vector>
#include <seiscomp/datamodel/stationreference.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(StationGroup);
DEFINE_SMARTPOINTER(StationReference);

class Inventory;


class SC_SYSTEM_CORE_API StationGroupIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		StationGroupIndex();
		StationGroupIndex(const std::string& code);

		//! Copy constructor
		StationGroupIndex(const StationGroupIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const StationGroupIndex&) const;
		bool operator!=(const StationGroupIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string code;
};


/**
 * \brief This type describes a group of stations, an array or a
 * \brief virtual network
 */
class SC_SYSTEM_CORE_API StationGroup : public PublicObject {
	DECLARE_SC_CLASS(StationGroup)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		StationGroup();

	public:
		//! Copy constructor
		StationGroup(const StationGroup& other);

		//! Constructor with publicID
		StationGroup(const std::string& publicID);

		//! Destructor
		~StationGroup() override;
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static StationGroup* Create();
		static StationGroup* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static StationGroup* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		StationGroup& operator=(const StationGroup& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const StationGroup& other) const;
		bool operator!=(const StationGroup& other) const;

		//! Wrapper that calls operator==
		bool equal(const StationGroup& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! ARRAY or DEPLOYMENT
		void setType(const OPT(StationGroupType)& type);
		StationGroupType type() const;

		//! Virtual network code (up to 20 characters)
		void setCode(const std::string& code);
		const std::string& code() const;

		//! Start of epoch in ISO datetime format
		void setStart(const OPT(Seiscomp::Core::Time)& start);
		Seiscomp::Core::Time start() const;

		//! End of epoch (empty string if the station is open)
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		//! Station group description
		void setDescription(const std::string& description);
		const std::string& description() const;

		//! Optional latitude (eg., of the central station) with
		//! respect to the
		//! World Geodetic System 1984 (WGS84) reference system
		//! (National
		//! Imagery and Mapping Agency 2000) in degrees. The
		//! uncertainties are
		//! given in kilometers.
		void setLatitude(const OPT(double)& latitude);
		double latitude() const;

		//! Optional longitude (eg., of the central station) with
		//! respect to the
		//! World Geodetic System 1984 (WGS84) reference system
		//! (National
		//! Imagery and Mapping Agency 2000) in degrees.
		void setLongitude(const OPT(double)& longitude);
		double longitude() const;

		//! Optional elevation (eg., of the central station) with
		//! respect to the
		//! World Geodetic System 1984 (WGS84) reference system
		//! (National Imagery
		//! and Mapping Agency 2000) in meters.
		void setElevation(const OPT(double)& elevation);
		double elevation() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const StationGroupIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const StationGroup* lhs) const;

	
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
		bool add(StationReference* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(StationReference* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeStationReference(size_t i);
		bool removeStationReference(const StationReferenceIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t stationReferenceCount() const;

		//! Index access
		//! @return The object at index i
		StationReference* stationReference(size_t i) const;
		StationReference* stationReference(const StationReferenceIndex& i) const;

		//! Find an object by its unique attribute(s)

		Inventory* inventory() const;

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
		StationGroupIndex _index;

		// Attributes
		OPT(StationGroupType) _type;
		OPT(Seiscomp::Core::Time) _start;
		OPT(Seiscomp::Core::Time) _end;
		std::string _description;
		OPT(double) _latitude;
		OPT(double) _longitude;
		OPT(double) _elevation;

		// Aggregations
		std::vector<StationReferencePtr> _stationReferences;

	DECLARE_SC_CLASSFACTORY_FRIEND(StationGroup);
};


}
}


#endif
