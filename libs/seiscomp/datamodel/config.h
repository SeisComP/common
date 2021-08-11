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


#ifndef SEISCOMP_DATAMODEL_CONFIG_H
#define SEISCOMP_DATAMODEL_CONFIG_H


#include <vector>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Config);
DEFINE_SMARTPOINTER(ParameterSet);
DEFINE_SMARTPOINTER(ConfigModule);


class SC_SYSTEM_CORE_API Config : public PublicObject {
	DECLARE_SC_CLASS(Config)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Config();

		//! Copy constructor
		Config(const Config& other);

		//! Destructor
		~Config() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Config& operator=(const Config& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Config& other) const;
		bool operator!=(const Config& other) const;

		//! Wrapper that calls operator==
		bool equal(const Config& other) const;

	
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
		bool add(ParameterSet* obj);
		bool add(ConfigModule* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(ParameterSet* obj);
		bool remove(ConfigModule* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeParameterSet(size_t i);
		bool removeConfigModule(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t parameterSetCount() const;
		size_t configModuleCount() const;

		//! Index access
		//! @return The object at index i
		ParameterSet* parameterSet(size_t i) const;
		ConfigModule* configModule(size_t i) const;

		//! Find an object by its unique attribute(s)
		ParameterSet* findParameterSet(const std::string& publicID) const;
		ConfigModule* findConfigModule(const std::string& publicID) const;

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
		std::vector<ParameterSetPtr> _parameterSets;
		std::vector<ConfigModulePtr> _configModules;

	DECLARE_SC_CLASSFACTORY_FRIEND(Config);
};


}
}


#endif
