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


#ifndef SEISCOMP_DATAMODEL_DATAAVAILABILITY_H
#define SEISCOMP_DATAMODEL_DATAAVAILABILITY_H


#include <vector>
#include <seiscomp/datamodel/dataextent.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(DataAvailability);
DEFINE_SMARTPOINTER(DataExtent);


/**
 * \brief This type can hold data availability related objects
 * \brief (extent and segment).
 */
class SC_SYSTEM_CORE_API DataAvailability : public PublicObject {
	DECLARE_SC_CLASS(DataAvailability)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataAvailability();

		//! Copy constructor
		DataAvailability(const DataAvailability& other);

		//! Destructor
		~DataAvailability() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		DataAvailability& operator=(const DataAvailability& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const DataAvailability& other) const;
		bool operator!=(const DataAvailability& other) const;

		//! Wrapper that calls operator==
		bool equal(const DataAvailability& other) const;

	
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
		bool add(DataExtent* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(DataExtent* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeDataExtent(size_t i);
		bool removeDataExtent(const DataExtentIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t dataExtentCount() const;

		//! Index access
		//! @return The object at index i
		DataExtent* dataExtent(size_t i) const;
		DataExtent* dataExtent(const DataExtentIndex& i) const;

		//! Find an object by its unique attribute(s)
		DataExtent* findDataExtent(const std::string& publicID) const;

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
		std::vector<DataExtentPtr> _dataExtents;

	DECLARE_SC_CLASSFACTORY_FRIEND(DataAvailability);
};


}
}


#endif
