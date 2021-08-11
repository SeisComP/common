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


#ifndef SEISCOMP_DATAMODEL_ROUTESEEDLINK_H
#define SEISCOMP_DATAMODEL_ROUTESEEDLINK_H


#include <string>
#include <seiscomp/datamodel/object.h>
#include <seiscomp/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(RouteSeedlink);

class Route;


class SC_SYSTEM_CORE_API RouteSeedlinkIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		RouteSeedlinkIndex();
		RouteSeedlinkIndex(const std::string& address);

		//! Copy constructor
		RouteSeedlinkIndex(const RouteSeedlinkIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const RouteSeedlinkIndex&) const;
		bool operator!=(const RouteSeedlinkIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string address;
};


/**
 * \brief This type describes an SeedLink route (data source)
 */
class SC_SYSTEM_CORE_API RouteSeedlink : public Object {
	DECLARE_SC_CLASS(RouteSeedlink)
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		RouteSeedlink();

		//! Copy constructor
		RouteSeedlink(const RouteSeedlink& other);

		//! Destructor
		~RouteSeedlink() override;
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		RouteSeedlink& operator=(const RouteSeedlink& other);
		//! Checks for equality of two objects. Child objects
		//! are not part of the check.
		bool operator==(const RouteSeedlink& other) const;
		bool operator!=(const RouteSeedlink& other) const;

		//! Wrapper that calls operator==
		bool equal(const RouteSeedlink& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Server address in ip:port format
		void setAddress(const std::string& address);
		const std::string& address() const;

		//! priority (1 is highest)
		void setPriority(const OPT(int)& priority);
		int priority() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const RouteSeedlinkIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const RouteSeedlink* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Route* route() const;

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
		RouteSeedlinkIndex _index;

		// Attributes
		OPT(int) _priority;
};


}
}


#endif
