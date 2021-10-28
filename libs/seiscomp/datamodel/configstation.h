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


#ifndef SEISCOMP_DATAMODEL_CONFIGSTATION_H
#define SEISCOMP_DATAMODEL_CONFIGSTATION_H


#include <string>
#include <seiscomp/datamodel/creationinfo.h>
#include <vector>
#include <seiscomp/datamodel/setup.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ConfigStation);
DEFINE_SMARTPOINTER(Setup);

class ConfigModule;


class SC_SYSTEM_CORE_API ConfigStationIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ConfigStationIndex();
		ConfigStationIndex(const std::string& networkCode,
		                   const std::string& stationCode);

		//! Copy constructor
		ConfigStationIndex(const ConfigStationIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ConfigStationIndex&) const;
		bool operator!=(const ConfigStationIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string networkCode;
		std::string stationCode;
};


class SC_SYSTEM_CORE_API ConfigStation : public PublicObject {
	DECLARE_SC_CLASS(ConfigStation)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		ConfigStation();

	public:
		//! Copy constructor
		ConfigStation(const ConfigStation& other);

		//! Constructor with publicID
		ConfigStation(const std::string& publicID);

		//! Destructor
		~ConfigStation() override;
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static ConfigStation* Create();
		static ConfigStation* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static ConfigStation* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ConfigStation& operator=(const ConfigStation& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const ConfigStation& other) const;
		bool operator!=(const ConfigStation& other) const;

		//! Wrapper that calls operator==
		bool equal(const ConfigStation& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setNetworkCode(const std::string& networkCode);
		const std::string& networkCode() const;

		void setStationCode(const std::string& stationCode);
		const std::string& stationCode() const;

		void setEnabled(bool enabled);
		bool enabled() const;

		//! CreationInfo for the ConfigStation object.
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo& creationInfo();
		const CreationInfo& creationInfo() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ConfigStationIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ConfigStation* lhs) const;

	
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
		bool add(Setup* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Setup* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeSetup(size_t i);
		bool removeSetup(const SetupIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t setupCount() const;

		//! Index access
		//! @return The object at index i
		Setup* setup(size_t i) const;
		Setup* setup(const SetupIndex& i) const;

		//! Find an object by its unique attribute(s)

		ConfigModule* configModule() const;

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
		ConfigStationIndex _index;

		// Attributes
		bool _enabled;
		OPT(CreationInfo) _creationInfo;

		// Aggregations
		std::vector<SetupPtr> _setups;

	DECLARE_SC_CLASSFACTORY_FRIEND(ConfigStation);
};


}
}


#endif
