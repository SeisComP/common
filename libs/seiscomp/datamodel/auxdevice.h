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


#ifndef SEISCOMP_DATAMODEL_AUXDEVICE_H
#define SEISCOMP_DATAMODEL_AUXDEVICE_H


#include <string>
#include <seiscomp/datamodel/blob.h>
#include <vector>
#include <seiscomp/datamodel/auxsource.h>
#include <seiscomp/datamodel/notifier.h>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(AuxDevice);
DEFINE_SMARTPOINTER(AuxSource);

class Inventory;


class SC_SYSTEM_CORE_API AuxDeviceIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AuxDeviceIndex();
		AuxDeviceIndex(const std::string& name);

		//! Copy constructor
		AuxDeviceIndex(const AuxDeviceIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const AuxDeviceIndex&) const;
		bool operator!=(const AuxDeviceIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
};


/**
 * \brief This type describes an auxiliary device
 */
class SC_SYSTEM_CORE_API AuxDevice : public PublicObject {
	DECLARE_SC_CLASS(AuxDevice)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		AuxDevice();

	public:
		//! Copy constructor
		AuxDevice(const AuxDevice &other);

		//! Constructor with publicID
		AuxDevice(const std::string& publicID);

		//! Destructor
		~AuxDevice() override;


	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static AuxDevice *Create();
		static AuxDevice *Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static AuxDevice *Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		AuxDevice &operator=(const AuxDevice &other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const AuxDevice &other) const;
		bool operator!=(const AuxDevice &other) const;

		//! Wrapper that calls operator==
		bool equal(const AuxDevice &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Unique device name
		void setName(const std::string& name);
		const std::string& name() const;

		//! Device description
		void setDescription(const std::string& description);
		const std::string& description() const;

		//! Device model
		void setModel(const std::string& model);
		const std::string& model() const;

		//! Device manufacturer
		void setManufacturer(const std::string& manufacturer);
		const std::string& manufacturer() const;

		void setRemark(const OPT(Blob)& remark);
		Blob& remark();
		const Blob& remark() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const AuxDeviceIndex &index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const AuxDevice *lhs) const;


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
		bool add(AuxSource *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(AuxSource *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeAuxSource(size_t i);
		bool removeAuxSource(const AuxSourceIndex &i);

		//! Retrieve the number of objects of a particular class
		size_t auxSourceCount() const;

		//! Index access
		//! @return The object at index i
		AuxSource *auxSource(size_t i) const;
		AuxSource *auxSource(const AuxSourceIndex &i) const;

		//! Find an object by its unique attribute(s)

		Inventory *inventory() const;

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
		AuxDeviceIndex _index;

		// Attributes
		std::string _description;
		std::string _model;
		std::string _manufacturer;
		OPT(Blob) _remark;

		// Aggregations
		std::vector<AuxSourcePtr> _auxSources;

	DECLARE_SC_CLASSFACTORY_FRIEND(AuxDevice);
};


}
}


#endif
