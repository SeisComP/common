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


#ifndef SEISCOMP_DATAMODEL_SETUP_H
#define SEISCOMP_DATAMODEL_SETUP_H


#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Setup);

class ConfigStation;


class SC_SYSTEM_CORE_API SetupIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SetupIndex();
		SetupIndex(const std::string& name);

		//! Copy constructor
		SetupIndex(const SetupIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const SetupIndex&) const;
		bool operator!=(const SetupIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
};


class SC_SYSTEM_CORE_API Setup : public Object {
	DECLARE_SC_CLASS(Setup)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Setup();

		//! Copy constructor
		Setup(const Setup& other);

		//! Destructor
		~Setup() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Setup& operator=(const Setup& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const Setup& other) const;
		bool operator!=(const Setup& other) const;

		//! Wrapper that calls operator==
		bool equal(const Setup& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setName(const std::string& name);
		const std::string& name() const;

		void setParameterSetID(const std::string& parameterSetID);
		const std::string& parameterSetID() const;

		void setEnabled(bool enabled);
		bool enabled() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const SetupIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Setup* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		ConfigStation* configStation() const;

		//! Implement Object interface
		bool assign(Object *other) override;
		bool attachTo(PublicObject *parent) override;
		bool detachFrom(PublicObject *parent) override;
		bool detach() override;

		//! Creates a clone
		Object *clone() const override;

		void accept(Visitor *visitor) override;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Index
		SetupIndex _index;

		// Attributes
		std::string _parameterSetID;
		bool _enabled;
};


}
}


#endif
