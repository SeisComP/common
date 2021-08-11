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


#ifndef SEISCOMP_DATAMODEL_ARCLINKUSER_H
#define SEISCOMP_DATAMODEL_ARCLINKUSER_H


#include <string>
#include <seiscomp/datamodel/publicobject.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ArclinkUser);

class ArclinkLog;


class SC_SYSTEM_CORE_API ArclinkUserIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkUserIndex();
		ArclinkUserIndex(const std::string& name,
		                 const std::string& email);

		//! Copy constructor
		ArclinkUserIndex(const ArclinkUserIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ArclinkUserIndex&) const;
		bool operator!=(const ArclinkUserIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
		std::string email;
};


class SC_SYSTEM_CORE_API ArclinkUser : public PublicObject {
	DECLARE_SC_CLASS(ArclinkUser)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		ArclinkUser();

	public:
		//! Copy constructor
		ArclinkUser(const ArclinkUser& other);

		//! Constructor with publicID
		ArclinkUser(const std::string& publicID);

		//! Destructor
		~ArclinkUser() override;
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static ArclinkUser* Create();
		static ArclinkUser* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static ArclinkUser* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ArclinkUser& operator=(const ArclinkUser& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const ArclinkUser& other) const;
		bool operator!=(const ArclinkUser& other) const;

		//! Wrapper that calls operator==
		bool equal(const ArclinkUser& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setName(const std::string& name);
		const std::string& name() const;

		void setEmail(const std::string& email);
		const std::string& email() const;

		void setPassword(const std::string& password);
		const std::string& password() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ArclinkUserIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ArclinkUser* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		ArclinkLog* arclinkLog() const;

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
		ArclinkUserIndex _index;

		// Attributes
		std::string _password;

	DECLARE_SC_CLASSFACTORY_FRIEND(ArclinkUser);
};


}
}


#endif
